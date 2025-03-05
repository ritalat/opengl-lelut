#include "camera.hh"
#include "gllelu.hh"
#include "model.hh"
#include "shader.hh"
#include "text_bitmap.hh"
#include "texture.hh"

#define SDL_MAIN_USE_CALLBACKS
#include "SDL3/SDL.h"
#include "SDL3/SDL_main.h"

#include <cstddef>
#include <cstdio>

// Write something less horrific at some point :)

class TriangleES3: public GLlelu
{
public:
    TriangleES3(int argc, char *argv[]);
    virtual ~TriangleES3();
    virtual SDL_AppResult iterate();

private:
    Shader m_triangleShader;
    unsigned int m_VAO;
    unsigned int m_VBO;
};

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

const char *samples[] {
    "triangle",
    "bitmap_font",
    "cubemaps"
};

SDL_AppResult SDL_AppInit(void **appstate, int argc, char **argv)
{
    try {
        SDL_MessageBoxButtonData buttons[] = {
            { 0, -1, "prev" },
            { 0, 0, "launch" },
            { 0, 1, "next" },
        };
        SDL_MessageBoxData messageBoxData = {
            SDL_MESSAGEBOX_INFORMATION,
            NULL,
            "Select a sample",
            samples[0],
            SDL_arraysize(buttons),
            buttons,
            NULL
        };

        int buttonId = 1;
        int selectedSample = 0;

        while (buttonId != 0) {
            messageBoxData.message = samples[selectedSample];
            if (!SDL_ShowMessageBox(&messageBoxData, &buttonId)) {
                fprintf(stderr, "Failed to show message box\n");
                return SDL_APP_FAILURE;
            }
            if (buttonId == 1) {
                ++selectedSample;
                if (selectedSample >= sizeof(samples) / sizeof(char*))
                    selectedSample = 0;
            }
            if (buttonId == -1) {
                --selectedSample;
                if (selectedSample < 0)
                    selectedSample = sizeof(samples) / sizeof(char*) - 1;
            }
        }

        switch (selectedSample) {
            case 0: {
                *appstate = new TriangleES3(argc, argv);
                break;
            }
            case 1: {
                *appstate = new BitmapFontES3(argc, argv);
                break;
            }
            case 2: {
                *appstate = new CubemapsES3(argc, argv);
                break;
            }
            default: {
                fprintf(stderr, "Unknown sample\n");
                return SDL_APP_FAILURE;
            }
        }

        return SDL_APP_CONTINUE;
    } catch (const std::exception &e) {
        fprintf(stderr, "\nUnhandled exception: %s\n", e.what());
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Unhandled exception!", e.what(), NULL);
        return SDL_APP_FAILURE;
    }
}

SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event)
{
    try {
        return static_cast<GLlelu *>(appstate)->event(event);
    } catch (const std::exception &e) {
        fprintf(stderr, "\nUnhandled exception: %s\n", e.what());
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Unhandled exception!", e.what(), NULL);
        return SDL_APP_FAILURE;
    }
}

SDL_AppResult SDL_AppIterate(void *appstate)
{
    try {
        return static_cast<GLlelu *>(appstate)->iterate();
    } catch (const std::exception &e) {
        fprintf(stderr, "\nUnhandled exception: %s\n", e.what());
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Unhandled exception!", e.what(), NULL);
        return SDL_APP_FAILURE;
    }
}

void SDL_AppQuit(void *appstate, SDL_AppResult result)
{
    (void)result;
    try {
        if (appstate) {
            delete static_cast<GLlelu *>(appstate);
            appstate = nullptr;
        }
    } catch (const std::exception &e) {
        fprintf(stderr, "\nUnhandled exception: %s\n", e.what());
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Unhandled exception!", e.what(), NULL);
    }
}
