#ifndef __ENGINE_H__
#define __ENGINE_H__

#include <map>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "libs/logger/logger.h"
#include "libs/thread/thread.h"

#include "objects.h"

namespace terrain
{

namespace drawer { class Drawer; }
namespace loader { class Loader; }

namespace engine
{

using namespace std;
using namespace terrain::objects;

class Engine
{
    friend class drawer::Drawer;
    friend class loader::Loader;

    Log     &debug;
    Logger  log;

    struct GL
    {
        GLFWwindow  *window;
        GLuint      buffer[11];
        GLuint      shaders[4];
        GLuint      MVP[4];
    } gl;

    struct Local
    {
        TerrainPoint    buffer[(1 << TILE_DENSITY_BITS) * (1 << TILE_DENSITY_BITS)];
        Tile            tile[9];

        bool            viewType;
        uint8_t         lod;
        uint32_t        lodSize[TILE_DENSITY_BITS];
        uint32_t        *lodIndices[TILE_DENSITY_BITS];
        map<float, map<float, int32_t> > world;

        float           zoom;
        float           rotation;
        glm::mat4       projection;

        glm::vec3       eye;
        glm::mat4       view;

        glm::vec2       mousePressPosition;
        glm::vec2       mousePrevPosition;
    } local;

    public:
        Engine(Log &_debug);
        ~Engine(void);

        void run(int argc, char **argv);
        void terminate(void);

    private:
        void loadMap(const char *path);
        void updateViewport(void);
        void updateView(void);

        static void glfwErrorCallback(int code, const char *message);
        static void glfwKeyCallback(GLFWwindow *window, int key, int scancode, int action, int mods);
        static void glfwMouseButtonCallback(GLFWwindow *window, int button, int action, int mods);
        static void glfwMouseMoveCallback(GLFWwindow *window, double x, double y);
        static void glfwWheelCallback(GLFWwindow *window, double x, double y);
        static void glfwWindowCloseCallback(GLFWwindow *window);
}; // class Engine

} // namespace engine

} // namespace terrain

#endif // __ENGINE_H__
