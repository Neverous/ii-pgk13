#ifndef __DRAWER_INL__
#define __DRAWER_INL__

#include "drawer.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "libs/logger/logger.h"
#include "libs/config/config.h"
#include "libs/thread/thread.h"

#define MIN_DRAWING_FPS 30

using namespace std;

namespace arkanoid
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
        drawBackground();
        drawObjects();
        glfwSwapBuffers(engine->gl.window);

        double currentFrame = glfwGetTime();
        double diff = currentFrame - lastFrame;
        if(options.framelimit && diff < 1.L / options.framelimit)
            this_thread::sleep_for(chrono::milliseconds(static_cast<unsigned int>(1000.L / options.framelimit - diff * 1000.0L)));

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
        engine->gl.positionVector = 0;
    }

    if(engine->gl.buffer)
    {
        glDeleteBuffers(1, &engine->gl.buffer);
        engine->gl.buffer = 0;
    }
}

inline
void Drawer::configure(Config &cfg)
{
    options.framelimit = cfg.get<unsigned int>("drawer", "framelimit", 0);
}

inline
void Drawer::drawBackground(void)
{
    glUniform3f(engine->gl.positionVector, 0., 0., 0.);
    glDrawArrays(GL_LINE_STRIP, engine->local.background.index, engine->local.background.points);
}

inline
void Drawer::drawObjects(void)
{
    // draw bricks
    for(unsigned int b = 0; b < engine->local.bricks; ++ b)
    {
        Object &brick = engine->local.brick[b];
        if(brick.alive)
        {
            glUniform3fv(engine->gl.positionVector, 1, brick.position);
            glDrawArrays(GL_QUADS, brick.index, brick.points / 2);
            glDrawArrays(GL_LINE_LOOP, brick.index + brick.points / 2, brick.points / 2);
        }
    }

    // draw paddle
    Object &paddle = engine->local.paddle;
    glUniform3fv(engine->gl.positionVector, 1, paddle.position);
    glDrawArrays(GL_QUADS, paddle.index, paddle.points / 2);
    glDrawArrays(GL_LINE_LOOP, paddle.index + paddle.points / 2, paddle.points / 2);

    // draw ball
    Object &ball = engine->local.ball;
    glUniform3fv(engine->gl.positionVector, 1, ball.position);
    glDrawArrays(GL_TRIANGLE_FAN, ball.index, ball.points);
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
    engine->gl.positionVector = glGetUniformLocation(engine->gl.shaders, "moveVector");
}

} // namespace arkanoid

#endif // __DRAWER_H__
