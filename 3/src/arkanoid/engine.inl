#ifndef __ENGINE_INL__
#define __ENGINE_INL__

#include "engine.h"

#include <stdexcept>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "libs/logger/logger.h"
#include "libs/config/config.h"
#include "libs/thread/thread.h"

//#include "objects.h"
#include "physics.h"
#include "drawer.h"

using namespace std;

extern arkanoid::Engine engine;

namespace arkanoid
{

inline
Engine::Engine(Log &_debug)
:debug(_debug)
,log(_debug, "ENGINE")
,physics(new Physics(_debug, this))
,drawer(new Drawer(_debug, this))
,options()
,glfw()
{
}

inline
Engine::~Engine(void)
{
    delete physics;
    delete drawer;
}

inline
void Engine::configure(Config &cfg)
{
    // Connect glfw error handler
    glfwSetErrorCallback(Engine::glfwErrorCallback);

    options.resW = cfg.get<unsigned int>("engine", "width", 800);
    options.resH = cfg.get<unsigned int>("engine", "height", 600);

    if(!glfwInit())
        throw runtime_error("GLFWInit error");

    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    if(!(glfw.window = glfwCreateWindow(options.resW, options.resH, "arkanoid", nullptr, nullptr)))
    {
        glfwDestroyWindow(glfw.window);
        glfw.window = nullptr;

        glfwTerminate();
        throw runtime_error("GLFWCreateWindow error");
    }

    glfwSetWindowCloseCallback(glfw.window, Engine::glfwWindowCloseCallback);
    glfwSetKeyCallback(glfw.window, Engine::glfwKeyCallback);

    log.debug("Configuring drawer");
    drawer->configure(cfg);
    log.notice("Drawer configured");

    log.debug("Configuring physics");
    physics->configure(cfg);
    log.debug("Physics configured");
}

inline
void Engine::run(void)
{
    log.info("Engine running");
    threads.activate();
    while(!glfwWindowShouldClose(glfw.window))
        glfwWaitEvents();

    log.info("Engine finished");
}

inline
void Engine::terminate(void)
{
    glfwSetWindowShouldClose(glfw.window, GL_TRUE);
    threads.deactivate();
}

inline
void Engine::movePaddle(int/* direction*/)
{
    // FIXME
}

inline
void Engine::releaseBall(void)
{
    // FIXME
}

/* GFLW CALLBACKS */
inline
void Engine::glfwErrorCallback(int code, const char *message)
{
    engine.debug.error("GLFW", "Error %d: %s", code, message);
}

inline
void Engine::glfwKeyCallback(GLFWwindow */*window*/, int key, int/* scancode*/, int action, int/* mods*/)
{
    switch(key)
    {
        case GLFW_KEY_ESCAPE:
        case GLFW_KEY_Q:
            if(action == GLFW_PRESS)
                engine.terminate();

            break;

        case GLFW_KEY_LEFT:
            if(action == GLFW_PRESS)
                engine.movePaddle(-1);

            else if(action == GLFW_RELEASE)
                engine.movePaddle(1);

            break;

        case GLFW_KEY_RIGHT:
            if(action == GLFW_PRESS)
                engine.movePaddle(1);

            else if(action == GLFW_RELEASE)
                engine.movePaddle(0);

            break;

        case GLFW_KEY_SPACE:
            engine.releaseBall();
            break;
    }
}

inline
void Engine::glfwWindowCloseCallback(GLFWwindow */*window*/)
{
    engine.terminate();
}

}; // namespace arkanoid

#endif // __ENGINE_INL__
