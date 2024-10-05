#include "gllelucamera.hh"

#if defined(__EMSCRIPTEN__)
#include <emscripten/emscripten.h>
#endif
#if defined(USE_GLES)
#include "glad/gles2.h"
#else
#include "glad/gl.h"
#endif
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "SDL3/SDL.h"

#include <cmath>
#include <cstdint>
#include <cstdlib>

#if defined(__EMSCRIPTEN__)
void browser_callback(void *arg)
{
    static_cast<GLleluCamera*>(arg)->iterate();
}
#endif

GLleluCamera::GLleluCamera(int argc, char *argv[], GLVersion glVersion):
    GLlelu(argc, argv, glVersion),
    m_gamepad(NULL),
    m_gamepadId(0),
    m_deltaTime(0),
    m_lastFrame(0),
    m_quit(false),
    m_exitStatus(EXIT_SUCCESS)
{
    find_gamepad();
}

GLleluCamera::~GLleluCamera()
{
    if (m_gamepad)
        SDL_CloseGamepad(m_gamepad);
}

int GLleluCamera::main_loop()
{
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

#if defined(__EMSCRIPTEN__)
    emscripten_set_main_loop_arg(browser_callback, this, 0, 1);
#else
    while (!m_quit) {
        iterate();
    }
#endif

    return m_exitStatus;
}

void GLleluCamera::iterate()
{
    uint64_t currentFrame = SDL_GetTicks();
    m_deltaTime = currentFrame - m_lastFrame;
    m_lastFrame = currentFrame;
    float deltaf = static_cast<float>(m_deltaTime);
    float cameraSpeedScaled = m_camera.speed * deltaf;
    float cameraSensitivityScaled = m_camera.sensitivity * deltaf;

    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        switch (e.type) {
            case SDL_EVENT_QUIT:
                m_quit = true;
                break;

            case SDL_EVENT_KEY_UP:
                if (SDL_SCANCODE_ESCAPE == e.key.scancode)
                    m_quit = true;
                if (SDL_SCANCODE_F == e.key.scancode)
                    window_fullscreen(!m_fullscreen);
                if (SDL_SCANCODE_G == e.key.scancode)
                    window_grab(!m_mouseGrab);
                break;

            case SDL_EVENT_MOUSE_MOTION:
                if (m_mouseGrab || SDL_BUTTON_LMASK & e.motion.state) {
                    m_camera.yaw += static_cast<float>(e.motion.xrel) * m_camera.sensitivity;
                    m_camera.pitch -= static_cast<float>(e.motion.yrel) * m_camera.sensitivity;
                    if (m_camera.pitch > 89.0f)
                        m_camera.pitch = 89.0f;
                    if (m_camera.pitch < -89.0f)
                        m_camera.pitch = -89.0f;
                }
                break;

            case SDL_EVENT_MOUSE_WHEEL:
                m_camera.fov -= float(e.wheel.y);
                if (m_camera.fov < 1.0f)
                    m_camera.fov = 1.0f;
                if (m_camera.fov > 100.0f)
                    m_camera.fov = 100.0f;
                break;

            case SDL_EVENT_GAMEPAD_BUTTON_UP:
                if (m_gamepad && e.gbutton.which == m_gamepadId) {
                    if (SDL_GAMEPAD_BUTTON_EAST == e.gbutton.button)
                        m_quit = true;
                    if (SDL_GAMEPAD_BUTTON_NORTH == e.gbutton.button)
                        window_fullscreen(!m_fullscreen);
                }
                break;

            case SDL_EVENT_GAMEPAD_ADDED:
                if (!m_gamepad) {
                    m_gamepad = SDL_OpenGamepad(e.gdevice.which);
                    m_gamepadId = SDL_GetJoystickID(SDL_GetGamepadJoystick(m_gamepad));
                }
                break;

            case SDL_EVENT_GAMEPAD_REMOVED:
                if (m_gamepad && e.cdevice.which == m_gamepadId) {
                    SDL_CloseGamepad(m_gamepad);
                    m_gamepad = NULL;
                    m_gamepadId = 0;
                    find_gamepad();
                }
                break;

            case SDL_EVENT_WINDOW_RESIZED:
                if (e.window.windowID == SDL_GetWindowID(m_window)) {
                    SDL_GetWindowSize(m_window, &m_windowSize.width, &m_windowSize.height);
                }
                break;

            case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
                if (e.window.windowID == SDL_GetWindowID(m_window)) {
                    SDL_GetWindowSizeInPixels(m_window, &m_fbSize.width, &m_fbSize.height);
                    glViewport(0, 0, m_fbSize.width, m_fbSize.height);
                }
                break;

            default:
                break;
        }

        Status ret = event(e);
        if (ret != Status::Ok) {
            m_quit = true;
            m_exitStatus = ret == Status::QuitSuccess ? EXIT_SUCCESS : EXIT_FAILURE;
        }
    }

#if defined(__EMSCRIPTEN__)
    if(m_quit)
        emscripten_cancel_main_loop();
#endif

    m_camera.front = glm::normalize(glm::vec3(
        cos(glm::radians(m_camera.yaw)) * cos(glm::radians(m_camera.pitch)),
        sin(glm::radians(m_camera.pitch)),
        sin(glm::radians(m_camera.yaw)) * cos(glm::radians(m_camera.pitch))
    ));
    m_camera.right = glm::normalize(glm::cross(m_camera.front, m_camera.worldUp));
    m_camera.up = glm::normalize(glm::cross(m_camera.right, m_camera.front));

    bool keyboardMovement = false;
    const bool *keystates = SDL_GetKeyboardState(NULL);

    if (keystates[SDL_SCANCODE_LSHIFT]) {
        cameraSpeedScaled *= 2.0f;
        keyboardMovement = true;
    }
    if (keystates[SDL_SCANCODE_W]) {
        m_camera.position += cameraSpeedScaled * m_camera.front;
        keyboardMovement = true;
    }
    if (keystates[SDL_SCANCODE_A]) {
        m_camera.position -= cameraSpeedScaled * m_camera.right;
        keyboardMovement = true;
    }
    if (keystates[SDL_SCANCODE_S]) {
        m_camera.position -= cameraSpeedScaled * m_camera.front;
        keyboardMovement = true;
    }
    if (keystates[SDL_SCANCODE_D]) {
        m_camera.position += cameraSpeedScaled * m_camera.right;
        keyboardMovement = true;
    }
    if (keystates[SDL_SCANCODE_SPACE]) {
        m_camera.position += cameraSpeedScaled * m_camera.worldUp;
        keyboardMovement = true;
    }
    if (keystates[SDL_SCANCODE_LCTRL]) {
        m_camera.position -= cameraSpeedScaled * m_camera.worldUp;
        keyboardMovement = true;
    }

    if (m_gamepad && !keyboardMovement) {
        if (SDL_GetGamepadButton(m_gamepad, SDL_GAMEPAD_BUTTON_SOUTH)) {
            cameraSpeedScaled *= 2.0f;
        }
        if (SDL_GetGamepadButton(m_gamepad, SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER)) {
            m_camera.fov -= (0.1f * deltaf);
        }
        if (SDL_GetGamepadButton(m_gamepad, SDL_GAMEPAD_BUTTON_LEFT_SHOULDER)) {
            m_camera.fov += (0.1f * deltaf);
        }
        if (m_camera.fov < 1.0f) {
            m_camera.fov = 1.0f;
        }
        if (m_camera.fov > 100.0f) {
            m_camera.fov = 100.0f;
        }

        auto scale_axis = [](Sint16 axis){ return axis / 32767.0f; };
        Sint16 axis = 0;

        axis = SDL_GetGamepadAxis(m_gamepad, SDL_GAMEPAD_AXIS_RIGHT_TRIGGER);
        if (axis > m_camera.deadzone) {
            m_camera.position += cameraSpeedScaled * m_camera.worldUp * scale_axis(axis);
        }
        axis = SDL_GetGamepadAxis(m_gamepad, SDL_GAMEPAD_AXIS_LEFT_TRIGGER);
        if (axis > m_camera.deadzone) {
            m_camera.position -= cameraSpeedScaled * m_camera.worldUp * scale_axis(axis);
        }

        axis = SDL_GetGamepadAxis(m_gamepad, SDL_GAMEPAD_AXIS_LEFTX);
        if (abs(axis) > m_camera.deadzone) {
            m_camera.position += cameraSpeedScaled * m_camera.right * scale_axis(axis);
        }
        axis = SDL_GetGamepadAxis(m_gamepad, SDL_GAMEPAD_AXIS_LEFTY);
        if (abs(axis) > m_camera.deadzone) {
            m_camera.position -= cameraSpeedScaled * m_camera.front * scale_axis(axis);
        }

        axis = SDL_GetGamepadAxis(m_gamepad, SDL_GAMEPAD_AXIS_RIGHTX);
        if (abs(axis) > m_camera.deadzone) {
            m_camera.yaw += cameraSensitivityScaled * scale_axis(axis);
        }
        axis = SDL_GetGamepadAxis(m_gamepad, SDL_GAMEPAD_AXIS_RIGHTY);
        if (abs(axis) > m_camera.deadzone) {
            m_camera.pitch -= cameraSensitivityScaled * scale_axis(axis);
        }
        if (m_camera.pitch > 89.0f) {
            m_camera.pitch = 89.0f;
        }
        if (m_camera.pitch < -89.0f) {
            m_camera.pitch = -89.0f;
        }
    }

    m_view = glm::lookAt(m_camera.position, m_camera.position + m_camera.front, m_camera.up);
    m_projection = glm::perspective(glm::radians(m_camera.fov),
                                    static_cast<float>(m_fbSize.width) / static_cast<float>(m_fbSize.height),
                                    0.1f, 100.0f);

    Status ret = update(m_deltaTime);
    if (ret != Status::Ok) {
        m_quit = true;
        m_exitStatus = ret == Status::QuitSuccess ? EXIT_SUCCESS : EXIT_FAILURE;
    }

    ret = render();
    if (ret != Status::Ok) {
        m_quit = true;
        m_exitStatus = ret == Status::QuitSuccess ? EXIT_SUCCESS : EXIT_FAILURE;
    }
}

Status GLleluCamera::event(SDL_Event &event)
{
    (void)event;
    return Status::Ok;
}

Status GLleluCamera::update(uint64_t deltaTime)
{
    (void)deltaTime;
    return Status::Ok;
}

void GLleluCamera::find_gamepad()
{
    int numJoysticks;
    SDL_JoystickID *joysticks = SDL_GetJoysticks(&numJoysticks);
    if (joysticks) {
        for (int i = 0; i < numJoysticks; ++i) {
            if (SDL_IsGamepad(i)) {
                m_gamepad = SDL_OpenGamepad(i);
                m_gamepadId = SDL_GetJoystickID(SDL_GetGamepadJoystick(m_gamepad));
                break;
            }
        }
        SDL_free(joysticks);
    }
}
