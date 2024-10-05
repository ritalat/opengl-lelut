#include "gllelu.hh"
#include "gllelu_main.hh"
#include "shader.hh"
#include "text_bitmap.hh"

#if defined(__EMSCRIPTEN__)
#include <emscripten/emscripten.h>
#endif
#include "glad/gles2.h"
#include "SDL3/SDL.h"

#include <cmath>
#include <cstdlib>
#include <string>

class BitmapFontES3: public GLlelu
{
public:
    BitmapFontES3(int argc, char *argv[]);
    virtual ~BitmapFontES3();
    virtual int main_loop();
    void iterate();

    TextRendererLatin1 txt;
    std::string fontLoaded;
    std::string glInfoDump;
    std::string aakkosia;
    bool quit;
};

#if defined(__EMSCRIPTEN__)
void browser_callback(void *arg)
{
    static_cast<BitmapFontES3*>(arg)->iterate();
}
#endif

BitmapFontES3::BitmapFontES3(int argc, char *argv[]):
    GLlelu(argc, argv),
    txt(m_fbSize.width, m_fbSize.height, "font8x8.png"),
    fontLoaded("Loaded font font8x8.png"),
    glInfoDump("OpenGL vendor: " + std::string((char *)glGetString(GL_VENDOR)) + '\n' +
               "OpenGL renderer: " + std::string((char *)glGetString(GL_RENDERER)) + '\n' +
               "OpenGL version: " + std::string((char *)glGetString(GL_VERSION)) + '\n' +
               "OpenGL Shading Language version: " + std::string((char *)glGetString(GL_SHADING_LANGUAGE_VERSION))),
    aakkosia("Ääkkösetkin toimii :---DD"),
    quit(false)
{
}

BitmapFontES3::~BitmapFontES3()
{
}

int BitmapFontES3::main_loop()
{
#if defined(__EMSCRIPTEN__)
    emscripten_set_main_loop_arg(browser_callback, this, 0, 1);
#else
    while (!quit) {
        iterate();
    }
#endif

    return EXIT_SUCCESS;
}

void BitmapFontES3::iterate()
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

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    txt.set_color(1.0f, 1.0f, 1.0f);
    txt.set_scale(1.0f);
    txt.draw_string(0, 0, glInfoDump);
    txt.draw_string(m_fbSize.width - static_cast<int>(fontLoaded.length()) * FONT_SIZE,
                    m_fbSize.height - FONT_SIZE, fontLoaded);

    txt.set_color(1.0f, 0.0f, 0.0f);
    txt.set_scale(2.5f);
    txt.draw_string(250 + static_cast<int>(100.0f * sin(static_cast<float>(SDL_GetTicks()) / 1000.0f)),
                    m_fbSize.height / 2, aakkosia);

    float x, y;
    unsigned int buttons = SDL_GetMouseState(&x, &y);
    if (buttons & SDL_BUTTON_MASK(SDL_BUTTON_LEFT)) {
        txt.set_color(0.0f, 1.0f, 0.0f);
    } else if (buttons & SDL_BUTTON_MASK(SDL_BUTTON_RIGHT)) {
        txt.set_color(0.0f, 0.0f, 1.0f);
    } else if (buttons & SDL_BUTTON_MASK(SDL_BUTTON_MIDDLE)) {
        txt.set_color(1.0f, 0.0f, 0.0f);
    } else {
        txt.set_color(1.0f, 1.0f, 1.0f);
    }
    txt.set_scale(1.0f);
    std::string mouse = "Mouse state: (" + std::to_string(static_cast<int>(x))
                        + "," + std::to_string(static_cast<int>(y)) + ")";
    txt.draw_string(0, m_fbSize.height - FONT_SIZE, mouse);

    SDL_GL_SwapWindow(m_window);
}

GLLELU_MAIN_IMPLEMENTATION(BitmapFontES3)
