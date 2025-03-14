#include "camera.hh"
#include "gllelu.hh"
#include "gllelu_main.hh"
#include "model.hh"
#include "shader.hh"
#include "texture.hh"

#include "glad/gles2.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "SDL3/SDL.h"

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

class CubemapsES3: public GLlelu
{
public:
    CubemapsES3(int argc, char *argv[]);
    virtual ~CubemapsES3();
    virtual SDL_AppResult event(SDL_Event *event);
    virtual SDL_AppResult iterate();

private:
    Camera m_camera;
    Shader m_skyboxShader;
    Shader m_environmentMapShader;
    Model m_teapot;
    Cubemap m_skybox;
    unsigned int m_skyboxVAO;
    unsigned int m_skyboxVBO;
};

CubemapsES3::CubemapsES3(int argc, char *argv[]):
    GLlelu(argc, argv),
    m_camera(this),
    m_skyboxShader("es3_skybox.vert", "es3_skybox.frag"),
    m_environmentMapShader("es3_environment_map.vert", "es3_environment_map.frag"),
    m_teapot("teapot.obj"),
    m_skybox(skyboxImages),
    m_skyboxVAO(0),
    m_skyboxVBO(0)
{
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    m_skyboxShader.use();
    m_skyboxShader.setInt("skybox", 0);
    m_environmentMapShader.use();
    m_environmentMapShader.setInt("skybox", 0);

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

CubemapsES3::~CubemapsES3()
{
    glDeleteVertexArrays(1, &m_skyboxVAO);
    glDeleteBuffers(1, &m_skyboxVBO);
}

SDL_AppResult CubemapsES3::event(SDL_Event *event)
{
    m_camera.event(event);
    return GLlelu::event(event);
}

SDL_AppResult CubemapsES3::iterate()
{
    m_camera.iterate();

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_skybox.activate(0);

    m_environmentMapShader.use();
    m_environmentMapShader.setMat4("view", m_camera.view());
    m_environmentMapShader.setMat4("projection", m_camera.projection());
    m_environmentMapShader.setVec3("viewPos", m_camera.data.position);

    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(0.0f, -0.5f, 0.0f));
    model = glm::rotate(model, glm::radians(static_cast<float>(SDL_GetTicks()) / 10.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::scale(model, glm::vec3(0.25f, 0.25f, 0.25f));
    m_environmentMapShader.setMat4("model", model);
    m_environmentMapShader.setMat4("normalMat", glm::transpose(glm::inverse(model)));

    glBindVertexArray(m_teapot.vao());
    glDrawElements(GL_TRIANGLES, m_teapot.indices(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    glDepthFunc(GL_LEQUAL);

    m_skyboxShader.use();
    m_skyboxShader.setMat4("view", glm::mat4(glm::mat3(m_camera.view())));
    m_skyboxShader.setMat4("projection", m_camera.projection());

    glBindVertexArray(m_skyboxVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);

    glDepthFunc(GL_LESS);

    return GLlelu::iterate();
}

GLLELU_MAIN_IMPLEMENTATION(CubemapsES3)
