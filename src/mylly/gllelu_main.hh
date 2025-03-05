#pragma once

#ifdef __ANDROID__
#define GLLELU_MAIN_IMPLEMENTATION(Lelu)
#else

#define SDL_MAIN_USE_CALLBACKS
#include "SDL3/SDL.h"
#include "SDL3/SDL_main.h"

#include <cstdio>
#include <cstdlib>
#include <exception>

#define GLLELU_MAIN_IMPLEMENTATION(Lelu)                                                        \
class Lelu;                                                                                     \
                                                                                                \
SDL_AppResult SDL_AppInit(void **appstate, int argc, char **argv)                               \
{                                                                                               \
    try {                                                                                       \
        *appstate = new Lelu(argc, argv);                                                       \
        return SDL_APP_CONTINUE;                                                                \
    } catch (const std::exception &e) {                                                         \
        fprintf(stderr, "\nUnhandled exception: %s\n", e.what());                               \
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Unhandled exception!", e.what(), NULL); \
        return SDL_APP_FAILURE;                                                                 \
    }                                                                                           \
}                                                                                               \
                                                                                                \
SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event)                                    \
{                                                                                               \
    try {                                                                                       \
        return static_cast<Lelu *>(appstate)->event(event);                                     \
    } catch (const std::exception &e) {                                                         \
        fprintf(stderr, "\nUnhandled exception: %s\n", e.what());                               \
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Unhandled exception!", e.what(), NULL); \
        return SDL_APP_FAILURE;                                                                 \
    }                                                                                           \
}                                                                                               \
                                                                                                \
SDL_AppResult SDL_AppIterate(void *appstate)                                                    \
{                                                                                               \
    try {                                                                                       \
        return static_cast<Lelu *>(appstate)->iterate();                                        \
    } catch (const std::exception &e) {                                                         \
        fprintf(stderr, "\nUnhandled exception: %s\n", e.what());                               \
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Unhandled exception!", e.what(), NULL); \
        return SDL_APP_FAILURE;                                                                 \
    }                                                                                           \
}                                                                                               \
                                                                                                \
void SDL_AppQuit(void *appstate, SDL_AppResult result)                                          \
{                                                                                               \
    (void)result;                                                                               \
    try {                                                                                       \
        if (appstate) {                                                                         \
            delete static_cast<Lelu *>(appstate);                                               \
            appstate = nullptr;                                                                 \
        }                                                                                       \
    } catch (const std::exception &e) {                                                         \
        fprintf(stderr, "\nUnhandled exception: %s\n", e.what());                               \
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Unhandled exception!", e.what(), NULL); \
    }                                                                                           \
}

#endif
