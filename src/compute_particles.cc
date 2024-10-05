#include "gllelu.hh"
#include "gllelu_main.hh"
#include "shader.hh"

#include "glad/gl.h"
#include "glm/glm.hpp"
#include "SDL3/SDL.h"

#include <cstdint>
#include <cstdlib>

#define NUM_PARTICLES 1000
#define LOCAL_SIZE_X 100

struct Particle {
    glm::vec2 position;
    glm::vec2 velocity;
};

struct Uniforms {
    float dt;
    float scale;
};

class ComputeParticles: public GLlelu
{
public:
    ComputeParticles(int argc, char *argv[]);
    virtual ~ComputeParticles();
    virtual int main_loop();
};

ComputeParticles::ComputeParticles(int argc, char *argv[]):
    GLlelu(argc, argv, GLVersion::GL43)
{
}

ComputeParticles::~ComputeParticles()
{
}

int ComputeParticles::main_loop()
{
    Particle initialParticles[NUM_PARTICLES];
    for (int i = 0; i < NUM_PARTICLES; ++i) {
        Particle p {
            glm::vec2(0.0f, 0.0f),
            glm::vec2(static_cast<float>(rand()) / static_cast<float>(RAND_MAX),
                      static_cast<float>(rand()) / static_cast<float>(RAND_MAX))
        };
        p.velocity.x = p.velocity.x * 2.0f - 1.0f;
        p.velocity.y = p.velocity.y * 2.0f - 1.0f;
        p.velocity = glm::normalize(p.velocity);
        initialParticles[i] = p;
    }

    Shader computeShader("compute_particles.comp");
    Shader drawShader("compute_particles.vert", "compute_particles.frag");

    unsigned int dummyVAO;
    glGenVertexArrays(1, &dummyVAO);

    unsigned int particleBufferA;
    glGenBuffers(1, &particleBufferA);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, particleBufferA);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(initialParticles), initialParticles, GL_DYNAMIC_COPY);

    unsigned int particleBufferB;
    glGenBuffers(1, &particleBufferB);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, particleBufferB);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(initialParticles), NULL, GL_DYNAMIC_COPY);

    unsigned int UBO;
    glGenBuffers(1, &UBO);
    glBindBuffer(GL_UNIFORM_BUFFER, UBO);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(Uniforms), NULL, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, UBO);

    glEnable(GL_PROGRAM_POINT_SIZE);

    uint64_t frameCount = 0;
    uint64_t lastFrame = 0;
    int speedScale = 0;
    bool quit = false;
    SDL_Event event;

    while (!quit) {
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_EVENT_QUIT:
                    quit = true;
                    break;
                case SDL_EVENT_KEY_UP:
                    switch (event.key.scancode) {
                        case SDL_SCANCODE_ESCAPE:
                            quit = true;
                            break;
                        case SDL_SCANCODE_RETURN:
                            glBindBuffer(GL_SHADER_STORAGE_BUFFER, particleBufferA);
                            glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(initialParticles), initialParticles);
                            glBindBuffer(GL_SHADER_STORAGE_BUFFER, particleBufferB);
                            glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(initialParticles), initialParticles);
                            break;
                        case SDL_SCANCODE_UP:
                            ++speedScale;
                            break;
                        case SDL_SCANCODE_DOWN:
                            --speedScale;
                        default:
                            break;
                    }
                    break;
                default:
                    break;
            }
        }

        uint64_t currentFrame = SDL_GetTicks();
        uint64_t delta = currentFrame - lastFrame;
        lastFrame = currentFrame;

        Uniforms uniforms {
            static_cast<float>(delta) / 1000.0f,
            1.0f + static_cast<float>(speedScale) / 10.0f
        };

        if (frameCount % 2 == 0) {
            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, particleBufferA);
            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, particleBufferB);
        } else {
            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, particleBufferA);
            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, particleBufferB);
        }

        computeShader.use();
        glBindBuffer(GL_UNIFORM_BUFFER, UBO);
        glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(Uniforms), &uniforms);

        glDispatchCompute(NUM_PARTICLES / LOCAL_SIZE_X, 1, 1);

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        drawShader.use();
        glBindVertexArray(dummyVAO);
        glDrawArrays(GL_POINTS, 0, NUM_PARTICLES);

        SDL_GL_SwapWindow(m_window);
        ++frameCount;
    }

    glDeleteVertexArrays(1, &dummyVAO);
    glDeleteBuffers(1, &particleBufferA);
    glDeleteBuffers(1, &particleBufferB);
    glDeleteBuffers(1, &UBO);

    return EXIT_SUCCESS;
}

GLLELU_MAIN_IMPLEMENTATION(ComputeParticles)
