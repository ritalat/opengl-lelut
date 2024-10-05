#include "gllelu.hh"
#include "gllelu_main.hh"
#include "shader.hh"

#if defined(__EMSCRIPTEN__)
#include <emscripten/emscripten.h>
#endif
#include "glad/gles2.h"
#include "SDL3/SDL.h"

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <string>

const float vertices[] = {
     0.0f,  0.5f, 0.0f, 0.0f, 0.0f, 1.0f,
    -0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f,
     0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f
};

class TriangleES3: public GLlelu
{
public:
    TriangleES3(int argc, char *argv[]);
    virtual ~TriangleES3();
    virtual int main_loop();
    void iterate();

    Shader triangleShader;
    unsigned int VAO;
    unsigned int VBO;
    bool quit;
};

#if defined(__EMSCRIPTEN__)
void browser_callback(void *arg)
{
    static_cast<TriangleES3*>(arg)->iterate();
}
#endif

TriangleES3::TriangleES3(int argc, char *argv[]):
    GLlelu(argc, argv),
    triangleShader("es3_triangle.vert", "es3_triangle.frag"),
    quit(false)
{
}

TriangleES3::~TriangleES3()
{
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
}

int TriangleES3::main_loop()
{
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

#if defined(__EMSCRIPTEN__)
    emscripten_set_main_loop_arg(browser_callback, this, 0, 1);
#else
    while (!quit) {
        iterate();
    }
#endif

    return EXIT_SUCCESS;
}

void TriangleES3::iterate()
{
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_EVENT_QUIT:
                quit = true;
                break;
            case SDL_EVENT_KEY_UP:
                if (SDL_SCANCODE_ESCAPE == event.key.scancode)
                    quit = true;
                break;
            default:
                break;
        }
    }

#if defined(__EMSCRIPTEN__)
    if(quit)
        emscripten_cancel_main_loop();
#endif

    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    triangleShader.use();

    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, 3);

    SDL_GL_SwapWindow(m_window);
}

GLLELU_MAIN_IMPLEMENTATION(TriangleES3)
