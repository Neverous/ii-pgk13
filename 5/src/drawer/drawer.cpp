#include "defines.h"
#include "drawer.h"

#include <chrono>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "engine/engine.h"
#include "engine/objects.h"
#include "libs/logger/logger.h"

using namespace std;
using namespace terrain;
using namespace terrain::drawer;

extern engine::Engine   engine;

void Drawer::start(void)
{
    pthread_setname_np(handle.native_handle(), "Drawer");
    log.debug("Starting drawer");

    glfwMakeContextCurrent(::engine.gl.window);
    if(glewInit() != GLEW_OK)
    {
        glfwDestroyWindow(::engine.gl.window);
        glfwTerminate();

        throw runtime_error("GLEWInit error");
    }

    glfwSwapInterval(0);
    glClearColor(0x2E / 255.0f, 0x34 / 255.0f, 0x36 / 255.0f, 1.0f);

    glGenBuffers(11, ::engine.gl.buffer);
    loadShaders();

    log.debug("Creating lod tables");
    for(int l = 0; l < TILE_DENSITY_BITS; ++ l)
    {
        ::engine.local.lodSize[l] = ((1 << (TILE_DENSITY_BITS - l)) - 1) * ((1 << (TILE_DENSITY_BITS - l)) - 1) * 6;
        ::engine.local.lodIndices[l] = new uint32_t[::engine.local.lodSize[l]];
        int i = 0;
        for(int h = 0; h < (1 << (TILE_DENSITY_BITS - l)) - 1; ++ h)
            for(int w = 0; w < (1 << (TILE_DENSITY_BITS - l)) - 1; ++ w)
            {
                uint32_t current    = w + h * (1 << (TILE_DENSITY_BITS - l)),
                         next       = current + (1 << (TILE_DENSITY_BITS - l));

                ::engine.local.lodIndices[l][i ++] = current;
                ::engine.local.lodIndices[l][i ++] = next;
                ::engine.local.lodIndices[l][i ++] = current + 1;

                ::engine.local.lodIndices[l][i ++] = next;
                ::engine.local.lodIndices[l][i ++] = next + 1;
                ::engine.local.lodIndices[l][i ++] = current + 1;
            }
    }

    log.debug("Creating loading mesh");
    objects::Position mesh[32768];
    int p = 0;
    for(int l = 0; l < 8192; ++ l)
    {
        mesh[p ++] = objects::Position(-32000000.0f, 32000000.0f - l * 64000000.0f / 8191);
        mesh[p ++] = objects::Position(32000000.0f, 32000000.0f - l * 64000000.0f / 8191);
    }

    for(int l = 0; l < 8192; ++ l)
    {
        mesh[p ++] = objects::Position(-32000000.0f + l * 64000000.0f / 8191, 32000000.0f);
        mesh[p ++] = objects::Position(-32000000.0f + l * 64000000.0f / 8191, -32000000.0f);
    }

    glBindBuffer(GL_ARRAY_BUFFER, ::engine.gl.buffer[LOADING_BUFFER]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(objects::Position) * 32768, mesh, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Drawer::run(void)
{
    static int lod = 0;
    log.debug("Running drawer");
    double lastFrame = 0;
    static double _counter = 0;
    static int _c = 0;
    while(state == Thread::STARTED)
    {
        lastFrame = glfwGetTime();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        drawTerrain(::engine.local.lod ? ::engine.local.lod : lod);
        glfwSwapBuffers(::engine.gl.window);

        double currentFrame = glfwGetTime();
        double diff = currentFrame - lastFrame;

        _counter += diff;
        ++ _c;
        if(_c == 10 * DRAWER_FPS)
        {
            log.debug("Current FPS: %.3lf", _c / _counter);
            _c = 0;
            _counter = 0;
        }

        if(diff < 1.0L / DRAWER_FPS)
        {
            lod = max(0, lod - 1);
            this_thread::sleep_for(chrono::milliseconds(static_cast<unsigned int>(1000.0L / (DRAWER_FPS - 1) - diff * 1000.0L)));
        }

        else if(diff > 1.0L / (DRAWER_FPS - 1))
        {
            log.warning("Rendering frame took: %.4lfs", diff);
            lod = min(lod + 1, 10);
        }
    }
}

void Drawer::stop(void)
{
}

void Drawer::terminate(void)
{
}

inline
void Drawer::drawTerrain(int lod)
{
    drawFoundation();
    for(int t = 0; t < 9; ++ t)
    {
        objects::Tile &tile = ::engine.local.tile[t];
        if(tile.synchronized == objects::Tile::Status::DESYNCHRONIZED)
            continue;

        drawTile(tile, lod);
    }
}

inline
void Drawer::drawFoundation(void)
{
    glBindBuffer(GL_ARRAY_BUFFER, ::engine.gl.buffer[LOADING_BUFFER]);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glUseProgram(::engine.gl.shaders[MAP_PROGRAM + LOADING_PROGRAM]);
    glm::mat4 uniform = ::engine.local.projection * ::engine.local.view;
    glUniformMatrix4fv(::engine.gl.MVP[MAP_PROGRAM + LOADING_PROGRAM], 1, GL_FALSE, &uniform[0][0]);

    glDrawArrays(GL_LINES, 0, 32768);

    glUseProgram(0);
    glDisableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

inline
void Drawer::drawTile(objects::Tile &tile, int lod)
{
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, tile.buffer);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glUseProgram(::engine.gl.shaders[MAP_PROGRAM + TILE_PROGRAM]);

    glm::mat4 uniform = ::engine.local.projection * ::engine.local.view;
    glUniformMatrix4fv(::engine.gl.MVP[MAP_PROGRAM + LOADING_PROGRAM], 1, GL_FALSE, &uniform[0][0]);
    glDrawElements(GL_TRIANGLES, ::engine.local.lodSize[lod], GL_UNSIGNED_INT, ::engine.local.lodIndices[lod]);

    glUseProgram(0);
    glDisableVertexAttribArray(0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

inline
void Drawer::loadShaders(void)
{
    ::engine.gl.shaders[MAP_PROGRAM + LOADING_PROGRAM] = loadShader("src/shaders/map/loadVertex.glsl", "src/shaders/map/loadFragment.glsl");
    ::engine.gl.MVP[MAP_PROGRAM + LOADING_PROGRAM] = glGetUniformLocation(::engine.gl.shaders[MAP_PROGRAM + LOADING_PROGRAM], "MVP");

    ::engine.gl.shaders[MAP_PROGRAM + TILE_PROGRAM] = loadShader("src/shaders/map/mapVertex.glsl", "src/shaders/map/mapFragment.glsl");
    ::engine.gl.MVP[MAP_PROGRAM + TILE_PROGRAM] = glGetUniformLocation(::engine.gl.shaders[MAP_PROGRAM + TILE_PROGRAM], "MVP");

    /*
    ::engine.gl.shaders[FLY_PROGRAM + LOADING_PROGRAM] = loadShader("src/shaders/fly/loadVertex.glsl", "src/shaders/fly/loadFragment.glsl");
    ::engine.gl.MVP[FLY_PROGRAM + LOADING_PROGRAM] = glGetUniformLocation(::engine.gl.shaders[FLY_PROGRAM + LOADING_PROGRAM], "MVP");

    ::engine.gl.shaders[FLY_PROGRAM + TILE_PROGRAM] = loadShader("src/shaders/fly/mapVertex.glsl", "src/shaders/fly/mapFragment.glsl");
    ::engine.gl.MVP[FLY_PROGRAM + FLY_PROGRAM] = glGetUniformLocation(::engine.gl.shaders[FLY_PROGRAM + TILE_PROGRAM], "MVP");
    */
}

Drawer::Drawer(Log &_log)
:log(_log, "DRAWER")
{
}

Drawer::~Drawer(void)
{
}

inline
GLuint Drawer::loadShader(const char *vertex, const char *fragment)
{
    GLuint  vShader         = glCreateShader(GL_VERTEX_SHADER);
    GLuint  fShader         = glCreateShader(GL_FRAGMENT_SHADER);

    // VERTEX SHADER
    {
        char    *vShaderCode    = nullptr;
        FILE*   vShaderFile     = fopen(vertex, "rb");
        if(!vShaderFile)
        {
            glfwDestroyWindow(::engine.gl.window);
            glfwTerminate();

            throw runtime_error("Cannot load vertex shader file");
        }

        fseek(vShaderFile, 0, SEEK_END);
        unsigned int bytes = ftell(vShaderFile);

        fseek(vShaderFile, 0, SEEK_SET);

        vShaderCode = new char[bytes + 1];
        if(!vShaderCode)
        {
            glfwDestroyWindow(::engine.gl.window);
            glfwTerminate();

            throw runtime_error("Insufficient memory to load vertex shader");
        }

        if(fread(vShaderCode, sizeof(char), bytes, vShaderFile) != bytes)
        {
            glfwDestroyWindow(::engine.gl.window);
            glfwTerminate();

            throw runtime_error("Couldn't read shader file");
        }

        vShaderCode[bytes] = 0;
        fclose(vShaderFile);
        vShaderFile = nullptr;

        log.debug("Compiling shader: %s", vertex);
        glShaderSource(vShader, 1, (const char **) &vShaderCode, nullptr);
        glCompileShader(vShader);

        GLint status = GL_FALSE;
        int logLength;

        glGetShaderiv(vShader, GL_COMPILE_STATUS, &status);
        glGetShaderiv(vShader, GL_INFO_LOG_LENGTH, &logLength);

        if(status == GL_FALSE)
        {
            char buffer[logLength];
            glGetShaderInfoLog(vShader, logLength, nullptr, buffer);

            glfwDestroyWindow(::engine.gl.window);
            glfwTerminate();

            throw runtime_error(buffer);
        }

        delete[] vShaderCode;
    }

    // FRAGMENT SHADER
    {
        char    *fShaderCode    = nullptr;
        FILE*   fShaderFile     = fopen(fragment, "rb");
        if(!fShaderFile)
        {
            glfwDestroyWindow(::engine.gl.window);
            glfwTerminate();

            throw runtime_error("Cannot load fragment shader file");
        }

        fseek(fShaderFile, 0, SEEK_END);
        unsigned int bytes = ftell(fShaderFile);

        fseek(fShaderFile, 0, SEEK_SET);

        fShaderCode = new char[bytes + 1];
        if(!fShaderCode)
        {
            glfwDestroyWindow(::engine.gl.window);
            glfwTerminate();

            throw runtime_error("Insufficient memory to load fragment shader");
        }

        if(fread(fShaderCode, sizeof(char), bytes, fShaderFile) != bytes)
        {
            glfwDestroyWindow(::engine.gl.window);
            glfwTerminate();

            throw runtime_error("Couldn't read shader file");
        }

        fShaderCode[bytes] = 0;
        fclose(fShaderFile);
        fShaderFile = nullptr;

        log.debug("Compiling shader: %s", fragment);
        glShaderSource(fShader, 1, (const char **) &fShaderCode, nullptr);
        glCompileShader(fShader);

        GLint status = GL_FALSE;
        int logLength;

        glGetShaderiv(fShader, GL_COMPILE_STATUS, &status);
        glGetShaderiv(fShader, GL_INFO_LOG_LENGTH, &logLength);

        if(status == GL_FALSE)
        {
            char buffer[logLength];
            glGetShaderInfoLog(fShader, logLength, nullptr, buffer);

            glfwDestroyWindow(::engine.gl.window);
            glfwTerminate();

            throw runtime_error(buffer);
        }

        delete[] fShaderCode;

    }

    log.debug("Linking program");
    GLuint shader = glCreateProgram();
    glAttachShader(shader, vShader);
    glAttachShader(shader, fShader);
    glLinkProgram(shader);

    GLint status = GL_FALSE;
    int logLength;

    glGetProgramiv(shader, GL_LINK_STATUS, &status);
    glGetProgramiv(shader, GL_INFO_LOG_LENGTH, &logLength);

    if(status == GL_FALSE)
    {
        char buffer[logLength];
        glGetProgramInfoLog(vShader, logLength, nullptr, buffer);

        glfwDestroyWindow(::engine.gl.window);
        glfwTerminate();

        throw runtime_error(buffer);
    }

    glDeleteShader(vShader);
    glDeleteShader(fShader);
    return shader;
}
