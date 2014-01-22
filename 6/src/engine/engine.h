#ifndef __ENGINE_H__
#define __ENGINE_H__

#include <unordered_map>
#include <vector>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "libs/logger/logger.h"
#include "libs/thread/thread.h"

#include "objects.h"

namespace viewer
{

namespace drawer { class Drawer; }
namespace movement { class Movement; }

namespace engine
{

using namespace std;

enum Buffer
{
    VERTS   = 0,
    UVS     = 0,
    NORMALS = 0,
    INDICES = 0,
}; // enum Buffer

class Engine
{
    friend class drawer::Drawer;
    friend class movement::Movement;

    Log     &debug;
    Logger  log;

    struct Threads
    {
        drawer::Drawer      *drawer;
        movement::Movement  *movement;
    } threads;

    struct Options
    {
        // WINDOW SIZE
        int32_t     width;
        int32_t     height;

        double      fov;
        double      speed;

        bool        lights;
        bool        textures;

        string      basedir;
    } options;

    struct Local
    {
        struct D3D
        {
            glm::dvec3  eye;
            glm::dvec3  right;
            glm::dvec3  direction;
            glm::dvec3  up;
            glm::dmat4  projection;
            glm::dmat4  view;
        } d3d;

        objects::Bound          bound;
        vector<objects::Mesh>   mesh;
    } local;

    struct GL
    {
        // GLFW WINDOWS
        GLFWwindow  *window;

        // SHADERS
        GLuint      program;
        GLuint      MVP;

        unordered_map<string, GLuint>  texture;
    } gl;

    public:
        Engine(Log &_debug);
        ~Engine(void);

        void run(const char *path);
        void terminate(void);

    private:
        void loadModel(const char *path);
        void loadTexture(const aiScene *scene);
        void updateViewport(void);
        void updateView(void);
        glm::mat4 getUniform(void);

    public:
        // GLFW CALLBACKS
        void glfwErrorCallback(int code, const char *message);
        void glfwKeyCallback(GLFWwindow *window, int key, int scancode, int action, int mods);
        void glfwMouseMoveCallback(GLFWwindow *window, double x, double y);
        void glfwWindowCloseCallback(GLFWwindow *window);
        void glfwWindowResizeCallback(GLFWwindow *window, int _width, int _height);
}; // class Engine

} // namespace engine

} // namespace viewer

#endif // __ENGINE_H__
