#include "defines.h"
#include "engine.h"

#include <cstring>
#include <cstdio>
#include <libgen.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/vector_angle.hpp>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "objects.h"
#include "drawer/drawer.h"
#include "movement/movement.h"

using namespace viewer;
using namespace viewer::engine;
using namespace viewer::objects;

extern engine::Engine engine;

// GLFW CALLBACKS
DECL_GLFW_CALLBACK(engine, glfwErrorCallback);
DECL_GLFW_CALLBACK(engine, glfwKeyCallback);
DECL_GLFW_CALLBACK(engine, glfwMouseMoveCallback);
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
    threads.movement    = new movement::Movement(_debug, *this);

    // OPTIONS
    options.width       = 800;
    options.height      = 600;
    options.fov         = 45.0;
    options.speed       = 0.05;
    options.lights      = true;
    options.textures    = true;

    // LOCAL
    //// 3D
    local.d3d.eye       = glm::dvec3(1.0, 0.0, 0.0);
    local.d3d.direction = glm::dvec3(-1.0, 0.0, 0.0);
    local.d3d.right     = glm::dvec3(0.0, -1.0, 0.0);
    local.d3d.up        = glm::dvec3(0.0, 0.0, 1.0);

    // BOUNDS
    local.bound.min.x   = 1000000000.0;
    local.bound.min.y   = 1000000000.0;
    local.bound.min.z   = 1000000000.0;
    local.bound.max.x   = -1000000000.0;
    local.bound.max.y   = -1000000000.0;
    local.bound.max.z   = -1000000000.0;

    // Connect glfw error handler
    glfwSetErrorCallback(GLFW_CALLBACK(glfwErrorCallback));

    if(!glfwInit())
        throw runtime_error("GLFWInit error!");

    glfwWindowHint(GLFW_SAMPLES,                4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR,  2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR,  0);

    if(!(gl.window = glfwCreateWindow(options.width, options.height, "viewer", nullptr, nullptr)))
    {
        glfwTerminate();
        throw runtime_error("GLFWCreateWindow error!");
    }

    // GLFW WINDOW CALLBACKS
    glfwSetInputMode(gl.window,                 GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetWindowCloseCallback(gl.window,       GLFW_CALLBACK(glfwWindowCloseCallback));
    glfwSetFramebufferSizeCallback(gl.window,   GLFW_CALLBACK(glfwWindowResizeCallback));
    glfwSetKeyCallback(gl.window,               GLFW_CALLBACK(glfwKeyCallback));
    glfwSetCursorPosCallback(gl.window,         GLFW_CALLBACK(glfwMouseMoveCallback));

    updateViewport();
    updateView();
}

Engine::~Engine(void)
{
}

void Engine::run(const char *path)
{
    log.debug("Starting up...");
    options.basedir = path;
    options.basedir = string(dirname(&options.basedir[0]));
    loadModel(path);
    local.d3d.eye = glm::dvec3(
        (local.bound.max.x + local.bound.min.x) / 2.0,
        (local.bound.max.y + local.bound.min.y) / 2.0,
        (local.bound.max.z + local.bound.min.z) / 2.0
    );

    options.speed = min(local.bound.max.x - local.bound.min.x, min(local.bound.max.y - local.bound.min.y, local.bound.max.z - local.bound.min.z)) / 10.0;

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
void Engine::loadModel(const char *path)
{
    log.debug("Loading %s", path);
    unordered_map<string, GLuint> textureID;
    Assimp::Importer    import;
    const aiScene       *scene  = import.ReadFile(path, aiProcessPreset_TargetRealtime_Fast);
    uint32_t            meshes  = scene->mNumMeshes;

    loadTexture(scene);
    local.mesh.resize(meshes);
    for(uint32_t m = 0; m < meshes; ++ m)
        local.mesh[m].load(scene->mMeshes[m], scene, local.bound.min, local.bound.max);
}

inline
void Engine::loadTexture(const aiScene *scene)
{
    uint32_t materials = scene->mNumMaterials;
    for(uint32_t m = 0; m < materials; ++ m)
    {
        uint32_t index = 0;
        aiString path;
        aiReturn res;
        while((res = scene->mMaterials[m]->GetTexture(aiTextureType_DIFFUSE, index ++, &path)) == AI_SUCCESS)
            gl.texture[path.data] = 0;
    }

    log.debug("Loaded %d texture paths", gl.texture.size());
}

void Engine::updateViewport(void)
{
    local.d3d.projection = glm::infinitePerspective(
        options.fov,
        1.0 * options.width / options.height,
        options.speed);
}

void Engine::updateView(void)
{
    local.d3d.view = glm::lookAt(local.d3d.eye, local.d3d.eye + local.d3d.direction, local.d3d.up);
}

glm::mat4 Engine::getUniform(void)
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

        // FIXME
    }
}

inline
void Engine::glfwMouseMoveCallback(GLFWwindow */*window*/, double x, double y)
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

