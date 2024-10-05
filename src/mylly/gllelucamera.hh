#pragma once

#include "gllelu.hh"

#include "glm/glm.hpp"
#include "SDL3/SDL.h"

#include <cstdint>

enum class Status
{
    QuitFailure = -1,
    Ok = 0,
    QuitSuccess = 1
};

struct Camera
{
    glm::vec3 position = glm::vec3(0.0f, 0.0f, 3.0f);
    glm::vec3 front = glm::vec3(0.0f, 0.0f, -1.0f);
    glm::vec3 right = glm::vec3(1.0f, 0.0f, 0.0f);
    glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::vec3 worldUp = glm::vec3(0.0f, 1.0f, 0.0f);
    float fov = 45.0f;
    float sensitivity = 0.1f;
    float speed = 0.0025f;
    float yaw = -90.0f;
    float pitch = 0.0f;
    int deadzone = 2000;
};

class GLleluCamera: public GLlelu
{
public:
    GLleluCamera(int argc, char *argv[], GLVersion glVersion = GLVersion::GL33);
    virtual ~GLleluCamera();
    virtual int main_loop() final;
    void iterate();
    virtual Status event(SDL_Event &event);
    virtual Status update(uint64_t deltaTime);
    virtual Status render() = 0;
    void find_gamepad();

    glm::mat4 m_view;
    glm::mat4 m_projection;
    Camera m_camera;
    SDL_Gamepad *m_gamepad;
    SDL_JoystickID m_gamepadId;
    uint64_t m_deltaTime;
    uint64_t m_lastFrame;
    bool m_quit;
    int m_exitStatus;
};
