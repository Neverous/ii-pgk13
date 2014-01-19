#include "defines.h"
#include "engine.h"

#include <cstring>
#include <cstdio>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/vector_angle.hpp>

#include "hgt/file.h"
#include "hgt/map.h"
#include "projection/mercator.h"

#include "drawer/drawer.h"
#include "loader/loader.h"
#include "movement/movement.h"

using namespace terrain;
using namespace terrain::engine;
using namespace terrain::objects;
using namespace terrain::projection;

extern engine::Engine engine;

// GLFW CALLBACKS
DECL_GLFW_CALLBACK(engine, glfwErrorCallback);
DECL_GLFW_CALLBACK(engine, glfwKeyCallback);
DECL_GLFW_CALLBACK(engine, glfwMouseButtonCallback);
DECL_GLFW_CALLBACK(engine, glfwMouseMoveCallback);
DECL_GLFW_CALLBACK(engine, glfwWheelCallback);
DECL_GLFW_CALLBACK(engine, glfwWindowCloseCallback);
DECL_GLFW_CALLBACK(engine, glfwWindowResizeCallback);

Engine::Engine(Log &_debug)
:debug(_debug)
,log(_debug, "ENGINE")
,threads()
,options()
,local()
,gl()
{
    srand(time(nullptr));

    // THREADS
    threads.drawer      = new drawer::Drawer(_debug, *this);
    threads.loader      = new loader::Loader(_debug, *this);
    threads.movement    = new movement::Movement(_debug, *this);

    // OPTIONS
    options.width       = 800;
    options.height      = 600;
    options.lod         = 0;
    options.viewType    = engine::VIEW_2D;
    options.fov         = 45.0;

    // LOCAL
    //// 2D
    local.d2d.zoom      = 1.0;
    local.d2d.rotation  = glm::angleAxis(0.0, glm::dvec3(0.0, 0.0, 1.0));
    local.d2d.eye       = glm::dvec3(0.0, 0.0, 10000.0);

    //// 3D
    local.d3d.eye       = glm::dvec3(0.0, 0.0, 10000.0);
    local.d3d.direction = glm::dvec3(-1.0, 0.0, 0.0);
    local.d3d.right     = glm::dvec3(0.0, -1.0, 0.0);
    local.d3d.up        = glm::dvec3(0.0, 0.0, 1.0);

    // BOUND
    local.bound.min.x   = MERCATOR_BOUNDS;
    local.bound.min.y   = MERCATOR_BOUNDS;
    local.bound.max.x   = -MERCATOR_BOUNDS;
    local.bound.max.y   = -MERCATOR_BOUNDS;

    // Connect glfw error handler
    glfwSetErrorCallback(GLFW_CALLBACK(glfwErrorCallback));

    if(!glfwInit())
        throw runtime_error("GLFWInit error!");

    glfwWindowHint(GLFW_VISIBLE,                false);
    if(!(gl.loader = glfwCreateWindow(1, 1, "loader", nullptr, nullptr)))
    {
        glfwTerminate();
        throw runtime_error("GLFWCreateWindow error!");
    }

    glfwWindowHint(GLFW_VISIBLE,                true);
    glfwWindowHint(GLFW_SAMPLES,                4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR,  2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR,  0);

    if(!(gl.window = glfwCreateWindow(options.width, options.height, "terrain", nullptr, gl.loader)))
    {
        glfwTerminate();
        throw runtime_error("GLFWCreateWindow error!");
    }

    // GLFW WINDOW CALLBACKS
    glfwSetWindowCloseCallback(gl.window,       GLFW_CALLBACK(glfwWindowCloseCallback));
    glfwSetFramebufferSizeCallback(gl.window,   GLFW_CALLBACK(glfwWindowResizeCallback));
    glfwSetKeyCallback(gl.window,               GLFW_CALLBACK(glfwKeyCallback));
    glfwSetMouseButtonCallback(gl.window,       GLFW_CALLBACK(glfwMouseButtonCallback));
    glfwSetCursorPosCallback(gl.window,         GLFW_CALLBACK(glfwMouseMoveCallback));
    glfwSetScrollCallback(gl.window,            GLFW_CALLBACK(glfwWheelCallback));

    updateViewport();
    updateView();
}

Engine::~Engine(void)
{
}

void Engine::run(int argc, char **argv)
{
    log.debug("Starting up...");
    log.debug("Loading %d maps", argc - 1);
    for(int a = 1; a < argc; ++ a)
        loadMap(argv[a]);

    local.d2d.eye = glm::dvec3(
        (local.bound.max.x + local.bound.min.x) / 2.0,
        (local.bound.max.y + local.bound.min.y) / 2.0,
        10000.0);

    local.d3d.eye = glm::dvec3(
        (local.bound.max.x + local.bound.min.x) / 2.0,
        (local.bound.max.y + local.bound.min.y) / 2.0,
        10000.0);

    updateViewport();
    updateView();

    ::threads.activate();
    log.debug("Running engine");
    while(!glfwWindowShouldClose(gl.window))
        glfwWaitEvents();
}

void Engine::terminate(void)
{
    glfwSetWindowShouldClose(gl.window, GL_TRUE);
    ::threads.deactivate();
}

inline
void Engine::loadMap(const char *path)
{
    log.debug("Loading %s map", path);
    char parse[1024]    = {},
         *filename      = nullptr;

    int32_t lat = 0,
            lon = 0;

    strncpy(parse, path, 1023);
    parseMapFilename(parse, filename, lat, lon);
    log.debug("Loading %s square (%d %d)", filename, lon, lat);

    hgt::Map &chunk = local.world[lat][lon];
    hgt::File map(path);

    for(int h = 0; h < 1201; ++ h)
        for(int w = 0; w < 1201; ++ w)
            chunk.set(w, h, map.get(w, h) + 1000);

    local.bound.max.x = max(local.bound.max.x, mercator::lonToMet(lon + 1));
    local.bound.max.y = max(local.bound.max.y, mercator::latToMet(lat + 1));
    local.bound.min.x = min(local.bound.min.x, mercator::lonToMet(lon));
    local.bound.min.y = min(local.bound.min.y, mercator::latToMet(lat));
    log.debug("Loaded data from %s", filename);
}

inline
void Engine::parseMapFilename(char *path, char *&filename, int32_t &lat, int32_t &lon)
{
    char *extension = nullptr,
         nPos[2]    = {},
         nDeg[4]    = {},
         wPos[2]    = {},
         wDeg[4]    = {};

    filename = basename(path);
    if(!(extension = strrchr(filename, '.')) || strcmp(extension + 1, "hgt"))
        throw runtime_error("Invalid map file type");

    *extension = 0;
    sscanf(filename, "%1[nNsS]%3[0123456789]%1[wWeE]%3[0123456789]", nPos, nDeg, wPos, wDeg);
    sscanf(nDeg, "%d", &lat);
    sscanf(wDeg, "%d", &lon);
    switch(tolower(nPos[0]))
    {
        case 's':
            lat *= -1;
            break;

        case 'n':
            break;

        default:
            throw runtime_error("Invalid map file name");
            break;
    }

    switch(tolower(wPos[0]))
    {
        case 'w':
            lon *= -1;
            break;

        case 'e':
            break;

        default:
            throw runtime_error("Invalid map file name");
            break;
    }
}

void Engine::updateViewport(void)
{
    glViewport(0, 0, options.width, options.height);
    switch(options.viewType)
    {
        case engine::VIEW_2D:
            updateViewport2D();
            break;

        case engine::VIEW_3D:
            updateViewport3D();
            break;

        default:
            throw runtime_error("Invalid view type");
            break;
    }
}

inline
void Engine::updateViewport2D(void)
{
    double wres = options.width / 2.0 / local.d2d.zoom;
    double hres = options.height / 2.0 / local.d2d.zoom;
    local.d2d.projection = glm::ortho(-wres, wres, -hres, hres, 0.0, 11000.0) * glm::toMat4(local.d2d.rotation);
}

inline
void Engine::updateViewport3D(void)
{
    local.d3d.projection = glm::infinitePerspective(
        options.fov,
        1.0 * options.width / options.height,
        10.0);
}

void Engine::updateView(void)
{
    switch(options.viewType)
    {
        case engine::VIEW_2D:
            updateView2D();
            break;

        case engine::VIEW_3D:
            updateView3D();
            break;

        default:
            throw runtime_error("Invalid view type");
            break;
    }
}

inline
void Engine::updateView2D(void)
{
    local.d2d.view = glm::lookAt(local.d2d.eye, glm::dvec3(glm::dvec2(local.d2d.eye), 0.0), glm::dvec3(0.0, 1.0, 0.0));
}

inline
void Engine::updateView3D(void)
{
    local.d3d.view = glm::lookAt(local.d3d.eye, local.d3d.eye + local.d3d.direction, local.d3d.up);
}

inline
void Engine::changeViewType(void)
{
    switch(options.viewType)
    {
        case engine::VIEW_2D:
            options.viewType = engine::VIEW_3D;
            setupView3D();
            break;

        case engine::VIEW_3D:
            options.viewType = engine::VIEW_2D;
            setupView2D();
            break;

        default:
            throw runtime_error("Invalid view type");
            break;
    }
}

inline
void Engine::setupView2D(void)
{
    glfwSetInputMode(gl.window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    glm::dvec3 eye = glm::normalize(local.d3d.eye) * mercator::EQUATORIAL_RADIUS;
    double lat = asin(eye.z / mercator::EQUATORIAL_RADIUS) * 180.0 / M_PI;
    double lon = atan2(eye.y, eye.x) * 180.0 / M_PI;
    local.d2d.eye.x = mercator::lonToMet(lon);
    local.d2d.eye.y = mercator::latToMet(lat);
    local.d2d.eye.z = 10000.0;
    updateViewport();
    updateView();
}

inline
void Engine::setupView3D(void)
{
    glfwSetInputMode(gl.window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPos(gl.window, options.width / 2.0, options.height / 2.0);

    double altitude = local.d2d.eye.z;
    glm::dvec2 lonlat(mercator::metToLon(local.d2d.eye.x) * M_PI / 180.0, mercator::metToLat(local.d2d.eye.y) * M_PI / 180.0);
    local.d3d.eye.x = (mercator::EQUATORIAL_RADIUS + altitude) * cos(lonlat.y) * cos(lonlat.x);
    local.d3d.eye.y = (mercator::EQUATORIAL_RADIUS + altitude) * cos(lonlat.y) * sin(lonlat.x);
    local.d3d.eye.z = (mercator::EQUATORIAL_RADIUS + altitude) * sin(lonlat.y);

    local.d3d.direction = glm::dvec3(-1.0, 0.0, 0.0);
    local.d3d.right     = glm::dvec3(0.0, -1.0, 0.0);
    local.d3d.up        = glm::dvec3(0.0, 0.0, 1.0);

    updateViewport();
    updateView();
}

glm::dvec4 Engine::getBoundingRect(void)
{
    switch(options.viewType)
    {
        case engine::VIEW_2D:
            return getBoundingRect2D();
            break;

        case engine::VIEW_3D:
            return getBoundingRect3D();
            break;

        default:
            throw runtime_error("Invalid view type");
            break;
    }

    return glm::dvec4(-1.0, -1.0, -1.0, -1.0);
}

inline
glm::dvec4 Engine::getBoundingRect2D(void)
{
    double res = sqrt(1.0 * options.width * options.width + options.height * options.height) / local.d2d.zoom / 2.0;
    return glm::dvec4(local.d2d.eye.x - res, local.d2d.eye.x + res, local.d2d.eye.y - res, local.d2d.eye.y + res);
}

inline
glm::dvec4 Engine::getBoundingRect3D(void)
{
    double zoom = min(1.0, max(0.0001, 25.0 / (glm::length(local.d3d.eye) - mercator::EQUATORIAL_RADIUS)));
    glm::dvec3 eye = glm::normalize(local.d3d.eye) * mercator::EQUATORIAL_RADIUS;
    double lat = asin(eye.z / mercator::EQUATORIAL_RADIUS) * 180.0 / M_PI;
    double lon = atan2(eye.y, eye.x) * 180.0 / M_PI;
    eye.x = mercator::lonToMet(lon);
    eye.y = mercator::latToMet(lat);

    double res = sqrt(1.0 * options.width * options.width + options.height * options.height) / zoom / 2.0;
    return glm::dvec4(eye.x - res, eye.x + res, eye.y - res, eye.y + res);
}

glm::mat4 Engine::getUniform(void)
{
    switch(options.viewType)
    {
        case engine::VIEW_2D:
            return getUniform2D();
            break;

        case engine::VIEW_3D:
            return getUniform3D();
            break;

        default:
            throw runtime_error("Invalid view type");
            break;
    }

    return glm::mat4();
}

inline
glm::mat4 Engine::getUniform2D(void)
{
    return glm::mat4(local.d2d.projection * local.d2d.view);
}

inline
glm::mat4 Engine::getUniform3D(void)
{
    return glm::mat4(local.d3d.projection * local.d3d.view);
}

/* GLFW CALLBACKS */
inline
void Engine::glfwErrorCallback(int code, const char *message)
{
    debug.error("GLFW", "Error %d: %s", code, message);
}

inline
void Engine::glfwKeyCallback(GLFWwindow */*window*/, int key, int/* scancode*/, int action, int/* mods*/)
{
    switch(key)
    {
        case GLFW_KEY_ESCAPE:
            if(action == GLFW_PRESS)
                terminate();

            break;

        case GLFW_KEY_V:
        case GLFW_KEY_TAB:
            if(action == GLFW_PRESS)
                changeViewType();

            break;

        case GLFW_KEY_KP_ADD:
            if(action == GLFW_PRESS)
            {
                options.lod = max(0, options.lod - 1);
                log.debug("LOD+: %d", options.lod);
            }

            break;

        case GLFW_KEY_KP_SUBTRACT:
            if(action == GLFW_PRESS)
            {
                options.lod = min(DETAIL_LEVELS - 1, options.lod + 1);
                log.debug("LOD-: %d", options.lod);
            }

            break;

        case GLFW_KEY_0:
            if(action == GLFW_PRESS)
            {
                options.lod = 0;
                log.debug("LOD=: %d", options.lod);
            }

            break;

        case GLFW_KEY_1:
            if(action == GLFW_PRESS)
            {
                options.lod = 1;
                log.debug("LOD=: %d", options.lod);
            }

            break;

        case GLFW_KEY_2:
            if(action == GLFW_PRESS)
            {
                options.lod = 2;
                log.debug("LOD=: %d", options.lod);
            }

            break;

        case GLFW_KEY_3:
            if(action == GLFW_PRESS)
            {
                options.lod = 3;
                log.debug("LOD=: %d", options.lod);
            }

            break;

        case GLFW_KEY_4:
            if(action == GLFW_PRESS)
            {
                options.lod = 4;
                log.debug("LOD=: %d", options.lod);
            }

            break;

        case GLFW_KEY_5:
            if(action == GLFW_PRESS)
            {
                options.lod = 5;
                log.debug("LOD=: %d", options.lod);
            }

            break;

        case GLFW_KEY_6:
            if(action == GLFW_PRESS)
            {
                options.lod = 6;
                log.debug("LOD=: %d", options.lod);
            }

            break;

        case GLFW_KEY_7:
            if(action == GLFW_PRESS)
            {
                options.lod = 7;
                log.debug("LOD=: %d", options.lod);
            }

            break;

        case GLFW_KEY_8:
            if(action == GLFW_PRESS)
            {
                options.lod = 8;
                log.debug("LOD=: %d", options.lod);
            }

            break;

        case GLFW_KEY_9:
            if(action == GLFW_PRESS)
            {
                options.lod = 9;
                log.debug("LOD=: %d", options.lod);
            }

            break;
    }
}

inline
void Engine::glfwMouseButtonCallback(GLFWwindow *window, int button, int action, int/* mods*/)
{
    if(options.viewType != engine::VIEW_2D) // Mouse buttons inactive in flight mode
        return;

    if(action == GLFW_PRESS) switch(button)
    {
        case GLFW_MOUSE_BUTTON_LEFT:
        case GLFW_MOUSE_BUTTON_RIGHT:
            {
                double x, y;
                glfwGetCursorPos(window, &x, &y);
                local.mouse.prev = local.mouse.press = glm::dvec2(x, y);
            }
            break;

        default:
            return;
    }

    if(action == GLFW_RELEASE) switch(button)
    {
        case GLFW_MOUSE_BUTTON_MIDDLE:
            local.d2d.rotation = glm::angleAxis(0.0, glm::dvec3(0.0, 0.0, 0.0));
            updateViewport();
            break;

        default:
            return;
    }
}

inline
void Engine::glfwMouseMoveCallback(GLFWwindow */*window*/, double x, double y)
{
    if(glm::dvec2(x, y) == local.mouse.prev)
        return;

    switch(options.viewType)
    {
        case engine::VIEW_2D:
            mouseMove2D(x, y);
            break;

        case engine::VIEW_3D:
            mouseMove3D(x, y);
            break;

        default:
            throw runtime_error("Invalid view type");
            break;
    }

    local.mouse.prev = glm::dvec2(x, y);
}

inline
void Engine::mouseMove2D(double x, double y)
{
    glm::dvec2 cur(x, y);
    if(glfwGetMouseButton(gl.window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) // Movement
    {
        glm::dvec3 diff = glm::rotate(local.d2d.rotation,
            glm::dvec3(cur - local.mouse.prev, 0.0)) / local.d2d.zoom;

        diff.x *= -1;
        local.d2d.eye += diff;
        updateView();
    }

    if( glfwGetMouseButton(gl.window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS
    &&  local.mouse.press != cur && local.mouse.press != local.mouse.prev) // Rotation
    {
        log.debug("Rotation");
        local.d2d.rotation = glm::angleAxis(
            glm::orientedAngle(glm::normalize(cur - local.mouse.press), glm::normalize(local.mouse.prev - local.mouse.press)),
            glm::dvec3(0.0, 0.0, 1.0)) * local.d2d.rotation;

        updateViewport();
    }
}

inline
void Engine::mouseMove3D(double x, double y)
{
    glfwSetCursorPos(gl.window, options.width / 2.0, options.height / 2.0);
    double horiz    = (options.width / 2.0 - x) * 0.01;
    double vert     = (y - options.height / 2.0) * 0.01;

    local.d3d.right     = glm::rotate(local.d3d.right, horiz, local.d3d.up);
    local.d3d.direction = glm::rotate(local.d3d.direction, horiz, local.d3d.up);

    local.d3d.direction = glm::rotate(local.d3d.direction, vert, local.d3d.right);
    local.d3d.up        = glm::rotate(local.d3d.up, vert, local.d3d.right);
    updateView();
}

void Engine::glfwWheelCallback(GLFWwindow */*window*/, double/* x*/, double y)
{
    if(options.viewType != engine::VIEW_2D) // Mouse buttons inactive in flight mode
        return;

    local.d2d.zoom = min(1.0, max(0.0001, local.d2d.zoom * pow(1.25, y)));
    updateViewport();
}

void Engine::glfwWindowCloseCallback(GLFWwindow */*window*/)
{
    terminate();
}

void Engine::glfwWindowResizeCallback(GLFWwindow */*window*/, int _width, int _height)
{
    options.width   = _width;
    options.height  = _height;
    updateViewport();
}

