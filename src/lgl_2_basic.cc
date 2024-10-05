#include "gllelucamera.hh"
#include "gllelu_main.hh"
#include "shader.hh"
#include "shapes.hh"

#include "glad/gl.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "SDL3/SDL.h"

#include <cstdio>
#include <cstdlib>

const int numCubes = 10;

const glm::vec3 cubePositions[numCubes] = {
    glm::vec3( 0.0f,  0.0f,  0.0f),
    glm::vec3( 2.0f,  5.0f, -15.0f),
    glm::vec3(-1.5f, -2.2f, -2.5f),
    glm::vec3(-3.8f, -2.0f, -12.3f),
    glm::vec3( 2.4f, -0.4f, -3.5f),
    glm::vec3(-1.7f,  3.0f, -7.5f),
    glm::vec3( 1.3f, -2.0f, -2.5f),
    glm::vec3( 1.5f,  2.0f, -2.5f),
    glm::vec3( 1.5f,  0.2f, -1.5f),
    glm::vec3(-1.3f,  1.0f, -1.5f)
};

struct BasicMaterial {
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
    float shininess;
};

// http://devernay.free.fr/cours/opengl/materials.html
const BasicMaterial cubeMaterials[numCubes] = {
    {   // Emerald
        glm::vec3( 0.0215f,  0.1745f,  0.0215f),
        glm::vec3( 0.07568f,  0.61424f,  0.07568f),
        glm::vec3( 0.633f,  0.727811f,  0.633f),
        128.0f * 0.6f
    },
    {   // Pearl
        glm::vec3( 0.25f,  0.20725f,  0.20725f),
        glm::vec3( 1.0f,  0.829f,  0.829f),
        glm::vec3( 0.296648f,  0.296648f,  0.296648f),
        128.0f * 0.088f
    },
    {   // Bronze
        glm::vec3( 0.2125f,  0.1275f,  0.054f),
        glm::vec3( 0.714f,  0.4284f,  0.18144f),
        glm::vec3( 0.393548f,  0.271906f,  0.166721f),
        128.0f * 0.2f
    },
    {   // Gold
        glm::vec3( 0.24725f,  0.1995f,  0.0745f),
        glm::vec3( 0.75164f,  0.60648f,  0.22648f),
        glm::vec3( 0.628281f,  0.555802f,  0.366065f),
        128.0f * 0.4f
    },
    {   // Cyan Plastic
        glm::vec3( 0.0f,  0.1f,  0.06f),
        glm::vec3( 0.0f,  0.50980392f,  0.50980392f),
        glm::vec3( 0.50196078f,  0.50196078f,  0.50196078f),
        128.0f * 0.25f
    },
    {   // Red Plastic
        glm::vec3( 0.0f,  0.0f,  0.0f),
        glm::vec3( 0.5f,  0.0f,  0.0f),
        glm::vec3( 0.7f,  0.6f,  0.6f),
        128.0f * 0.25f
    },
    {   // Green Rubber
        glm::vec3( 0.0f,  0.05f,  0.0f),
        glm::vec3( 0.4f,  0.5f,  0.4f),
        glm::vec3( 0.04f,  0.7f,  0.04f),
        128.0f * 0.078125f
    },
    {   // Yellow Rubber
        glm::vec3( 0.05f,  0.05f,  0.0f),
        glm::vec3( 0.5f,  0.5f,  0.4f),
        glm::vec3( 0.7f,  0.7f,  0.04f),
        128.0f * 0.078125f
    },
    {   // Silver
        glm::vec3( 0.19225f,  0.19225f,  0.19225f),
        glm::vec3( 0.50754f,  0.50754f,  0.50754f),
        glm::vec3( 0.508273f,  0.508273f,  0.508273f),
        128.0f * 0.4f
    },
    {   // Ruby
        glm::vec3( 0.1745f,  0.01175f,  0.01175f),
        glm::vec3( 0.61424f,  0.04136f,  0.04136f),
        glm::vec3( 0.727811f,  0.626959f,  0.626959f),
        128.0f * 0.6f
    }
};

const glm::vec3 lightPos(1.2f, 1.0f, 2.0f);

class LGL_2_Basic: public GLleluCamera
{
public:
    LGL_2_Basic(int argc, char *argv[]);
    virtual ~LGL_2_Basic();
    virtual Status render();

    Shader m_lightingShader;
    Shader m_lightShader;
    unsigned int m_VAO;
    unsigned int m_lightVAO;
    unsigned int m_VBO;
};

LGL_2_Basic::LGL_2_Basic(int argc, char *argv[]):
    GLleluCamera(argc, argv),
    m_lightingShader("lighting.vert", "lighting_basic.frag"),
    m_lightShader("lighting.vert", "lighting_light.frag"),
    m_VAO(0),
    m_lightVAO(0),
    m_VBO(0)
{
    glGenVertexArrays(1, &m_VAO);
    glBindVertexArray(m_VAO);

    glGenBuffers(1, &m_VBO);
    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);

    glBufferData(GL_ARRAY_BUFFER, sizeof(cube), cube, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glGenVertexArrays(1, &m_lightVAO);
    glBindVertexArray(m_lightVAO);

    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

LGL_2_Basic::~LGL_2_Basic()
{
    glDeleteVertexArrays(1, &m_VAO);
    glDeleteVertexArrays(1, &m_lightVAO);
    glDeleteBuffers(1, &m_VBO);
}

Status LGL_2_Basic::render()
{
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_lightingShader.use();

    m_lightingShader.set_mat4("view", m_view);
    m_lightingShader.set_mat4("projection", m_projection);

    m_lightingShader.set_vec3("light.ambient", 1.0f, 1.0f, 1.0f);
    m_lightingShader.set_vec3("light.diffuse", 1.0f, 1.0f, 1.0f);
    m_lightingShader.set_vec3("light.specular", 1.0f, 1.0f, 1.0f);
    m_lightingShader.set_vec3("light.position", lightPos);

    m_lightingShader.set_vec3("viewPos", m_camera.position);

    glm::mat4 model = glm::mat4(1.0f);

    glBindVertexArray(m_VAO);
    for (int i = 0; i < numCubes; ++i) {
        model = glm::mat4(1.0f);
        model = glm::translate(model, cubePositions[i]);
        float angle = 20.0f * static_cast<float>(i);
        model = glm::rotate(model, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
        m_lightingShader.set_mat4("model", model);

        m_lightingShader.set_vec3("material.ambient", cubeMaterials[i].ambient);
        m_lightingShader.set_vec3("material.diffuse", cubeMaterials[i].diffuse);
        m_lightingShader.set_vec3("material.specular", cubeMaterials[i].specular);
        m_lightingShader.set_float("material.shininess", cubeMaterials[i].shininess);

        glDrawArrays(GL_TRIANGLES, 0, 36);
    }

    m_lightShader.use();

    m_lightShader.set_mat4("view", m_view);
    m_lightShader.set_mat4("projection", m_projection);

    model = glm::mat4(1.0f);
    model = glm::translate(model, lightPos);
    model = glm::scale(model, glm::vec3(0.2f));
    m_lightShader.set_mat4("model", model);

    glBindVertexArray(m_lightVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);

    SDL_GL_SwapWindow(m_window);

    return Status::Ok;
}

GLLELU_MAIN_IMPLEMENTATION(LGL_2_Basic)
