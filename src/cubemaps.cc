#include "gllelucamera.hh"
#include "gllelu_main.hh"
#include "model.hh"
#include "shader.hh"
#include "texture.hh"

#include "glad/gl.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "SDL.h"

#include <array>
#include <string_view>

const std::array<std::string_view, 6> skyboxImages {
    "lgl_skybox_right.jpg",
    "lgl_skybox_left.jpg",
    "lgl_skybox_top.jpg",
    "lgl_skybox_bottom.jpg",
    "lgl_skybox_front.jpg",
    "lgl_skybox_back.jpg"
};

const float skyboxVertices[] = {
    -1.0f,  1.0f, -1.0f,
    -1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

    -1.0f,  1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f,  1.0f
};

class Cubemaps: public GLleluCamera
{
public:
    Cubemaps(int argc, char *argv[]);
    virtual ~Cubemaps();
    virtual Status render();

    Shader m_skyboxShader;
    Shader m_environmentMapShader;
    Model m_teapot;
    Cubemap m_skybox;
    unsigned int m_skyboxVAO;
    unsigned int m_skyboxVBO;
};

Cubemaps::Cubemaps(int argc, char *argv[]):
    GLleluCamera(argc, argv),
    m_skyboxShader("skybox.vert", "skybox.frag"),
    m_environmentMapShader("environment_map.vert", "environment_map.frag"),
    m_teapot("teapot.obj"),
    m_skybox(skyboxImages),
    m_skyboxVAO(0),
    m_skyboxVBO(0)
{
    m_skyboxShader.use();
    m_skyboxShader.set_int("skybox", 0);
    m_environmentMapShader.use();
    m_environmentMapShader.set_int("skybox", 0);

    glGenVertexArrays(1, &m_skyboxVAO);
    glBindVertexArray(m_skyboxVAO);

    glGenBuffers(1, &m_skyboxVBO);
    glBindBuffer(GL_ARRAY_BUFFER, m_skyboxVBO);

    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), skyboxVertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

Cubemaps::~Cubemaps()
{
    glDeleteVertexArrays(1, &m_skyboxVAO);
    glDeleteBuffers(1, &m_skyboxVBO);
}

Status Cubemaps::render()
{
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_skybox.activate(0);

    m_environmentMapShader.use();
    m_environmentMapShader.set_mat4("view", m_view);
    m_environmentMapShader.set_mat4("projection", m_projection);
    m_environmentMapShader.set_vec3("viewPos", m_camera.position);

    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(0.0f, -0.5f, 0.0f));
    model = glm::rotate(model, glm::radians(SDL_GetTicks() / 10.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::scale(model, glm::vec3(0.25f, 0.25f, 0.25f));
    m_environmentMapShader.set_mat4("model", model);

    glBindVertexArray(m_teapot.m_VAO);
    glDrawElements(GL_TRIANGLES, m_teapot.m_indices.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    glDepthFunc(GL_LEQUAL);

    m_skyboxShader.use();
    m_skyboxShader.set_mat4("view", glm::mat4(glm::mat3(m_view)));
    m_skyboxShader.set_mat4("projection", m_projection);

    glBindVertexArray(m_skyboxVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);

    glDepthFunc(GL_LESS);

    SDL_GL_SwapWindow(m_window);

    return Status::Ok;
}

GLLELU_MAIN_IMPLEMENTATION(Cubemaps)
