#include "defines.h"
#include "movement.h"

#include <cstring>
#include <cassert>
#include <algorithm>
#include <chrono>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "engine/engine.h"
#include "libs/logger/logger.h"

using namespace std;
using namespace terrain;
using namespace terrain::movement;

extern engine::Engine   engine;

void Movement::start(void)
{
    pthread_setname_np(handle.native_handle(), "Movement");

    log.debug("Starting movement");
}

void Movement::run(void)
{
    log.debug("Running movement");
    double lastFrame = 0;
    while(state == Thread::STARTED)
    {
        lastFrame = glfwGetTime();
        if(::engine.local.viewType == VIEW_FPP)
            move();

        double currentFrame = glfwGetTime();
        double diff = currentFrame - lastFrame;

        if(diff < 1.0L / MOVEMENT_FPS)
            this_thread::sleep_for(chrono::milliseconds(static_cast<uint32_t>(1000.0L / (MOVEMENT_FPS - 1) - diff * 1000.0L)));

        else if(diff > 1.0L / (MOVEMENT_FPS - 1))
            log.warning("Checking tiles took: %.4lfs", diff);
    }
}

inline
void Movement::move(void)
{
    if(glfwGetKey(::engine.gl.window, GLFW_KEY_W) == GLFW_PRESS)
        ::engine.local.eye += ::engine.local.viewpoint * 1000.0;

    if(glfwGetKey(::engine.gl.window, GLFW_KEY_S) == GLFW_PRESS)
        ::engine.local.eye -= ::engine.local.viewpoint * 1000.0;

    ::engine.updateView();
}

void Movement::stop(void)
{
}

void Movement::terminate(void)
{
}

Movement::Movement(Log &_log)
:log(_log, "MOVEMENT")
{
}

Movement::~Movement(void)
{
}
