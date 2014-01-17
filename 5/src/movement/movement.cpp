#include "defines.h"
#include "movement.h"

#include <cstring>
#include <cassert>
#include <algorithm>
#include <chrono>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/gtx/rotate_vector.hpp>

#include "engine/engine.h"
#include "libs/logger/logger.h"

using namespace std;
using namespace terrain;
using namespace terrain::movement;

Movement::Movement(Log &_log, engine::Engine &_engine)
:log(_log, "MOVEMENT")
,engine(_engine)
{
}

Movement::~Movement(void)
{
}

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
        if(engine.options.viewType == ::engine::VIEW_3D)
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
    const double speed = glfwGetKey(engine.gl.window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS ? 100000.0 : 1000.0;
    if(glfwGetKey(engine.gl.window, GLFW_KEY_W) == GLFW_PRESS)
        engine.local.d3d.eye += engine.local.d3d.direction * speed;

    if(glfwGetKey(engine.gl.window, GLFW_KEY_S) == GLFW_PRESS)
        engine.local.d3d.eye -= engine.local.d3d.direction * speed;

    if(glfwGetKey(engine.gl.window, GLFW_KEY_D) == GLFW_PRESS)
        engine.local.d3d.eye -= engine.local.d3d.right * speed;

    if(glfwGetKey(engine.gl.window, GLFW_KEY_A) == GLFW_PRESS)
        engine.local.d3d.eye += engine.local.d3d.right * speed;

    if(glfwGetKey(engine.gl.window, GLFW_KEY_Q) == GLFW_PRESS)
    {
        engine.local.d3d.right  = glm::rotate(engine.local.d3d.right, -M_PI / 180.0, engine.local.d3d.direction);
        engine.local.d3d.up     = glm::rotate(engine.local.d3d.up, -M_PI / 180.0, engine.local.d3d.direction);
    }

    if(glfwGetKey(engine.gl.window, GLFW_KEY_E) == GLFW_PRESS)
    {
        engine.local.d3d.right  = glm::rotate(engine.local.d3d.right, M_PI / 180.0, engine.local.d3d.direction);
        engine.local.d3d.up     = glm::rotate(engine.local.d3d.up, M_PI / 180.0, engine.local.d3d.direction);
    }



    engine.updateView();
}

void Movement::stop(void)
{
}

void Movement::terminate(void)
{
}

