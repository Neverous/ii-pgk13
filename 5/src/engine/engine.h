#ifndef __ENGINE_H__
#define __ENGINE_H__

#include <unordered_map>
#include <vector>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include "libs/logger/logger.h"
#include "libs/thread/thread.h"

#include "objects.h"
#include "hgt/map.h"

namespace terrain
{

namespace drawer { class Drawer; }
namespace loader { class Loader; }
namespace movement { class Movement; }

namespace engine
{

using namespace std;
using namespace terrain::objects;

enum Buffers
{
    TILE_BUFFER_1   = 0,
    TILE_BUFFER_2   = 1,
    TILE_BUFFER_3   = 2,
    TILE_BUFFER_4   = 3,
    TILE_BUFFER_5   = 4,
    TILE_BUFFER_6   = 5,
    TILE_BUFFER_7   = 6,
    TILE_BUFFER_8   = 7,
    TILE_BUFFER_9   = 8,

    GRID_BUFFER     = 9,
    SWAP_BUFFER_1   = 10,
    SWAP_BUFFER_2   = 11,
    SWAP_BUFFER_3   = 12,
    SWAP_BUFFER_4   = 13,
    SWAP_BUFFER_5   = 14,
    SWAP_BUFFER_6   = 15,
    SWAP_BUFFER_7   = 16,
    SWAP_BUFFER_8   = 17,
    SWAP_BUFFER_9   = 18
}; // enum Buffers

enum ViewType
{
    VIEW_2D = 0,
    VIEW_3D = 1,
}; // enum ViewType

class Engine
{
    friend class drawer::Drawer;
    friend class loader::Loader;
    friend class movement::Movement;

    Log     &debug;
    Logger  log;

    struct Threads
    {
        drawer::Drawer      *drawer;
        loader::Loader      *loader;
        movement::Movement  *movement;
    } threads;

    struct Options
    {
        // WINDOW SIZE
        int32_t     width;
        int32_t     height;

        uint8_t     lod;
        ViewType    viewType;
        double      fov;
    } options;

    struct Local
    {
        Tile            tile[9];

        uint32_t        gridSize[DETAIL_LEVELS];
        uint32_t        tileSize[DETAIL_LEVELS];
        unordered_map<int16_t, unordered_map<int16_t, hgt::Map> > world;

        struct D2D
        {
            double      zoom;
            glm::dquat  rotation;
            glm::dvec3  eye;
            glm::dmat4  projection;
            glm::dmat4  view;
        } d2d;

        struct D3D
        {
            glm::dvec3  eye;
            glm::dvec3  direction;
            glm::dvec3  up;
            glm::dmat4  projection;
            glm::dmat4  view;
        } d3d;

        struct Mouse
        {
            glm::dvec2  press;
            glm::dvec2  prev;
        } mouse;

        struct Bound
        {
            struct Min
            {
                double x;
                double y;
            } min;

            struct Max
            {
                double x;
                double y;
            } max;
        } bound;
    } local;

    struct GL
    {
        // GLFW WINDOWS
        GLFWwindow  *loader;
        GLFWwindow  *window;

        // SHADERS
        GLuint      program[2][2];
        GLuint      MVP[2][2];
        GLuint      BOX[2][2];

        // INDICES
        GLuint      gridIndice[DETAIL_LEVELS];
        GLuint      tileIndice[DETAIL_LEVELS];

        // BUFFERS
        GLuint      buffer[19];
    } gl;

    public:
        Engine(Log &_debug);
        ~Engine(void);

        void run(int argc, char **argv);
        void terminate(void);

    private:
        void loadMap(const char *path);
        void parseMapFilename(char *path, char *&filename, int32_t &lat, int32_t &lon);

        void updateViewport(void);
        void updateViewport2D(void);
        void updateViewport3D(void);

        void updateView(void);
        void updateView2D(void);
        void updateView3D(void);

        void changeViewType(void);
        void setupView2D(void);
        void setupView3D(void);

        glm::dvec4 getBoundingRect(void);
        glm::dvec4 getBoundingRect2D(void);
        glm::dvec4 getBoundingRect3D(void);

        glm::mat4 getUniform(void);
        glm::mat4 getUniform2D(void);
        glm::mat4 getUniform3D(void);

    public:
        // GLFW CALLBACKS
        void glfwErrorCallback(int code, const char *message);
        void glfwKeyCallback(GLFWwindow *window, int key, int scancode, int action, int mods);
        void glfwMouseButtonCallback(GLFWwindow *window, int button, int action, int mods);
        void glfwMouseMoveCallback(GLFWwindow *window, double x, double y);
        void mouseMove2D(double x, double y);
        void mouseMove3D(double x, double y);
        void glfwWheelCallback(GLFWwindow *window, double x, double y);
        void glfwWindowCloseCallback(GLFWwindow *window);
        void glfwWindowResizeCallback(GLFWwindow *window, int _width, int _height);
}; // class Engine

} // namespace engine

} // namespace terrain

#endif // __ENGINE_H__
