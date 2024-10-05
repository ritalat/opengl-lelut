#include "gllelucamera.hh"
#include "gllelu_main.hh"
#include "shader.hh"
#include "shapes.hh"
#include "texture.hh"

#include "glad/gl.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "SDL3/SDL.h"

#include <cmath>
#include <stdexcept>

#define SHADOWMAP_SIZE 2048

const float floorPlane[] = {
    -1.0f, 0.0f, -1.0f,   0.0f, 1.0f, 0.0f,   0.0f, 2.5f,
     1.0f, 0.0f,  1.0f,   0.0f, 1.0f, 0.0f,   2.5f, 0.0f,
     1.0f, 0.0f, -1.0f,   0.0f, 1.0f, 0.0f,   2.5f, 2.5f,
     1.0f, 0.0f,  1.0f,   0.0f, 1.0f, 0.0f,   2.5f, 0.0f,
    -1.0f, 0.0f, -1.0f,   0.0f, 1.0f, 0.0f,   0.0f, 2.5f,
    -1.0f, 0.0f,  1.0f,   0.0f, 1.0f, 0.0f,   0.0f, 0.0f
};

class ShadowMap: public GLleluCamera
{
public:
    ShadowMap(int argc, char *argv[]);
    virtual ~ShadowMap();
    virtual Status event(SDL_Event &event);
    virtual Status render();
    void draw_scene(Shader &shader);

    Shader m_shadowShader;
    Shader m_sceneShader;
    Texture m_floorTexture;
    Texture m_cubeTexture;
    Texture m_cubeSpecularTexture;
    unsigned int m_planeVAO;
    unsigned int m_planeVBO;
    unsigned int m_cubeVAO;
    unsigned int m_cubeVBO;
    unsigned int m_shadowMapFBO;
    unsigned int m_shadowMap;
    bool m_shadowMapping;
};

ShadowMap::ShadowMap(int argc, char *argv[]):
    GLleluCamera(argc, argv),
    m_shadowShader("shadowmap.vert", "shadowmap.frag"),
    m_sceneShader("shadowmap_scene.vert", "shadowmap_scene.frag"),
    m_floorTexture("lgl_wall.jpg"),
    m_cubeTexture("lgl_container2.png"),
    m_cubeSpecularTexture("lgl_container2_specular.png"),
    m_planeVAO(0),
    m_planeVBO(0),
    m_cubeVAO(0),
    m_cubeVBO(0),
    m_shadowMap(0),
    m_shadowMapping(true)
{
    m_camera.position = glm::vec3(1.5f, 5.5f, 3.0f);
    m_camera.pitch = -60.0f;
    m_camera.yaw = -120.0f;

    glGenVertexArrays(1, &m_planeVAO);
    glBindVertexArray(m_planeVAO);

    glGenBuffers(1, &m_planeVBO);
    glBindBuffer(GL_ARRAY_BUFFER, m_planeVBO);

    glBufferData(GL_ARRAY_BUFFER, sizeof(floorPlane), floorPlane, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glGenVertexArrays(1, &m_cubeVAO);
    glBindVertexArray(m_cubeVAO);

    glGenBuffers(1, &m_cubeVBO);
    glBindBuffer(GL_ARRAY_BUFFER, m_cubeVBO);

    glBufferData(GL_ARRAY_BUFFER, sizeof(cube), cube, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glGenFramebuffers(1, &m_shadowMapFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, m_shadowMapFBO);

    glGenTextures(1, &m_shadowMap);
    glBindTexture(GL_TEXTURE_2D, m_shadowMap);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOWMAP_SIZE, SHADOWMAP_SIZE, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glBindTexture(GL_TEXTURE_2D, 0);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_shadowMap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        throw std::runtime_error("Failed to create FBO");

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

ShadowMap::~ShadowMap()
{
    glDeleteVertexArrays(1, &m_planeVAO);
    glDeleteBuffers(1, &m_planeVBO);
    glDeleteVertexArrays(1, &m_cubeVAO);
    glDeleteBuffers(1, &m_cubeVBO);
    glDeleteFramebuffers(1, &m_shadowMapFBO);
    glDeleteTextures(1, &m_shadowMap);
}

Status ShadowMap::event(SDL_Event &event)
{
    switch (event.type) {
        case SDL_EVENT_KEY_UP:
            if (SDL_SCANCODE_RETURN == event.key.scancode) {
                m_shadowMapping = !m_shadowMapping;
            }
            break;
        default:
            break;
    }
    return Status::Ok;
}

Status ShadowMap::render()
{
    glm::vec3 lightPos = glm::vec3(-2.0f * sin(static_cast<float>(SDL_GetTicks()) / 500.0f),
                                   4.0f,
                                   2.0f * cos(static_cast<float>(SDL_GetTicks()) / 500.0f));
    glm::vec3 lightTarget = glm::vec3(0.0f);
    glm::mat4 lightProjection = glm::ortho(-10.f, 10.0f, -10.0f, 10.0f, 1.0f, 7.5f);
    glm::mat4 lightView = glm::lookAt(lightPos,
                                      lightTarget,
                                      glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 lightSpaceMatrix = lightProjection * lightView;

    if (m_shadowMapping) {
        glViewport(0, 0, SHADOWMAP_SIZE, SHADOWMAP_SIZE);
        glBindFramebuffer(GL_FRAMEBUFFER, m_shadowMapFBO);
        glClear(GL_DEPTH_BUFFER_BIT);

        m_shadowShader.use();
        m_shadowShader.set_mat4("lightSpaceMatrix", lightSpaceMatrix);
        glDisable(GL_CULL_FACE);
        draw_scene(m_shadowShader);
        glEnable(GL_CULL_FACE);
    }

    glViewport(0, 0, m_fbSize.width, m_fbSize.height);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_sceneShader.use();
    m_sceneShader.set_bool("enableShadows", m_shadowMapping);
    m_sceneShader.set_int("shadowMap", 10);

    m_sceneShader.set_mat4("view", m_view);
    m_sceneShader.set_mat4("projection", m_projection);
    m_sceneShader.set_mat4("lightSpaceMatrix", lightSpaceMatrix);

    m_sceneShader.set_float("material.shininess", 64.0f);
    m_sceneShader.set_vec3("light.ambient", 0.2f, 0.2f, 0.2f);
    m_sceneShader.set_vec3("light.diffuse", 0.5f, 0.5f, 0.5f);
    m_sceneShader.set_vec3("light.specular", 1.0f, 1.0f, 1.0f);
    m_sceneShader.set_vec3("light.direction", lightTarget - lightPos);
    m_sceneShader.set_vec3("viewPos", m_camera.position);

    glActiveTexture(GL_TEXTURE0 + 10);
    glBindTexture(GL_TEXTURE_2D, m_shadowMap);

    draw_scene(m_sceneShader);

    SDL_GL_SwapWindow(m_window);

    return Status::Ok;
}

void ShadowMap::draw_scene(Shader &shader)
{
    shader.set_int("material.diffuse", 0);
    shader.set_int("material.specular", 0);

    m_floorTexture.activate(0);

    glBindVertexArray(m_planeVAO);
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(0.0f, -0.5f, 0.0f));
    model = glm::scale(model, glm::vec3(2.5f, 1.0f, 2.5f));
    shader.set_mat4("model", model);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    shader.set_int("material.diffuse", 0);
    shader.set_int("material.specular", 1);

    m_cubeTexture.activate(0);
    m_cubeSpecularTexture.activate(1);

    glBindVertexArray(m_cubeVAO);
    model = glm::translate(glm::mat4(1.0f), glm::vec3(-1.0f, 0.0f, 1.0f));
    shader.set_mat4("model", model);
    glDrawArrays(GL_TRIANGLES, 0, 36);

    model = glm::translate(glm::mat4(1.0f), glm::vec3(1.0f, 0.0f, -1.0f));
    shader.set_mat4("model", model);
    glDrawArrays(GL_TRIANGLES, 0, 36);

    model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 2.0f, 0.0f));
    model = glm::rotate(model,
                        glm::radians(static_cast<float>(SDL_GetTicks()) / 10.0f),
                        glm::vec3(0.0f, 1.0f, 0.0f));
    shader.set_mat4("model", model);
    glDrawArrays(GL_TRIANGLES, 0, 36);
}

GLLELU_MAIN_IMPLEMENTATION(ShadowMap)
