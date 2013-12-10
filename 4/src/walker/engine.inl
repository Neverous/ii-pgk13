#ifndef __ENGINE_INL__
#define __ENGINE_INL__

#include "engine.h"

#include <cmath>
#include <stdexcept>
#include <cassert>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "libs/logger/logger.h"
#include "libs/config/config.h"
#include "libs/thread/thread.h"
#include "libs/file/chunkFileReader.h"

#include "objects.h"
#include "animations.h"
#include "drawer.h"

using namespace std;

extern walker::Engine engine;

namespace walker
{

inline
Engine::Engine(Log &_debug)
:debug(_debug)
,log(_debug, "ENGINE")
,animations(new Animations(_debug, this))
,drawer(new Drawer(_debug, this))
,gl()
,local()
{
    srand(time(nullptr));
    // Connect glfw error handler
    glfwSetErrorCallback(Engine::glfwErrorCallback);

    if(!glfwInit())
        throw runtime_error("GLFWInit error");

    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    if(!(gl.window = glfwCreateWindow(800, 600, "walker", nullptr, nullptr)))
    {
        glfwDestroyWindow(gl.window);
        gl.window = nullptr;

        glfwTerminate();
        throw runtime_error("GLFWCreateWindow error");
    }

    glfwSetWindowCloseCallback(gl.window, Engine::glfwWindowCloseCallback);
    glfwSetKeyCallback(gl.window, Engine::glfwKeyCallback);

    Color COLOR[15] = {
        {_GL_COLOR(0xC4), _GL_COLOR(0xA0), _GL_COLOR(0x00)},
        {_GL_COLOR(0xED), _GL_COLOR(0xD4), _GL_COLOR(0x00)},
        {_GL_COLOR(0xCE), _GL_COLOR(0x5C), _GL_COLOR(0x00)},
        {_GL_COLOR(0xF5), _GL_COLOR(0x79), _GL_COLOR(0x00)},
        {_GL_COLOR(0x4E), _GL_COLOR(0x9A), _GL_COLOR(0x06)},
        {_GL_COLOR(0x73), _GL_COLOR(0xD2), _GL_COLOR(0x16)},
        {_GL_COLOR(0x20), _GL_COLOR(0x4A), _GL_COLOR(0x87)},
        {_GL_COLOR(0x34), _GL_COLOR(0x65), _GL_COLOR(0xA4)},
        {_GL_COLOR(0x5C), _GL_COLOR(0x35), _GL_COLOR(0x66)},
        {_GL_COLOR(0x75), _GL_COLOR(0x50), _GL_COLOR(0x7B)},
        {_GL_COLOR(0xA4), _GL_COLOR(0x00), _GL_COLOR(0x00)},
        {_GL_COLOR(0xCC), _GL_COLOR(0x00), _GL_COLOR(0x00)},
        {_GL_COLOR(0x8F), _GL_COLOR(0x59), _GL_COLOR(0x02)},
        {_GL_COLOR(0x55), _GL_COLOR(0x57), _GL_COLOR(0x53)},
        {_GL_COLOR(0xEE), _GL_COLOR(0xEE), _GL_COLOR(0xEC)},
    };

    local.figures = 256;
    for(unsigned int f = 0; f < local.figures; ++ f)
    {
        Figure &figure = local.figure[f];
        figure.var = rand() % 32;
        figure.position.x = 1.0f * f / local.figures * 20;
        figure.position.y = 1.0f;
        figure.position.z = 0.0f;
        figure.points = 2;
        figure.local = reservePoints(figure.points, figure.index);
        memcpy(&figure.local[0].R, &COLOR[f % 15], sizeof(Color));
        memcpy(&figure.local[1].R, &COLOR[f % 15], sizeof(Color));
        figure.local[0].x = figure.local[0].y = figure.local[0].z = 0.0f;
        figure.local[1].x = 1.00f;
        figure.local[1].y = figure.local[1].z = 0.0f;

        // "NECK"
        figure.bone[0].x = 0.0f; figure.bone[0].y = -1.0f / 8.0f; figure.bone[0].z = 0.0f;

        // TORSO
        figure.bone[1].x = 0.0f; figure.bone[1].y = -13.0f / 24.0f; figure.bone[1].z = 0.0f;

        // RIGHT LEG
        figure.bone[2].x = -0.02f; figure.bone[2].y = -1.0f / 4.0f; figure.bone[2].z = 0.0f;
        figure.bone[3].x = 0.0f; figure.bone[3].y = -1.0f / 4.0f; figure.bone[3].z = 0.0f;

        figure.bone[10].x = 0.0f; figure.bone[10].y = 0.0f; figure.bone[10].z = 1.0f / 20.0f;

        // LEFT LEG
        figure.bone[4].x = 0.02f; figure.bone[4].y = -1.0f / 4.0f; figure.bone[4].z = 0.0f;
        figure.bone[5].x = 0.0f; figure.bone[5].y = -1.0f / 4.0f; figure.bone[5].z = 0.0f;

        figure.bone[11].x = 0.0f; figure.bone[11].y = 0.0f; figure.bone[11].z = 1.0f / 20.0f;

        // RIGHT ARM
        figure.bone[6].x = -0.02f; figure.bone[6].y = -1.0f / 5.0f; figure.bone[6].z = 0.0f;
        figure.bone[7].x = 0.0f; figure.bone[7].y = 0.0f; figure.bone[7].z = 1.0f / 8.0f;

        // LEFT ARM
        figure.bone[8].x = 0.02f; figure.bone[8].y = -1.0f / 5.0f; figure.bone[8].z = 0.0f;
        figure.bone[9].x = 0.0f; figure.bone[9].y = 0.0f; figure.bone[9].z = 1.0f / 8.0f;

    }
}

inline
Engine::~Engine(void)
{
    delete animations;
    delete drawer;
}

inline
void Engine::run(void)
{
    log.info("Engine running");
    threads.activate();
    while(!glfwWindowShouldClose(gl.window))
        glfwWaitEvents();

    log.info("Engine finished");
}

inline
void Engine::terminate(void)
{
    glfwSetWindowShouldClose(gl.window, GL_TRUE);
    threads.deactivate();
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

        case GLFW_KEY_V:
        case GLFW_KEY_TAB:
            if(action == GLFW_PRESS)
            {
                ++ engine.local.view;
                if(engine.local.view == 4)
                    engine.local.view = 0;

            }

            break;
    }
}

inline
void Engine::glfwWindowCloseCallback(GLFWwindow */*window*/)
{
    engine.terminate();
}

inline
Point *Engine::reservePoints(const int count, GLuint &index)
{
    Point *result = local.point + local.points;
    index = local.points;
    local.points += count;
    return result;
}

}; // namespace walker

#endif // __ENGINE_INL__
