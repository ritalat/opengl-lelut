#include "gllelu.hh"
#include "gllelu_main.hh"
#include "shader.hh"
#include "text_bitmap.hh"

#include "glad/gles2.h"
#include "SDL3/SDL.h"

#include <cmath>
#include <cstdlib>
#include <string>

#if defined(__ANDROID__)
#define SCALE 2
#else
#define SCALE 1
#endif

class BitmapFontES3: public GLlelu
{
public:
    BitmapFontES3(int argc, char *argv[]);
    virtual ~BitmapFontES3();
    virtual SDL_AppResult event(SDL_Event *event);
    virtual SDL_AppResult iterate();

private:
    TextRendererLatin1 m_txt;
    std::string m_fontLoaded;
    std::string m_glInfoDump;
    std::string m_aakkosia;
};

BitmapFontES3::BitmapFontES3(int argc, char *argv[]):
    GLlelu(argc, argv),
    m_txt(fbSize().width, fbSize().height, "font8x8.png"),
    m_fontLoaded("Loaded font font8x8.png"),
    m_glInfoDump("OpenGL vendor: " + std::string((char *)glGetString(GL_VENDOR)) + '\n' +
                 "OpenGL renderer: " + std::string((char *)glGetString(GL_RENDERER)) + '\n' +
                 "OpenGL version: " + std::string((char *)glGetString(GL_VERSION)) + '\n' +
                 "OpenGL Shading Language version: " + std::string((char *)glGetString(GL_SHADING_LANGUAGE_VERSION))),
    m_aakkosia("Ääkkösetkin toimii :---DD")
{
}

BitmapFontES3::~BitmapFontES3()
{
}

SDL_AppResult BitmapFontES3::event(SDL_Event *event)
{
    switch (event->type) {
        case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
            if (event->window.windowID == windowId()) {
                m_txt.setWindowSize(fbSize().width, fbSize().height);
            }
            break;

        default:
            break;
    }

    return GLlelu::event(event);
}

SDL_AppResult BitmapFontES3::iterate()
{
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    SDL_Rect safe;
    SDL_GetWindowSafeArea(SDL_GetWindowFromID(windowId()), &safe);

    m_txt.setColor(1.0f, 1.0f, 1.0f);
    m_txt.setScale(1.0f * SCALE);
    m_txt.drawString(safe.x, safe.y, m_glInfoDump);
    m_txt.drawString(safe.w - static_cast<int>(m_fontLoaded.length()) * FONT_SIZE * SCALE,
                     safe.h - FONT_SIZE * SCALE, m_fontLoaded);

    m_txt.setColor(1.0f, 0.0f, 0.0f);
    m_txt.setScale(2.5f * SCALE);
    m_txt.drawString(safe.x + 250 + static_cast<int>(100.0f * sin(static_cast<float>(SDL_GetTicks()) / 1000.0f)),
                     safe.h / 2, m_aakkosia);

    float x, y;
    unsigned int buttons = SDL_GetMouseState(&x, &y);
    if (buttons & SDL_BUTTON_MASK(SDL_BUTTON_LEFT)) {
        m_txt.setColor(0.0f, 1.0f, 0.0f);
    } else if (buttons & SDL_BUTTON_MASK(SDL_BUTTON_RIGHT)) {
        m_txt.setColor(0.0f, 0.0f, 1.0f);
    } else if (buttons & SDL_BUTTON_MASK(SDL_BUTTON_MIDDLE)) {
        m_txt.setColor(1.0f, 0.0f, 0.0f);
    } else {
        m_txt.setColor(1.0f, 1.0f, 1.0f);
    }
    m_txt.setScale(1.0f * SCALE);
    std::string mouse = "Mouse state: (" + std::to_string(static_cast<int>(x))
                        + "," + std::to_string(static_cast<int>(y)) + ")";
    m_txt.drawString(safe.x, safe.h - FONT_SIZE * SCALE, mouse);

    return GLlelu::iterate();
}

GLLELU_MAIN_IMPLEMENTATION(BitmapFontES3)
