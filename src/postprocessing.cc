#include "gllelucamera.hh"
#include "gllelu_main.hh"
#include "shader.hh"
#include "shapes.hh"
#include "texture.hh"

#include "glad/gl.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "SDL.h"

#include <stdexcept>

enum Effect {
    EFFECT_NONE,
    EFFECT_NEGATIVE,
    EFFECT_GRAYSCALE,
    EFFECT_SHARPEN,
    EFFECT_BLUR,
    EFFECT_EDGE,
    EFFECT_END
};

const float floorPlane[] = {
    -1.0f, 0.0f, -1.0f,   0.0f, 1.0f, 0.0f,   0.0f, 2.5f,
     1.0f, 0.0f,  1.0f,   0.0f, 1.0f, 0.0f,   2.5f, 0.0f,
     1.0f, 0.0f, -1.0f,   0.0f, 1.0f, 0.0f,   2.5f, 2.5f,
     1.0f, 0.0f,  1.0f,   0.0f, 1.0f, 0.0f,   2.5f, 0.0f,
    -1.0f, 0.0f, -1.0f,   0.0f, 1.0f, 0.0f,   0.0f, 2.5f,
    -1.0f, 0.0f,  1.0f,   0.0f, 1.0f, 0.0f,   0.0f, 0.0f
};

class PostProcessing: public GLleluCamera
{
public:
    PostProcessing(int argc, char *argv[]);
    virtual ~PostProcessing();

protected:
    virtual Status event(SDL_Event &event);
    virtual Status render();
    void recreate_framebuffer();

private:
    Shader m_ppShader;
    Shader m_sceneShader;
    Texture m_floorTexture;
    Texture m_cubeTexture;
    unsigned int m_planeVAO;
    unsigned int m_planeVBO;
    unsigned int m_cubeVAO;
    unsigned int m_cubeVBO;
    unsigned int m_dummyVAO;
    unsigned int m_FBO;
    unsigned int m_colorTexture;
    unsigned int m_depthStencilRBO;
    Effect m_currentEffect;
};

PostProcessing::PostProcessing(int argc, char *argv[]):
    GLleluCamera(argc, argv),
    m_ppShader("fullscreen.vert", "postprocessing.frag"),
    m_sceneShader("postprocessing_scene.vert", "postprocessing_scene.frag"),
    m_floorTexture("lgl_wall.jpg"),
    m_cubeTexture("lgl_container.jpg"),
    m_planeVAO(0),
    m_planeVBO(0),
    m_cubeVAO(0),
    m_cubeVBO(0),
    m_dummyVAO(0),
    m_colorTexture(0),
    m_depthStencilRBO(0),
    m_currentEffect(EFFECT_NONE)
{
    m_ppShader.use();
    m_ppShader.set_int("texture0", 0);
    m_sceneShader.use();
    m_sceneShader.set_int("texture0", 0);

    camera().position = glm::vec3(2.5f, 1.5f, 3.5f);
    camera().pitch = -20.0f;
    camera().yaw = -127.5f;

    glGenVertexArrays(1, &m_planeVAO);
    glBindVertexArray(m_planeVAO);

    glGenBuffers(1, &m_planeVBO);
    glBindBuffer(GL_ARRAY_BUFFER, m_planeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(floorPlane), floorPlane, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glGenVertexArrays(1, &m_cubeVAO);
    glBindVertexArray(m_cubeVAO);

    glGenBuffers(1, &m_cubeVBO);
    glBindBuffer(GL_ARRAY_BUFFER, m_cubeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cube), cube, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glGenVertexArrays(1, &m_dummyVAO);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glGenFramebuffers(1, &m_FBO);

    recreate_framebuffer();
}

PostProcessing::~PostProcessing()
{
    glDeleteVertexArrays(1, &m_planeVAO);
    glDeleteBuffers(1, &m_planeVBO);
    glDeleteVertexArrays(1, &m_cubeVAO);
    glDeleteBuffers(1, &m_cubeVBO);
    glDeleteVertexArrays(1, &m_dummyVAO);
    glDeleteFramebuffers(1, &m_FBO);
    glDeleteTextures(1, &m_colorTexture);
    glDeleteRenderbuffers(1, &m_depthStencilRBO);
}

Status PostProcessing::event(SDL_Event &event)
{
    switch (event.type) {
        case SDL_KEYUP:
            if (SDL_SCANCODE_RETURN == event.key.keysym.scancode) {
                int tmp = static_cast<int>(m_currentEffect);
                ++tmp;
                if (tmp >= static_cast<int>(EFFECT_END))
                    tmp = 0;
                m_currentEffect = static_cast<Effect>(tmp);
            }
            break;
        case SDL_WINDOWEVENT:
            if (event.window.windowID == window_id()) {
                switch (event.window.event) {
                    case SDL_WINDOWEVENT_SIZE_CHANGED:
                        recreate_framebuffer();
                        break;
                    default:
                        break;
                }
            }
            break;
        default:
            break;
    }
    return Status::Ok;
}

Status PostProcessing::render()
{
    glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_sceneShader.use();
    m_sceneShader.set_mat4("view", view());
    m_sceneShader.set_mat4("projection", projection());

    m_floorTexture.activate(0);

    glBindVertexArray(m_planeVAO);
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(0.0f, -0.51f, 0.0f));
    model = glm::scale(model, glm::vec3(2.5f, 1.0f, 2.5f));
    m_sceneShader.set_mat4("model", model);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    m_cubeTexture.activate(0);

    glBindVertexArray(m_cubeVAO);
    model = glm::translate(glm::mat4(1.0f), glm::vec3(-1.0f, 0.0f, 1.0f));
    m_sceneShader.set_mat4("model", model);
    glDrawArrays(GL_TRIANGLES, 0, 36);

    model = glm::translate(glm::mat4(1.0f), glm::vec3(1.0f, 0.0f, -1.0f));
    m_sceneShader.set_mat4("model", model);
    glDrawArrays(GL_TRIANGLES, 0, 36);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDisable(GL_DEPTH_TEST);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    m_ppShader.use();
    m_ppShader.set_int("effect", static_cast<int>(m_currentEffect));

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_colorTexture);

    glBindVertexArray(m_dummyVAO);
    glDrawArrays(GL_TRIANGLES, 0, 3);

    swap_window();

    return Status::Ok;
}

void PostProcessing::recreate_framebuffer()
{
    if (m_colorTexture)
        glDeleteTextures(1, &m_colorTexture);
    if (m_depthStencilRBO)
        glDeleteRenderbuffers(1, &m_depthStencilRBO);

    glBindFramebuffer(GL_FRAMEBUFFER, m_FBO);

    int width = fb_size().width;
    int height = fb_size().height;

    glGenTextures(1, &m_colorTexture);
    glBindTexture(GL_TEXTURE_2D, m_colorTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glBindTexture(GL_TEXTURE_2D, 0);

    glGenRenderbuffers(1, &m_depthStencilRBO);
    glBindRenderbuffer(GL_RENDERBUFFER, m_depthStencilRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);

    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_colorTexture, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_depthStencilRBO);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        throw std::runtime_error("Failed to create FBO");

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

GLLELU_MAIN_IMPLEMENTATION(PostProcessing)
