#include "defines.h"
#include "engine.h"

#include <cstring>
#include <cstdio>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/vector_angle.hpp>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "drawer/drawer.h"
#include "movement/movement.h"

using namespace viewer;
using namespace viewer::engine;

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

    // LOCAL
    //// 3D
    local.d3d.eye       = glm::dvec3(2.0, 0.0, 0.0);
    local.d3d.direction = glm::dvec3(-1.0, 0.0, 0.0);
    local.d3d.right     = glm::dvec3(0.0, -1.0, 0.0);
    local.d3d.up        = glm::dvec3(0.0, 0.0, 1.0);

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
    loadModel(path);
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
    Assimp::Importer    import;
    const aiScene       *scene  = import.ReadFile(path, aiProcessPreset_TargetRealtime_Fast);
    aiMesh              *mesh   = scene->mMeshes[0];

    uint32_t    faces   = mesh->mNumFaces;
    uint32_t    indices = faces * 3;

    local.indice.resize(indices);
    for(uint32_t f = 0; f < faces; ++ f)
    {
        const aiFace &face = mesh->mFaces[f];
        assert(face.mNumIndices == 3);
        local.indice[f * 3 + 0] = face.mIndices[0];
        local.indice[f * 3 + 1] = face.mIndices[1];
        local.indice[f * 3 + 2] = face.mIndices[2];
    }

    uint32_t verts = mesh->mNumVertices;
    local.vert.resize(verts * 3);
    local.normal.resize(verts * 3);
    local.uv.resize(verts * 2);
    for(uint32_t v = 0; v < verts; ++ v)
    {
        if(mesh->HasPositions())
        {
            local.vert[v * 3 + 0] = mesh->mVertices[v].x;
            local.vert[v * 3 + 1] = mesh->mVertices[v].y;
            local.vert[v * 3 + 2] = mesh->mVertices[v].z;
        }

        if(mesh->HasNormals())
        {
            local.normal[v * 3 + 0] = mesh->mNormals[v].x;
            local.normal[v * 3 + 1] = mesh->mNormals[v].x;
            local.normal[v * 3 + 2] = mesh->mNormals[v].x;
        }

        if(mesh->HasTextureCoords(0))
        {
            local.uv[v * 2 + 0] = mesh->mTextureCoords[0][v].x;
            local.uv[v * 2 + 1] = mesh->mTextureCoords[0][v].y;
        }
    }

    log.debug("Loaded %d indices, %d verts, %d normals, %d uvs", local.indice.size(), local.vert.size(), local.normal.size(), local.uv.size());
}

void Engine::updateViewport(void)
{
    local.d3d.projection = glm::infinitePerspective(
        options.fov,
        1.0 * options.width / options.height,
        0.000001);
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

