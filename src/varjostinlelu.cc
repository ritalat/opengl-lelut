#include "gllelu.hh"
#include "gllelu_main.hh"
#include "shader.hh"
#include "text_bitmap.hh"

#include "glad/gl.h"
#include "SDL3/SDL.h"

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <optional>
#include <stdexcept>
#include <string>

const float vertices[] = {
    -1.0f,  1.0f, 0.0f,
    -1.0f, -1.0f, 0.0f,
     1.0f, -1.0f, 0.0f,
     1.0f,  1.0f, 0.0f
};

const unsigned int indices[] = {
    0, 1, 3,
    1, 2, 3
};

class VarjostinLelu: public GLlelu
{
public:
    VarjostinLelu(int argc, char *argv[]);
    virtual ~VarjostinLelu();
    virtual int main_loop();
    void load_shaders();

    std::optional<Shader> m_leluShader;
    std::string m_error;
};

VarjostinLelu::VarjostinLelu(int argc, char *argv[]):
    GLlelu(argc, argv)
{
}

VarjostinLelu::~VarjostinLelu()
{
}

int VarjostinLelu::main_loop()
{
    std::string notification;
    uint64_t notificationExpiration = 0;

    TextRendererLatin1 txt(m_fbSize.width, m_fbSize.height, "font8x8.png");

    load_shaders();

    unsigned int leluVAO;
    glGenVertexArrays(1, &leluVAO);
    glBindVertexArray(leluVAO);

    unsigned int leluVBO;
    glGenBuffers(1, &leluVBO);
    glBindBuffer(GL_ARRAY_BUFFER, leluVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    unsigned int leluEBO;
    glGenBuffers(1, &leluEBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, leluEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    bool quit = false;
    SDL_Event event;

    while (!quit) {
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_EVENT_QUIT:
                    quit = true;
                    break;
                case SDL_EVENT_KEY_UP:
                    if (SDL_SCANCODE_ESCAPE == event.key.scancode)
                        quit = true;

                    if (SDL_SCANCODE_RETURN == event.key.scancode) {
                        notification = "Reloading shaders...";
                        notificationExpiration = SDL_GetTicks() + 2000;
                        load_shaders();
                    }
                    break;
                default:
                    break;
            }
        }

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        if (m_leluShader) {
            m_leluShader->use();

            m_leluShader->set_vec2("u_resolution", static_cast<float>(m_fbSize.width), static_cast<float>(m_fbSize.height));
            float x, y;
            SDL_GetMouseState(&x, &y);
            m_leluShader->set_vec2("u_mouse", x, static_cast<float>(m_fbSize.height) - y);
            m_leluShader->set_float("u_time", static_cast<float>(SDL_GetTicks()) / 1000.0f);

            glBindVertexArray(leluVAO);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
            glBindVertexArray(0);
        }

        if (!m_error.empty()) {
            txt.set_scale(1.0f);
            txt.set_color(1.0f, 0.0f, 0.0f);
            txt.draw_string(0, 0, m_error);
        }

        if (notificationExpiration > SDL_GetTicks()) {
            float scale = 3.0f;
            txt.set_scale(scale);
            txt.set_color(0.0f, 1.0f, 0.0f);
            txt.draw_string(0, m_fbSize.height - static_cast<int>(scale) * FONT_SIZE, notification);
        }

        SDL_GL_SwapWindow(m_window);
    }

    glDeleteBuffers(1, &leluEBO);
    glDeleteBuffers(1, &leluVBO);
    glDeleteVertexArrays(1, &leluVAO);

    return EXIT_SUCCESS;
}

void VarjostinLelu::load_shaders()
{
    try {
        m_leluShader.emplace("varjostinlelu.vert", "varjostinlelu.frag");
        fprintf(stderr, "Varjostinlelu shaders loaded successfully\n");
        m_error.clear();
    } catch (const std::runtime_error &e) {
        fprintf(stderr, "Failed to load varjostinlelu shaders: %s\n", e.what());
        m_error = "Failed to compile varjostinlelu shaders!\n"
                  "\nException:\n"
                  + std::string(e.what()) +
                  "\nPress enter to reload.";
    }
}

GLLELU_MAIN_IMPLEMENTATION(VarjostinLelu)
