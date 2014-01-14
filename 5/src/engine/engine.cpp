#include "defines.h"
#include "engine.h"

#include <cstring>
#include <cstdio>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "glm/vector_angle.h"

#include "hgt/file.h"
#include "projection/mercator.h"

using namespace terrain;
using namespace terrain::engine;
using namespace terrain::objects;
using namespace terrain::projection;

extern terrain::engine::Engine engine;

Engine::Engine(Log &_debug)
:debug(_debug)
,log(_debug, "ENGINE")
,gl()
,local()
{
    srand(time(nullptr));

    // Connect glfw error handler
    glfwSetErrorCallback(Engine::glfwErrorCallback);

    if(!glfwInit())
        throw runtime_error("GLFWInit error!");

    glfwWindowHint(GLFW_VISIBLE,                false);
    if(!(gl.loader = glfwCreateWindow(1, 1, "loader", nullptr, nullptr)))
    {
        glfwTerminate();
        throw runtime_error("GLFWCreateWindow error!");
    }

    glfwWindowHint(GLFW_VISIBLE,                true);
    glfwWindowHint(GLFW_RESIZABLE,              false);
    glfwWindowHint(GLFW_SAMPLES,                4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR,  2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR,  0);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glEnable(GL_MULTISAMPLE);

    if(!(gl.window = glfwCreateWindow(800, 600, "terrain", nullptr, gl.loader)))
    {
        glfwTerminate();
        throw runtime_error("GLFWCreateWindow error!");
    }

    glfwSetWindowCloseCallback(gl.window, Engine::glfwWindowCloseCallback);

    glfwSetKeyCallback(gl.window,           Engine::glfwKeyCallback);
    glfwSetMouseButtonCallback(gl.window,   Engine::glfwMouseButtonCallback);
    glfwSetCursorPosCallback(gl.window,     Engine::glfwMouseMoveCallback);
    glfwSetScrollCallback(gl.window,        Engine::glfwWheelCallback);

    local.zoom = 1.0;
    local.eye = glm::dvec3(0.0, 0.0, 1.0);
    local.bound.maxX = local.bound.maxY = -32000000.0;
    local.bound.minX = local.bound.minY = 32000000.0;

    updateViewport();
    updateView();
}

Engine::~Engine(void)
{
}

void Engine::run(int argc, char **argv)
{
    log.notice("Loading engine");
    log.debug("Loading maps");
    for(int a = 1; a < argc; ++ a)
        loadMap(argv[a]);

    local.eye = glm::dvec3((local.bound.maxX + local.bound.minX) / 2.0, (local.bound.maxY + local.bound.minY) / 2.0, 1.0);
    updateView();
    threads.activate();
    log.debug("Running main loop");
    while(!glfwWindowShouldClose(gl.window))
        glfwWaitEvents();
}

void Engine::terminate(void)
{
    glfwSetWindowShouldClose(gl.window, GL_TRUE);
    threads.deactivate();
}

inline
void Engine::loadMap(const char *path)
{
    char parse[1024]    = {},
         *filename      = nullptr,
         *extension     = nullptr,
         nPos[2]        = {},
         nDeg[4]        = {},
         wPos[2]        = {},
         wDeg[4]        = {};

    int32_t lat = 0,
            lon = 0;

    log.debug("Loading %s map", path);
    strncpy(parse, path, 1023);
    filename = basename(parse);
    if(!(extension = strrchr(filename, '.')) || strcmp(extension + 1, "hgt"))
        throw runtime_error("Invalid map file type");

    log.debug("Trying to load %s square", filename);
    *extension = 0;
    sscanf(filename, "%2[nNsS]%4[0123456789]%2[wWeE]%4[0123456789]", nPos, nDeg, wPos, wDeg);
    sscanf(nDeg, "%d", &lat);
    sscanf(wDeg, "%d", &lon);
    switch(nPos[0])
    {
        case 'n':
        case 'N':
            lat *= -1;
            break;

        case 's':
        case 'S':
            break;

        default:
            throw runtime_error("Invalid map file name");
            break;
    }

    switch(wPos[0])
    {
        case 'w':
        case 'W':
            lon *= -1;
            break;

        case 'e':
        case 'E':
            break;

        default:
            throw runtime_error("Invalid map file name");
            break;
    }

    log.debug("Square position %d %d", lon, lat);
    hgt::File map(path);
    for(int h = 0; h <= 1200; ++ h)
        for(int w = 0; w <= 1200; ++ w)
        {
            double x = mercator::lonToMet(lon + 1.0 * w / 1200);
            double y = mercator::latToMet(lat + 1.0 * h / 1200);
            local.bound.maxX = max(local.bound.maxX, x);
            local.bound.maxY = max(local.bound.maxY, y);
            local.bound.minX = min(local.bound.minX, x);
            local.bound.minY = min(local.bound.minY, y);
            local.world[y][x] = map.get(w, h) + 900;
        }
}

inline
void Engine::updateViewport(void)
{
    double wres = 400.0 / local.zoom;
    double hres = 300.0 / local.zoom;
    local.projection = glm::rotate(glm::ortho(-wres, wres, -hres, hres, 0.0, 10.0), local.rotation, glm::dvec3(0.0, 0.0, 1.0));
    //log.debug("VIEWPORT: %.3f %.3f %.3f %.3f ZOOM: %.3f ROTATION: %.3f", -wres, wres, -hres, hres, local.zoom, local.rotation);
}

inline
void Engine::updateView(void)
{
    local.view = glm::lookAt(local.eye, glm::dvec3(glm::dvec2(local.eye), 0.0), glm::dvec3(0.0, 1.0, 0.0));
    //log.debug("VIEW: %.3f %.3f", local.eye.x, local.eye.y);
}

glm::dvec4 Engine::getView(void)
{
    double res = 500.0 / local.zoom;
    return glm::dvec4(local.eye.x -res, local.eye.x + res, local.eye.y - res, local.eye.y + res);
}

/* GLFW CALLBACKS */

void Engine::glfwErrorCallback(int code, const char *message)
{
    ::engine.debug.error("GLFW", "Error %d: %s", code, message);
}

void Engine::glfwKeyCallback(GLFWwindow */*window*/, int/* key*/, int/* scancode*/, int/* action*/, int/* mods*/)
{
    ::engine.log.debug("Key cb");
}

void Engine::glfwMouseButtonCallback(GLFWwindow *window, int button, int action, int/* mods*/)
{
    if(::engine.local.viewType != VIEW_MAP)
        return;

    //::engine.log.debug("Button cb");
    if(action == GLFW_PRESS) switch(button)
    {
        case GLFW_MOUSE_BUTTON_LEFT:
        case GLFW_MOUSE_BUTTON_RIGHT:
            {
                double x, y;
                glfwGetCursorPos(window, &x, &y);
                ::engine.local.mousePrevPosition = ::engine.local.mousePressPosition = glm::dvec2(x, y);
            }
            break;

        default:
            return;
    }

    if(action == GLFW_RELEASE) switch(button)
    {
        case GLFW_MOUSE_BUTTON_MIDDLE:
            ::engine.local.rotation = 0;
            ::engine.updateViewport();
            break;

        default:
            return;
    }
}

void Engine::glfwMouseMoveCallback(GLFWwindow *window, double x, double y)
{
    if(::engine.local.viewType == VIEW_FPP)
    {
        ::engine.log.debug("Move cb");
        return;
    }

    if( glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) != GLFW_PRESS
    &&  glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) != GLFW_PRESS)
        return;

    //::engine.log.debug("Move cb");
    glm::dvec2 mouseCurPosition(x, y);
    if(glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
    {
        glm::dvec2 diff = glm::rotate(mouseCurPosition - ::engine.local.mousePrevPosition, ::engine.local.rotation) / ::engine.local.zoom;
        diff.x *= -1;

        ::engine.local.eye += glm::dvec3(diff, 0.0);
        ::engine.updateView();
    }

    if(glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS && mouseCurPosition != ::engine.local.mousePressPosition && ::engine.local.mousePrevPosition != ::engine.local.mousePressPosition)
    {
        ::engine.local.rotation += orientedAngle(   glm::normalize(mouseCurPosition - ::engine.local.mousePressPosition),
                                                    glm::normalize(::engine.local.mousePrevPosition - ::engine.local.mousePressPosition));

        while(::engine.local.rotation > 360.0)
            ::engine.local.rotation -= 360.0;

        while(::engine.local.rotation < 0)
            ::engine.local.rotation += 360.0;

        ::engine.updateViewport();
    }

    ::engine.local.mousePrevPosition = mouseCurPosition;
}

void Engine::glfwWheelCallback(GLFWwindow */*window*/, double/* x*/, double y)
{
    if(::engine.local.viewType != VIEW_MAP)
        return;

    //::engine.log.debug("Wheel cb");
    ::engine.local.zoom = min(10.0, max(0.0001, ::engine.local.zoom * powf(1.25, y)));
    ::engine.updateViewport();
}

void Engine::glfwWindowCloseCallback(GLFWwindow */*window*/)
{
    ::engine.terminate();
}
