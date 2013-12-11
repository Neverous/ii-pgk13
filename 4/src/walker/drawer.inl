#ifndef __DRAWER_INL__
#define __DRAWER_INL__

#include "drawer.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "libs/logger/logger.h"
#include "libs/config/config.h"
#include "libs/thread/thread.h"

#define MIN_DRAWING_FPS 30
#define MAX_DRAWING_FPS 60

using namespace std;

namespace walker
{

inline
void Drawer::start(void)
{
    pthread_setname_np(handle.native_handle(), "Drawer");

    log.debug("Starting drawer");
    glfwMakeContextCurrent(engine->gl.window);
    if(glewInit() != GLEW_OK)
    {
        glfwDestroyWindow(engine->gl.window);
        glfwTerminate();

        throw runtime_error("GLEWInit error");
    }

    log.debug("Generating GL structures");
    // Generate GL buffers
    glGenBuffers(1, &engine->gl.buffer);
    glBindBuffer(GL_ARRAY_BUFFER, engine->gl.buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Point) * engine->local.points, engine->local.point, GL_STATIC_DRAW);

    // Load shaders
    loadShaders();

    glClearColor(_GL_COLOR(0x2E), _GL_COLOR(0x34), _GL_COLOR(0x36), 0.);

    glUseProgram(engine->gl.shaders);

    glBindBuffer(GL_ARRAY_BUFFER, engine->gl.buffer);
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Point), nullptr);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Point), (const GLvoid *) sizeof(Position));
}

inline
void Drawer::run(void)
{
    log.debug("Running drawer");
    double lastFrame = 0;
    while(state == Thread::STARTED)
    {
        lastFrame = glfwGetTime();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        drawObjects();
        glfwSwapBuffers(engine->gl.window);

        double currentFrame = glfwGetTime();
        double diff = currentFrame - lastFrame;
        if(diff < 1.L / MAX_DRAWING_FPS)
            this_thread::sleep_for(chrono::milliseconds(static_cast<unsigned int>(1000.L / MAX_DRAWING_FPS - diff * 1000.0L)));

        else if(diff > 1.L / MIN_DRAWING_FPS)
            log.warning("Rendering frame took: %.4lfs", diff);
    }
}

inline
void Drawer::stop(void)
{
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(0);
    glUseProgram(0);
}

inline
void Drawer::terminate(void)
{
}

inline
Drawer::Drawer(Log &_log, Engine *_engine)
:log(_log, "DRAWER")
,engine(_engine)
{
}

inline
Drawer::~Drawer(void)
{
    if(engine->gl.shaders)
    {
        glDeleteProgram(engine->gl.shaders);
        engine->gl.shaders = 0;
        engine->gl.MVP = 0;
    }

    if(engine->gl.buffer)
    {
        glDeleteBuffers(1, &engine->gl.buffer);
        engine->gl.buffer = 0;
    }
}

inline
void Drawer::drawObjects(void)
{
    glm::mat4   Projection  = glm::perspective(90.0f, 4.0f / 3.0f, 1.0f, 100.0f);
    glm::mat4   View;
    switch(engine->local.view)
    {
        case VIEW_FRONT:
            View = glm::lookAt(
                glm::vec3(5.0f, 1.0f, 5.0f),
                glm::vec3(5.0f, 0.25f, 0.0f),
                glm::vec3(0.0f, 1.0f, 0.0f)
            );

            break;

        case VIEW_RIGHT:
            View = glm::lookAt(
                glm::vec3(7.0f, 1.0f, 0.5f),
                glm::vec3(5.0f, 0.25f, 0.0f),
                glm::vec3(0.0f, 1.0f, 0.0f)
            );

            break;

        case VIEW_FPP:
            {
                Figure &figure = engine->local.figure[63];
                View = glm::lookAt(
                    figure.position,
                    figure.position + glm::vec3(0.0f, 0.0f, 5.0f),
                    glm::vec3(0.0f, 1.0f, 0.0f)
                );
            }

            break;

        case VIEW_TPP:
            {
                Figure &figure = engine->local.figure[63];
                View = glm::lookAt(
                    figure.position + glm::vec3(0.0f, 0.5f, -1.0f),
                    figure.position + glm::vec3(0.0f, 0.0f, 5.0f),
                    glm::vec3(0.0f, 1.0f, 0.0f)
                );
            }

            break;
    }

    glm::mat4   VP  = Projection * View;
    glm::mat4   MVP = VP;

    glUniformMatrix4fv(engine->gl.MVP, 1, GL_FALSE, &MVP[0][0]);
    glDrawArrays(GL_QUADS, engine->local.ground.index, engine->local.ground.points);

    // draw figures
    for(unsigned int f = 0; f < engine->local.figures; ++ f)
    {
        Figure      &figure     = engine->local.figure[f];
        glm::mat4   Model[12]   = {};
        Model[0][3] = glm::vec4(figure.position.x, figure.position.y, figure.position.z, 0.0f);
        unsigned int ref[12]     = {0, 0, 1, 2, 1, 4, 0, 6, 0, 8, 3, 5};
        for(unsigned int m = 0; m < 12; ++ m)
        {
            if(ref[m] != m) Model[m][3] += Model[ref[m]][0] + Model[ref[m]][3];
            Model[m][0] = glm::vec4(figure.bone[m].x, figure.bone[m].y, figure.bone[m].z, 0.0f);
            Model[m][3][3] = 1.0f;

            MVP = VP * Model[m];
            glUniformMatrix4fv(engine->gl.MVP, 1, GL_FALSE, &MVP[0][0]);
            glDrawArrays(GL_LINES, figure.index, figure.points);

            glm::vec4   a(figure.local[0].x, figure.local[0].y, figure.local[0].z, 1.0f),
                        b(figure.local[1].x, figure.local[1].y, figure.local[1].z, 1.0f);

            a = MVP * a;
            b = MVP * b;
        }
    }
}

inline
void Drawer::loadShaders(void)
{
    GLuint  vShader         = glCreateShader(GL_VERTEX_SHADER);
    GLuint  fShader         = glCreateShader(GL_FRAGMENT_SHADER);

    // VERTEX SHADER
    {
        char    *vShaderCode    = nullptr;
        FILE*   vShaderFile     = fopen("shader.vsh", "r");
        if(!vShaderFile)
        {
            glfwDestroyWindow(engine->gl.window);
            glfwTerminate();

            throw runtime_error("Cannot load vertex shader file");
        }

        fseek(vShaderFile, 0, SEEK_END);
        unsigned int bytes = ftell(vShaderFile);

        fseek(vShaderFile, 0, SEEK_SET);

        vShaderCode = new char[bytes];
        if(!vShaderCode)
        {
            glfwDestroyWindow(engine->gl.window);
            glfwTerminate();

            throw runtime_error("Insufficient memory to load vertex shader");
        }

        fread(vShaderCode, sizeof(char), bytes, vShaderFile);
        fclose(vShaderFile);
        vShaderFile = nullptr;

        log.debug("Compiling shader: shader.vsh");
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

            glfwDestroyWindow(engine->gl.window);
            glfwTerminate();

            throw runtime_error(buffer);
        }

        delete[] vShaderCode;
    }

    // FRAGMENT SHADER
    {
        char    *fShaderCode    = nullptr;
        FILE*   fShaderFile     = fopen("shader.fsh", "r");
        if(!fShaderFile)
        {
            glfwDestroyWindow(engine->gl.window);
            glfwTerminate();

            throw runtime_error("Cannot load fragment shader file");
        }

        fseek(fShaderFile, 0, SEEK_END);
        unsigned int bytes = ftell(fShaderFile);

        fseek(fShaderFile, 0, SEEK_SET);

        fShaderCode = new char[bytes];
        if(!fShaderCode)
        {
            glfwDestroyWindow(engine->gl.window);
            glfwTerminate();

            throw runtime_error("Insufficient memory to load fragment shader");
        }

        fread(fShaderCode, sizeof(char), bytes, fShaderFile);
        fclose(fShaderFile);
        fShaderFile = nullptr;

        log.debug("Compiling shader: shader.fsh");
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

            glfwDestroyWindow(engine->gl.window);
            glfwTerminate();

            throw runtime_error(buffer);
        }

        delete[] fShaderCode;

    }

    log.debug("Linking program");
    engine->gl.shaders = glCreateProgram();
    glAttachShader(engine->gl.shaders, vShader);
    glAttachShader(engine->gl.shaders, fShader);
    glLinkProgram(engine->gl.shaders);

    GLint status = GL_FALSE;
    int logLength;

    glGetProgramiv(engine->gl.shaders, GL_LINK_STATUS, &status);
    glGetProgramiv(engine->gl.shaders, GL_INFO_LOG_LENGTH, &logLength);

    if(status == GL_FALSE)
    {
        char buffer[logLength];
        glGetProgramInfoLog(vShader, logLength, nullptr, buffer);

        glfwDestroyWindow(engine->gl.window);
        glfwTerminate();

        throw runtime_error(buffer);
    }

    glDeleteShader(vShader);
    glDeleteShader(fShader);
    engine->gl.MVP = glGetUniformLocation(engine->gl.shaders, "MVP");
}

} // namespace walker

#endif // __DRAWER_H__
