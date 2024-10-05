#include "gllelucamera.hh"
#include "gllelu_main.hh"
#include "shader.hh"
#include "shapes.hh"
#include "texture.hh"

#include "glad/gl.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "SDL3/SDL.h"

class Stencil: public GLleluCamera
{
public:
    Stencil(int argc, char *argv[]);
    virtual ~Stencil();
    virtual Status render();

    Shader m_stencilShader;
    Texture m_floorTexture;
    Texture m_cubeTexture;
    unsigned int m_planeVAO;
    unsigned int m_planeVBO;
    unsigned int m_cubeVAO;
    unsigned int m_cubeVBO;
};

Stencil::Stencil(int argc, char *argv[]):
    GLleluCamera(argc, argv),
    m_stencilShader("stencil.vert", "stencil.frag"),
    m_floorTexture("lgl_marble.jpg"),
    m_cubeTexture("vt_texture.jpg"),
    m_planeVAO(0),
    m_planeVBO(0),
    m_cubeVAO(0),
    m_cubeVBO(0)
{
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_STENCIL_TEST);
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

    m_stencilShader.use();
    m_stencilShader.set_int("texture0", 0);

    m_cubeTexture.wrapping(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);

    m_camera.position = glm::vec3(0.0f, 1.0f, 3.0f);
    m_camera.pitch = -20.0f;

    glGenVertexArrays(1, &m_planeVAO);
    glBindVertexArray(m_planeVAO);

    glGenBuffers(1, &m_planeVBO);
    glBindBuffer(GL_ARRAY_BUFFER, m_planeVBO);

    glBufferData(GL_ARRAY_BUFFER, sizeof(plane), plane, GL_STATIC_DRAW);

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

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

Stencil::~Stencil()
{
    glDeleteVertexArrays(1, &m_planeVAO);
    glDeleteBuffers(1, &m_planeVBO);
    glDeleteVertexArrays(1, &m_cubeVAO);
    glDeleteBuffers(1, &m_cubeVBO);
}

Status Stencil::render()
{
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glStencilFunc(GL_ALWAYS, 1, 0xFF);
    glStencilMask(0xFF);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    glm::mat4 model = glm::mat4(1.0f);

    m_stencilShader.use();
    m_stencilShader.set_mat4("view", m_view);
    m_stencilShader.set_mat4("projection", m_projection);

    // Floor
    m_floorTexture.activate(0);
    glBindVertexArray(m_planeVAO);
    model = glm::translate(model, glm::vec3(0.0f, -0.5f, 0.0f));
    model = glm::rotate(model,
                        glm::radians(static_cast<float>(SDL_GetTicks()) / 10.0f),
                        glm::vec3(0.0f, 1.0f, 0.0f));
    m_stencilShader.set_mat4("model", model);
    m_stencilShader.set_float("alpha", 1.0f);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    glStencilFunc(GL_EQUAL, 1, 0xff);
    glStencilMask(0x00);

    glFrontFace(GL_CW);
    glDisable(GL_DEPTH_TEST);

    // Reflection Cube
    m_cubeTexture.activate(0);
    glBindVertexArray(m_cubeVAO);
    model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -1.001f, 0.0f));
    model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    model = glm::rotate(model,
                        glm::radians(static_cast<float>(SDL_GetTicks()) / -10.0f),
                        glm::vec3(0.0f, 0.0f, 1.0f));
    model = glm::scale(model, glm::vec3(1.0f, 1.0f, -1.0f));
    m_stencilShader.set_mat4("model", model);
    m_stencilShader.set_float("alpha", 0.3f);
    glDrawArrays(GL_TRIANGLES, 0, 36);

    glEnable(GL_DEPTH_TEST);
    glFrontFace(GL_CCW);

    glStencilFunc(GL_ALWAYS, 1, 0xFF);
    glStencilMask(0x00);

    // Real Cube
    m_cubeTexture.activate(0);
    glBindVertexArray(m_cubeVAO);
    model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.001f, 0.0f));
    model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    model = glm::rotate(model,
                        glm::radians(static_cast<float>(SDL_GetTicks()) / -10.0f),
                        glm::vec3(0.0f, 0.0f, 1.0f));
    m_stencilShader.set_mat4("model", model);
    m_stencilShader.set_float("alpha", 1.0f);
    glDrawArrays(GL_TRIANGLES, 0, 36);

    SDL_GL_SwapWindow(m_window);

    return Status::Ok;
}

GLLELU_MAIN_IMPLEMENTATION(Stencil)
