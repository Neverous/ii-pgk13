#ifndef __ENGINE_H__
#define __ENGINE_H__

#include <stdexcept>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "libs/logger/logger.h"
#include "libs/config/config.h"
#include "libs/thread/thread.h"

using namespace std;

namespace arkanoid
{

class Physics;
class Drawer;

class Engine
{
    friend class Physics;
    friend class Drawer;

    Log     &debug;
    Logger  log;

    Physics *physics;
    Drawer  *drawer;

    struct
    {
        unsigned int resW;
        unsigned int resH;
    } options;

    struct
    {
        GLFWwindow *window;
    } glfw;

    public:
        Engine(Log &_debug);
        ~Engine(void);

        void configure(Config &cfg);
        void run(void);
        void terminate(void);

    private:
        void movePaddle(int direction);
        void releaseBall(void);

        static void glfwErrorCallback(int code, const char *message);
        static void glfwKeyCallback(GLFWwindow *window, int key, int scancode, int action, int mods);
        static void glfwWindowCloseCallback(GLFWwindow *window);
}; // class Engine

}; // namespace arkanoid

#include "engine.inl"

#endif // __ENGINE_H__
