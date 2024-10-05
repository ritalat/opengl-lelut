#pragma once

#include "SDL3/SDL.h"

#include <string_view>

#define GLVERSION(major, minor) (major << 4 | minor)
#define GLVERSIONMAJOR(glver) (int)glver >> 4
#define GLVERSIONMINOR(glver) (int)glver & 0b1111

enum class GLVersion
{
    GL20 = GLVERSION(2, 0),
    GL21 = GLVERSION(2, 1),
    GL30 = GLVERSION(3, 0),
    GL31 = GLVERSION(3, 1),
    GL32 = GLVERSION(3, 2),
    GL33 = GLVERSION(3, 3),
    GL40 = GLVERSION(4, 0),
    GL41 = GLVERSION(4, 1),
    GL42 = GLVERSION(4, 2),
    GL43 = GLVERSION(4, 3),
    GL44 = GLVERSION(4, 4),
    GL45 = GLVERSION(4, 5),
    GL46 = GLVERSION(4, 6)
};

struct Size
{
    int width;
    int height;
};

class GLlelu
{
public:
    GLlelu(int argc, char *argv[], GLVersion glVersion = GLVersion::GL33);
    virtual ~GLlelu();
    void window_name(std::string_view name);
    void window_size(int w, int h);
    void window_fullscreen(bool fullscreen);
    void window_grab(bool grabMouse);
    int run();
    virtual int main_loop() = 0;

    SDL_Window *m_window;
    SDL_GLContext m_context;
    Size m_windowSize;
    Size m_fbSize;
    bool m_fullscreen;
    bool m_mouseGrab;
    bool m_windowedMouseGrab;
};
