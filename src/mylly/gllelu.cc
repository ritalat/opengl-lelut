#include "gllelu.hh"

#include "path.hh"

#if defined(USE_GLES)
#include "glad/gles2.h"
#else
#include "glad/gl.h"
#endif
#include "SDL3/SDL.h"

#include <cstdio>
#include <filesystem>
#include <stdexcept>
#include <string_view>
#if defined(__EMSCRIPTEN__)
#include <unistd.h>
#endif

#define WINDOW_WIDTH 1024
#define WINDOW_HEIGHT 768

#define WINDOW_NAME "SDL OpenGL"

GLlelu::GLlelu(int argc, char *argv[], GLVersion glVersion):
    m_window(nullptr),
    m_context(nullptr),
    m_windowSize({ WINDOW_WIDTH, WINDOW_HEIGHT }),
    m_fullscreen(false),
    m_mouseGrab(false),
    m_windowedMouseGrab(false)
{
#if defined(USE_GLES)
    (void)glVersion;
#endif
#if defined(__EMSCRIPTEN__)
    dup2(1, 2);
#endif
    if (argc > 0) {
        Path exe = Path(argv[0]).filename();
        fprintf(stderr, "Starting %s (%s)\n", WINDOW_NAME, cpath(exe));
    } else {
        fprintf(stderr, "Starting %s\n", WINDOW_NAME);
    }

    int linked = SDL_GetVersion();
    fprintf(stderr, "Compiled with: SDL %u.%u.%u\n",
                    SDL_MAJOR_VERSION, SDL_MINOR_VERSION, SDL_MICRO_VERSION);
    fprintf(stderr, "Loaded: SDL %u.%u.%u\n",
                    SDL_VERSIONNUM_MAJOR(linked),
                    SDL_VERSIONNUM_MINOR(linked),
                    SDL_VERSIONNUM_MICRO(linked));

    Path dataDir = datadir();
    if (dataDir.is_absolute()) {
        fprintf(stderr, "Data directory: %s\n", cpath(dataDir));
    } else {
        Path absolute = std::filesystem::weakly_canonical(dataDir);
        fprintf(stderr, "Data directory: %s (%s)\n", cpath(dataDir), cpath(absolute));
    }

    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMEPAD))
        throw std::runtime_error("Failed to init SDL");

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
#if defined(USE_GLES)
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,
                        SDL_GL_CONTEXT_PROFILE_ES);
#else
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, GLVERSIONMAJOR(glVersion));
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, GLVERSIONMINOR(glVersion));
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,
                        SDL_GL_CONTEXT_PROFILE_CORE);
#endif
    m_window = SDL_CreateWindow(WINDOW_NAME,
                                WINDOW_WIDTH, WINDOW_HEIGHT,
                                SDL_WINDOW_OPENGL | SDL_WINDOW_HIDDEN);
    if (!m_window)
        throw std::runtime_error("Failed to create SDL window");

    m_context = SDL_GL_CreateContext(m_window);
    if (!m_context)
        throw std::runtime_error("Failed to create OpenGL context");

#if defined(USE_GLES)
    if (!gladLoadGLES2((GLADloadfunc) SDL_GL_GetProcAddress))
        throw std::runtime_error("Failed to load OpenGL functions");
#else
    if (!gladLoadGL((GLADloadfunc) SDL_GL_GetProcAddress))
        throw std::runtime_error("Failed to load OpenGL functions");
#endif

    SDL_GetWindowSizeInPixels(m_window, &m_fbSize.width, &m_fbSize.height);
    glViewport(0, 0, m_fbSize.width, m_fbSize.height);

    SDL_GL_SetSwapInterval(1);
}

GLlelu::~GLlelu()
{
    if (m_context)
        SDL_GL_DestroyContext(m_context);

    if (m_window)
        SDL_DestroyWindow(m_window);

    SDL_Quit();
}

void GLlelu::window_name(std::string_view name)
{
    SDL_SetWindowTitle(m_window, name.data());
}

void GLlelu::window_size(int w, int h)
{
    if (!m_fullscreen) {
        SDL_SetWindowSize(m_window, m_windowSize.width, m_windowSize.height);
    }
}

void GLlelu::window_fullscreen(bool fullscreen)
{
    if (fullscreen != m_fullscreen) {
        if (fullscreen) {
            SDL_ShowWindow(m_window);
            m_windowedMouseGrab = m_mouseGrab;
            window_grab(true);
        } else {
            window_grab(m_windowedMouseGrab);
        }
    }
    m_fullscreen = fullscreen;
    SDL_SetWindowFullscreen(m_window, m_fullscreen);
}

void GLlelu::window_grab(bool grabMouse)
{
    m_mouseGrab = grabMouse;
    SDL_SetWindowRelativeMouseMode(m_window, m_mouseGrab);
}

int GLlelu::run()
{
    if (!m_window || !m_context)
        return EXIT_FAILURE;

    if (m_fullscreen) {
        fprintf(stderr, "Window size: fullscreen\n");
    } else {
        fprintf(stderr, "Window size: %dx%d\n", m_windowSize.width, m_windowSize.height);
    }
    fprintf(stderr, "Drawable size: %dx%d\n", m_fbSize.width, m_fbSize.height);

    fprintf(stderr, "OpenGL vendor: %s\n", glGetString(GL_VENDOR));
    fprintf(stderr, "OpenGL renderer: %s\n", glGetString(GL_RENDERER));
    fprintf(stderr, "OpenGL version: %s\n", glGetString(GL_VERSION));
    fprintf(stderr, "OpenGL Shading Language version: %s\n",
                    glGetString(GL_SHADING_LANGUAGE_VERSION));

    SDL_ShowWindow(m_window);
    return main_loop();
}
