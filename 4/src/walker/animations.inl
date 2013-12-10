#ifndef __ANIMATIONS_INL__
#define __ANIMATIONS_INL__

#include "animations.h"

#include "libs/logger/logger.h"
#include "libs/config/config.h"
#include "libs/thread/thread.h"

#include <glm/glm.hpp>
#include <glm/gtx/rotate_vector.hpp>

#define ANIMATIONS_FPS 60

using namespace std;

namespace walker
{

inline
void Animations::start(void)
{
    pthread_setname_np(handle.native_handle(), "Animations");

    log.debug("Starting animations");
    for(unsigned int f = 0; f < engine->local.figures; ++ f)
    {
        Figure &figure = engine->local.figure[f];
        unsigned int _step = figure.var / 10 + 1;
        { // ARMS
            float deg = -45.0f;
            // RIGHT ARM
            figure.bone[6] = glm::rotate(figure.bone[6], deg / _step, glm::vec3(1.0f, 0.0f, 0.0f));
            figure.bone[7] = glm::rotate(figure.bone[7], deg / _step, glm::vec3(1.0f, 0.0f, 0.0f));

            // LEFT ARM
            figure.bone[8] = glm::rotate(figure.bone[8], -deg / _step, glm::vec3(1.0f, 0.0f, 0.0f));
            figure.bone[9] = glm::rotate(figure.bone[9], -deg / _step, glm::vec3(1.0f, 0.0f, 0.0f));
        }

        { // LEGS
            float deg = -20.0f;

            // RIGHT LEG
            figure.bone[2] = glm::rotate(figure.bone[2], -deg / _step, glm::vec3(1.0f, 0.0f, 0.0f));

            // LEFT LEG
            figure.bone[4] = glm::rotate(figure.bone[4], deg / _step, glm::vec3(1.0f, 0.0f, 0.0f));
        }
    }
}

inline
void Animations::run(void)
{
    log.debug("Running animations");
    double lastFrame = 0;
    while(state == Thread::STARTED)
    {
        lastFrame = glfwGetTime();
        step();

        double currentFrame = glfwGetTime();
        double diff = currentFrame - lastFrame;
        if(diff <= 1.L / ANIMATIONS_FPS)
            this_thread::sleep_for(chrono::milliseconds(static_cast<unsigned int>(1000.L / ANIMATIONS_FPS - diff * 1000.L)));

        else
            log.warning("Animations step took: %.4lfs", diff);
    }
}

inline
void Animations::stop(void)
{
}

inline
void Animations::terminate(void)
{
}

inline
Animations::Animations(Log &_log, Engine *_engine)
:log(_log, "ANIMATIONS")
,engine(_engine)
,s(0)
{
}

inline
Animations::~Animations(void)
{
}

inline
void Animations::step(void)
{
    ++ s;
    // ANIMATE
    for(unsigned int f = 0; f < engine->local.figures; ++ f)
    {
        Figure &figure = engine->local.figure[f];
        unsigned int _step = figure.var / 10 + 1;
        { // MOVEMENT
            figure.position.z += 0.01f / _step;
        }

        { // HEAD
            float _diff = 0.002f;
            if(s % 40 >= 20)
                _diff = -0.002f;

            figure.position.y += _diff / _step * 4;
        }


        { // ARMS
            float deg = 1.0f;
            if(s % 40 >= 20)
                deg = -1.0f;

            // RIGHT ARM
            figure.bone[6] = glm::rotate(figure.bone[6], deg / _step * 4, glm::vec3(1.0f, 0.0f, 0.0f));
            figure.bone[7] = glm::rotate(figure.bone[7], deg / _step * 4, glm::vec3(1.0f, 0.0f, 0.0f));

            // LEFT ARM
            figure.bone[8] = glm::rotate(figure.bone[8], -deg / _step * 4, glm::vec3(1.0f, 0.0f, 0.0f));
            figure.bone[9] = glm::rotate(figure.bone[9], -deg / _step * 4, glm::vec3(1.0f, 0.0f, 0.0f));
        }

        { // LEGS
            float deg = 1.0f;
            if(s % 40 >= 20)
                deg = -1.0f;

            // RIGHT LEG
            figure.bone[2] = glm::rotate(figure.bone[2], -deg / _step * 2, glm::vec3(1.0f, 0.0f, 0.0f));

            // LEFT LEG
            figure.bone[4] = glm::rotate(figure.bone[4], deg / _step * 2, glm::vec3(1.0f, 0.0f, 0.0f));
        }

        { // LEGS
            float deg = 0.5f;
            if(s % 40 >= 20)
                deg = -0.5f;

            // RIGHT LEG
            figure.bone[3] = glm::rotate(figure.bone[3], -deg / _step, glm::vec3(1.0f, 0.0f, 0.0f));

            // LEFT LEG
            figure.bone[5] = glm::rotate(figure.bone[5], deg / _step, glm::vec3(1.0f, 0.0f, 0.0f));
        }
    }
}

} // namespace walker

#endif // __ANIMATIONS_H__
