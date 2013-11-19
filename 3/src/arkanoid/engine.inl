#ifndef __ENGINE_INL__
#define __ENGINE_INL__

#include "engine.h"

#include <cmath>
#include <stdexcept>
#include <cassert>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "libs/logger/logger.h"
#include "libs/config/config.h"
#include "libs/thread/thread.h"
#include "libs/file/chunkFileReader.h"

#include "objects.h"
#include "physics.h"
#include "drawer.h"

using namespace std;

extern arkanoid::Engine engine;

namespace arkanoid
{

inline
Engine::Engine(Log &_debug)
:debug(_debug)
,log(_debug, "ENGINE")
,physics(new Physics(_debug, this))
,drawer(new Drawer(_debug, this))
,options()
,gl()
{
}

inline
Engine::~Engine(void)
{
    delete physics;
    delete drawer;
}

inline
void Engine::configure(Config &cfg)
{
    // Connect glfw error handler
    glfwSetErrorCallback(Engine::glfwErrorCallback);

    options.resW    = cfg.get<unsigned int>("engine", "width", 800);
    options.resH    = cfg.get<unsigned int>("engine", "height", 600);
    options.level   = cfg.get("arkanoid", "level");
    local.lives     = cfg.get<int>("arkanoid", "lives", 3);

    if(!glfwInit())
        throw runtime_error("GLFWInit error");

    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    if(!(gl.window = glfwCreateWindow(options.resW, options.resH, "arkanoid", nullptr, nullptr)))
    {
        glfwDestroyWindow(gl.window);
        gl.window = nullptr;

        glfwTerminate();
        throw runtime_error("GLFWCreateWindow error");
    }

    glfwSetWindowCloseCallback(gl.window, Engine::glfwWindowCloseCallback);
    glfwSetKeyCallback(gl.window, Engine::glfwKeyCallback);

    log.debug("Loading level");
    loadLevel(options.level);
    log.debug("Level loaded");

    log.debug("Configuring drawer");
    drawer->configure(cfg);
    log.notice("Drawer configured");

    log.debug("Configuring physics");
    physics->configure(cfg);
    log.debug("Physics configured");
}

inline
void Engine::run(void)
{
    log.info("Engine running");
    threads.activate();
    while(!glfwWindowShouldClose(gl.window))
        glfwWaitEvents();

    log.info("Engine finished");
}

inline
void Engine::terminate(void)
{
    glfwSetWindowShouldClose(gl.window, GL_TRUE);
    threads.deactivate();
}

inline
void Engine::movePaddle(int direction)
{
    local.paddle.acceleration.x = direction * physics->options.paddleAcceleration;
}

inline
void Engine::releaseBall(void)
{
    if(local.ball.alive)
        return;

    local.ball.alive = true;
    if(local.paddle.velocity.x)
        local.ball.acceleration.x = .3 * physics->options.maxPaddleVelocity / local.paddle.velocity.x;

    else
        local.ball.acceleration.x = 0.;

    local.ball.acceleration.y = 1.;
    local.ball.acceleration.normalize(physics->options.ballAcceleration);
}

inline
void Engine::loadLevel(const string filename)
{
    Color COLOR[9][2] = {
        { {_GL_COLOR(0xC4), _GL_COLOR(0xA0), _GL_COLOR(0x00)}, {_GL_COLOR(0xED), _GL_COLOR(0xD4), _GL_COLOR(0x00)}, }, // YELLOW
        { {_GL_COLOR(0xCE), _GL_COLOR(0x5C), _GL_COLOR(0x00)}, {_GL_COLOR(0xF5), _GL_COLOR(0x79), _GL_COLOR(0x00)}, }, // ORANGE
        { {_GL_COLOR(0x4E), _GL_COLOR(0x9A), _GL_COLOR(0x06)}, {_GL_COLOR(0x73), _GL_COLOR(0xD2), _GL_COLOR(0x16)}, }, // GREEN
        { {_GL_COLOR(0x20), _GL_COLOR(0x4A), _GL_COLOR(0x87)}, {_GL_COLOR(0x34), _GL_COLOR(0x65), _GL_COLOR(0xA4)}, }, // BLUE
        { {_GL_COLOR(0x5C), _GL_COLOR(0x35), _GL_COLOR(0x66)}, {_GL_COLOR(0x75), _GL_COLOR(0x50), _GL_COLOR(0x7B)}, }, // PURPLE
        { {_GL_COLOR(0xA4), _GL_COLOR(0x00), _GL_COLOR(0x00)}, {_GL_COLOR(0xCC), _GL_COLOR(0x00), _GL_COLOR(0x00)}, }, // RED

        { {_GL_COLOR(0x8F), _GL_COLOR(0x59), _GL_COLOR(0x02)}, {_GL_COLOR(0x8F), _GL_COLOR(0x59), _GL_COLOR(0x02)}, }, // PADDLE
        { {_GL_COLOR(0xEE), _GL_COLOR(0xEE), _GL_COLOR(0xEC)}, {_GL_COLOR(0xEE), _GL_COLOR(0xEE), _GL_COLOR(0xEC)}, }, // BALL
        { {_GL_COLOR(0x55), _GL_COLOR(0x57), _GL_COLOR(0x53)}, {_GL_COLOR(0x55), _GL_COLOR(0x75), _GL_COLOR(0x53)}, }, // GRID
    };

    /* CREATE HEXGRID */
    {
        Object &background = local.background;
        background.points = 0;
        // calculate number of points
        CREATE_HEXGRID({
            background.points += 2;
        });

        log.debug("Background grid points = %d", background.points);
        background.local = reservePoints(background.points, background.index);

        Point *current = background.local;
        for(unsigned int c = 0; c < background.points; ++ c, ++ current)
            memcpy(&current->R, &COLOR[8][0], sizeof(Color));

        int p = 0;
        CREATE_HEXGRID({
            background.local[p].x = px;
            background.local[p].y = py;
            ++ p;
            background.local[p].x = x;
            background.local[p].y = y;
            ++ p;
        });
    }

    ChunkFileReader reader(filename.c_str());
    Object *brick = local.brick;
    unsigned int color = 0;
    for(int h = 0; h < 6; ++ h)
    {
        for(int w = 0; w < 13; ++ w)
        {
            switch(*reader)
            {
                case 'Y':
                    color = 0;
                    break;

                case 'O':
                    color = 1;
                    break;

                case 'G':
                    color = 2;
                    break;

                case 'B':
                    color = 3;
                    break;

                case 'P':
                    color = 4;
                    break;

                case 'R':
                    color = 5;
                    break;

                case '.':
                    ++ reader;
                    continue;

                default:
                    log.debug("Read '%c'\n", *reader);
                    throw runtime_error("Invalid level file");
                    break;
            }

            ++ reader;

            brick->alive = true;
            brick->points = 8;
            brick->position.x = -1. + 1. / 13 + w * 2. / 13;
            brick->position.y = .50 - 1. / 4 / 6 - h * 2. / 4 / 6;
            brick->box.right    = 1. / 13;
            brick->box.left     = -1. / 13;
            brick->box.bottom   = -1. / 4 / 6;
            brick->box.top      = 1. / 4 / 6;

            brick->local = reservePoints(brick->points, brick->index);
            Point *current = brick->local;
            for(unsigned int c = 0; c < brick->points / 2; ++ c, ++ current)
                memcpy(&current->R, &COLOR[color][0], sizeof(Color));

            for(unsigned int c = 0; c < brick->points / 2; ++ c, ++ current)
                memcpy(&current->R, &COLOR[color][1], sizeof(Color));

            brick->local[4].x = brick->local[0].x = brick->box.left;
            brick->local[4].y = brick->local[0].y = brick->box.bottom;

            brick->local[5].x = brick->local[1].x = brick->box.right;
            brick->local[5].y = brick->local[1].y = brick->box.bottom;

            brick->local[6].x = brick->local[2].x = brick->box.right;
            brick->local[6].y = brick->local[2].y = brick->box.top;

            brick->local[7].x = brick->local[3].x = brick->box.left;
            brick->local[7].y = brick->local[3].y = brick->box.top;
            ++ brick;
            ++ local.bricks;
        }

        assert(*reader == '\n');
        ++ reader;
    }

    {
        Object &paddle = local.paddle;
        paddle.alive = true;
        paddle.points = 8;
        paddle.position.x = 0.;
        paddle.position.y = -1. + 1. / 4 / 6 + 1./ 4 / 12;
        paddle.box.right    = 1. / 10;
        paddle.box.left     = -1. / 10;
        paddle.box.bottom   = -1. / 4 / 12;
        paddle.box.top      = 1. / 4 / 12;

        paddle.local = reservePoints(paddle.points, paddle.index);

        Point *current = paddle.local;
        for(unsigned int c = 0; c < paddle.points / 2; ++ c, ++ current)
            memcpy(&current->R, &COLOR[6][0], sizeof(Color));

        for(unsigned int c = 0; c < paddle.points / 2; ++ c, ++ current)
            memcpy(&current->R, &COLOR[6][1], sizeof(Color));

        paddle.local[4].x = paddle.local[0].x = paddle.box.left;
        paddle.local[4].y = paddle.local[0].y = paddle.box.bottom;

        paddle.local[5].x = paddle.local[1].x = paddle.box.right;
        paddle.local[5].y = paddle.local[1].y = paddle.box.bottom;

        paddle.local[6].x = paddle.local[2].x = paddle.box.right;
        paddle.local[6].y = paddle.local[2].y = paddle.box.top;

        paddle.local[7].x = paddle.local[3].x = paddle.box.left;
        paddle.local[7].y = paddle.local[3].y = paddle.box.top;
    }

    {
        Object &ball = local.ball;
        ball.alive = false;
        ball.points = 16;
        ball.position.x = 0;
        ball.position.y = -1. + 1. / 4 / 6 + 2. / 4 / 12 + 1. / 4 / 12;
        ball.box.right  = 1. / 4 / 12;
        ball.box.left   = -1. / 4 / 12;
        ball.box.bottom = -1. / 4 / 12;
        ball.box.top    = 1. / 4 / 12;

        ball.local = reservePoints(ball.points, ball.index);

        Point *current = ball.local;
        for(unsigned int c = 0; c < ball.points; ++ c, ++ current)
            memcpy(&current->R, &COLOR[7][0], sizeof(Color));

        ball.local[0].x = 0.;
        ball.local[0].y = 0.;

        for(unsigned int t = 0; t < ball.points - 1; ++ t)
        {
            ball.local[t + 1].x = 1. / 4 / 12 * cos(t * M_PI * 2. / (ball.points - 2));
            ball.local[t + 1].y = 1. / 4 / 12 * sin(t * M_PI * 2. / (ball.points - 2));
        }
    }

}

/* GFLW CALLBACKS */
inline
void Engine::glfwErrorCallback(int code, const char *message)
{
    engine.debug.error("GLFW", "Error %d: %s", code, message);
}

inline
void Engine::glfwKeyCallback(GLFWwindow */*window*/, int key, int/* scancode*/, int action, int/* mods*/)
{
    switch(key)
    {
        case GLFW_KEY_ESCAPE:
        case GLFW_KEY_Q:
            if(action == GLFW_PRESS)
                engine.terminate();

            break;

        case GLFW_KEY_LEFT:
            if(action == GLFW_PRESS)
                engine.movePaddle(-1);

            else if(action == GLFW_RELEASE)
                engine.movePaddle(0);

            break;

        case GLFW_KEY_RIGHT:
            if(action == GLFW_PRESS)
                engine.movePaddle(1);

            else if(action == GLFW_RELEASE)
                engine.movePaddle(0);

            break;

        case GLFW_KEY_SPACE:
            engine.releaseBall();
            break;
    }
}

inline
void Engine::glfwWindowCloseCallback(GLFWwindow */*window*/)
{
    engine.terminate();
}

inline
Point *Engine::reservePoints(const int count, GLuint &index)
{
    Point *result = local.point + local.points;
    index = local.points;
    local.points += count;
    return result;
}

}; // namespace arkanoid

#endif // __ENGINE_INL__
