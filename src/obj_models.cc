#include "gllelucamera.hh"
#include "gllelu_main.hh"
#include "himmeli.hh"
#include "shader.hh"
#include "shapes.hh"

#include "glad/gl.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "SDL.h"

const glm::vec3 lightPos(1.2f, 1.0f, 2.0f);

const BasicMaterial emerald {
    glm::vec3( 0.0215f,  0.1745f,  0.0215f),
    glm::vec3( 0.07568f,  0.61424f,  0.07568f),
    glm::vec3( 0.633f,  0.727811f,  0.633f),
    128.0f * 0.6f
};

const BasicMaterial gold {
    glm::vec3( 0.24725f,  0.1995f,  0.0745f),
    glm::vec3( 0.75164f,  0.60648f,  0.22648f),
    glm::vec3( 0.628281f,  0.555802f,  0.366065f),
    128.0f * 0.4f
};

class ObjFiles: public GLleluCamera
{
public:
    ObjFiles(int argc, char *argv[]);
    virtual ~ObjFiles();

protected:
    virtual Status render();

private:
    Shader m_lightingShader;
    Shader m_lightingShaderBasic;
    Shader m_lightShader;
    Himmeli m_room;
    BasicHimmeli m_monkey;
    BasicHimmeli m_teapot;
    unsigned int m_lightVAO;
    unsigned int m_lightVBO;
};

ObjFiles::ObjFiles(int argc, char *argv[]):
    GLleluCamera(argc, argv),
    m_lightingShader("lighting.vert", "lighting_textured.frag"),
    m_lightingShaderBasic("lighting.vert", "lighting_basic.frag"),
    m_lightShader("light.vert", "light.frag"),
    m_room("vt_viking_room.obj", "vt_viking_room.png"),
    m_monkey("suzanne.obj", gold),
    m_teapot("teapot.obj", emerald),
    m_lightVAO(0),
    m_lightVBO(0)
{
    m_room.translate(glm::vec3(0.0f, -0.5f, 0.0f));
    m_room.rotate(glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    m_room.rotate(glm::radians(-90.0f), glm::vec3(0.0f, 0.0f, 1.0f));

    m_monkey.translate(glm::vec3(0.475f, -0.3f, 0.0f));
    m_monkey.rotate(glm::radians(-45.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    m_monkey.rotate(glm::radians(-45.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    m_monkey.scale(glm::vec3(0.1f, 0.1f, 0.1f));

    m_teapot.translate(glm::vec3(0.0f, 0.5f, 0.0f));
    m_teapot.scale(glm::vec3(0.1f, 0.1f, 0.1f));

    glGenVertexArrays(1, &m_lightVAO);
    glBindVertexArray(m_lightVAO);

    glGenBuffers(1, &m_lightVBO);
    glBindBuffer(GL_ARRAY_BUFFER, m_lightVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cube), cube, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

ObjFiles::~ObjFiles()
{
    glDeleteVertexArrays(1, &m_lightVAO);
    glDeleteBuffers(1, &m_lightVBO);
}

Status ObjFiles::render()
{
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_lightingShader.use();
    m_lightingShader.setMat4("view", view());
    m_lightingShader.setMat4("projection", projection());

    m_lightingShader.setVec3("light.ambient", 0.2f, 0.2f, 0.2f);
    m_lightingShader.setVec3("light.diffuse", 0.5f, 0.5f, 0.5f);
    m_lightingShader.setVec3("light.specular", 1.0f, 1.0f, 1.0f);
    m_lightingShader.setVec3("light.position", lightPos);
    m_lightingShader.setVec3("viewPos", camera().position);

    m_room.draw(m_lightingShader);

    m_lightingShaderBasic.use();
    m_lightingShaderBasic.setMat4("view", view());
    m_lightingShaderBasic.setMat4("projection", projection());

    m_lightingShaderBasic.setVec3("light.ambient", 1.0f, 1.0f, 1.0f);
    m_lightingShaderBasic.setVec3("light.diffuse", 1.0f, 1.0f, 1.0f);
    m_lightingShaderBasic.setVec3("light.specular", 1.0f, 1.0f, 1.0f);
    m_lightingShaderBasic.setVec3("light.position", lightPos);
    m_lightingShaderBasic.setVec3("viewPos", camera().position);

    m_monkey.draw(m_lightingShaderBasic);
    m_teapot.rotate(glm::mat4(1.0f),
                    glm::radians(static_cast<float>(SDL_GetTicks()) / 10.0f),
                    glm::vec3(0.0f, 1.0f, 0.0f));
    m_teapot.draw(m_lightingShaderBasic);

    m_lightShader.use();
    m_lightShader.setMat4("view", view());
    m_lightShader.setMat4("projection", projection());

    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, lightPos);
    model = glm::scale(model, glm::vec3(0.2f));
    m_lightShader.setMat4("model", model);

    glBindVertexArray(m_lightVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);

    swapWindow();

    return Status::Ok;
}

GLLELU_MAIN_IMPLEMENTATION(ObjFiles)
