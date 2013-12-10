#ifndef __ENGINE_H__
#define __ENGINE_H__

#include <stdexcept>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "libs/logger/logger.h"
#include "libs/config/config.h"
#include "libs/thread/thread.h"

#include "objects.h"

using namespace std;

namespace walker
{

class Animations;
class Drawer;

class Engine
{
    friend class Animations;
    friend class Drawer;

    Log     &debug;
    Logger  log;

    Animations  *animations;
    Drawer      *drawer;

    struct GL
    {
        GLFWwindow  *window;
        GLuint      buffer;
        GLuint      shaders;
        GLuint      MVP;
    } gl;

    struct Local
    {
        unsigned int    view;
        unsigned int    figures;
        unsigned int    points;
        Figure          figure[1024];
        Point           point[32768];
    } local;

    public:
        Engine(Log &_debug);
        ~Engine(void);

        void run(void);
        void terminate(void);

    private:
        Point *reservePoints(const int count, GLuint &index);

        static void glfwErrorCallback(int code, const char *message);
        static void glfwKeyCallback(GLFWwindow *window, int key, int scancode, int action, int mods);
        static void glfwWindowCloseCallback(GLFWwindow *window);
}; // class Engine

}; // namespace walker

#include "engine.inl"

#endif // __ENGINE_H__
