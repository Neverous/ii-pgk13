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

    glfwWindowHint(GLFW_RESIZABLE,              false);
    glfwWindowHint(GLFW_SAMPLES,                4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR,  2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR,  0);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glEnable(GL_MULTISAMPLE);

    if(!(gl.window = glfwCreateWindow(800, 600, "terrain", nullptr, nullptr)))
    {
        glfwTerminate();
        throw runtime_error("GLFWCreateWindow error!");
    }

    glfwSetWindowCloseCallback(gl.window, Engine::glfwWindowCloseCallback);

    glfwSetKeyCallback(gl.window,           Engine::glfwKeyCallback);
    glfwSetMouseButtonCallback(gl.window,   Engine::glfwMouseButtonCallback);
    glfwSetCursorPosCallback(gl.window,     Engine::glfwMouseMoveCallback);
    glfwSetScrollCallback(gl.window,        Engine::glfwWheelCallback);

    local.zoom = 1.0f;
    local.eye = glm::vec3(0.0f, 0.0f, 1.0f);

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
            local.world[mercator::lonToMet(lon + 1.0 * w / 1200)][mercator::latToMet(lat + 1.0 * h / 1200)] = map.get(w, h);
}

inline
void Engine::updateViewport(void)
{
    float wres = 400.0f / local.zoom;
    float hres = 300.0f / local.zoom;
    local.projection = glm::rotate(glm::ortho(-wres, wres, -hres, hres, 0.0f, 10.0f), local.rotation, glm::vec3(0, 0, 1));
    //log.debug("VIEWPORT: %.3f %.3f %.3f %.3f ZOOM: %.3f ROTATION: %.3f", -wres, wres, -hres, hres, local.zoom, local.rotation);
}

inline
void Engine::updateView(void)
{
    local.view = glm::lookAt(local.eye, glm::vec3(glm::vec2(local.eye), 0.0f), glm::vec3(0, 1, 0));
    //log.debug("VIEW: %.3f %.3f", local.eye.x, local.eye.y);
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
                ::engine.local.mousePrevPosition = ::engine.local.mousePressPosition = glm::vec2(x, y);
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
    glm::vec2 mouseCurPosition(x, y);
    if(glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
    {
        glm::vec2 diff = glm::rotate(mouseCurPosition - ::engine.local.mousePrevPosition, ::engine.local.rotation) / ::engine.local.zoom;
        diff.x *= -1;

        ::engine.local.eye += glm::vec3(diff, 0.0f);
        ::engine.updateView();
    }

    if(glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS && mouseCurPosition != ::engine.local.mousePressPosition && ::engine.local.mousePrevPosition != ::engine.local.mousePressPosition)
    {
        ::engine.local.rotation += orientedAngle(   glm::normalize(mouseCurPosition - ::engine.local.mousePressPosition),
                                                    glm::normalize(::engine.local.mousePrevPosition - ::engine.local.mousePressPosition));

        while(::engine.local.rotation > 360.0f)
            ::engine.local.rotation -= 360.0f;

        while(::engine.local.rotation < 0)
            ::engine.local.rotation += 360.0f;

        ::engine.updateViewport();
    }

    ::engine.local.mousePrevPosition = mouseCurPosition;
}

void Engine::glfwWheelCallback(GLFWwindow */*window*/, double/* x*/, double y)
{
    if(::engine.local.viewType != VIEW_MAP)
        return;

    //::engine.log.debug("Wheel cb");
    ::engine.local.zoom = min(10.0f, max(0.0001f, ::engine.local.zoom * powf(1.25, y)));
    ::engine.updateViewport();
}

void Engine::glfwWindowCloseCallback(GLFWwindow */*window*/)
{
    ::engine.terminate();
}
