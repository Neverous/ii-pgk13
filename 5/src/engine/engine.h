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
        GLFWwindow  *loader;
        GLFWwindow  *window;
        GLuint      buffer[11];
        GLuint      shaders[4];
        GLuint      MVP[4];
        GLuint      BOX[4];
        GLuint      lodIndices[TILE_DENSITY_BITS];
    } gl;

    struct Local
    {
        TerrainPoint    buffer[(1 << TILE_DENSITY_BITS) * (1 << TILE_DENSITY_BITS)];
        Tile            tile[9];

        bool            viewType;
        uint8_t         lod;
        uint32_t        lodSize[TILE_DENSITY_BITS];
        map<double, map<double, int32_t> > world;

        double          zoom;
        double          rotation;
        glm::dmat4       projection;

        glm::dvec3       eye;
        glm::dmat4       view;

        glm::dvec2       mousePressPosition;
        glm::dvec2       mousePrevPosition;

        struct Bound
        {
            double  minX;
            double  maxX;

            double  minY;
            double  maxY;
        } bound;
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
        glm::dvec4 getView(void);

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
