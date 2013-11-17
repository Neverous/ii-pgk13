#ifndef __PHYSICS_INL__
#define __PHYSICS_INL__

#include "physics.h"

#include "libs/logger/logger.h"
#include "libs/config/config.h"
#include "libs/thread/thread.h"

#define PHYSICS_FPS 60

using namespace std;

namespace arkanoid
{

inline
void Physics::start(void)
{
    pthread_setname_np(handle.native_handle(), "Physics");

    log.debug("Starting physics");
}

inline
void Physics::run(void)
{
    log.debug("Running physics");
    double lastFrame = 0;
    while(state == Thread::STARTED)
    {
        lastFrame = glfwGetTime();
        physicsStep();

        double currentFrame = glfwGetTime();
        double diff = currentFrame - lastFrame;
        if(diff <= 1.L / PHYSICS_FPS)
            this_thread::sleep_for(chrono::milliseconds(static_cast<unsigned int>(1000.L / PHYSICS_FPS - diff * 1000.L)));

        else
            log.warning("Physics step took: %.4lfs", diff);
    }
}

inline
void Physics::stop(void)
{
}

inline
void Physics::terminate(void)
{
}

inline
Physics::Physics(Log &_log, Engine *_engine)
:log(_log, "PHYSICS")
,engine(_engine)
{
}

inline
Physics::~Physics(void)
{
}

inline
void Physics::configure(Config &cfg)
{
    options.maxBallVelocity     = cfg.get<float>("physics", "maxBallVelocity",      .03);
    options.ballAcceleration    = cfg.get<float>("physics", "ballAcceleration",     .007);
    options.ballAngle           = cfg.get<float>("physics", "ballAngle",            .75);

    options.maxPaddleVelocity   = cfg.get<float>("physics", "maxPaddleVelocity",    .04);
    options.paddleAcceleration  = cfg.get<float>("physics", "paddleAcceleration",   .007);
}

inline
void Physics::physicsStep(void)
{
    Object &paddle = engine->local.paddle;
    Object &ball = engine->local.ball;

    // ACCELERATE
    if(paddle.acceleration.len())
    {
        paddle.velocity += paddle.acceleration;
        paddle.velocity.normalize(options.maxPaddleVelocity);
    }

    else
        paddle.velocity.clear();

    if(ball.acceleration.len())
    {
        ball.velocity += ball.acceleration;
        ball.velocity.normalize(options.maxBallVelocity);

        if(abs(ball.acceleration.y / ball.acceleration.x) < options.ballAngle)
            ball.acceleration.y *= options.ballAngle / abs(ball.acceleration.y / ball.acceleration.x);
    }

    else
        ball.velocity.clear();

    // MOVE
    paddle.position += paddle.velocity;
    ball.position += ball.velocity;

    // COLLIDE
    if(paddle.position.x + paddle.box.right > 1.)
        paddle.position.x = 1. - paddle.box.right;

    if(paddle.position.x + paddle.box.left < -1.)
        paddle.position.x = -1. - paddle.box.left;

    if(ball.alive)
    {
        // WALLS
        if(ball.position.x + ball.box.right > 1.)
        {
            ball.position.x     = 1. - ball.box.right;
            ball.velocity.x     = -ball.velocity.x;
            ball.acceleration.x = -ball.acceleration.x;
        }

        if(ball.position.x + ball.box.left < -1.)
        {
            ball.position.x = -1. - ball.box.left;
            ball.velocity.x     = -ball.velocity.x;
            ball.acceleration.x = -ball.acceleration.x;
        }

        if(ball.position.y + ball.box.top > 1.)
        {
            ball.position.y = 1. - ball.box.top;
            ball.velocity.y     = -ball.velocity.y;
            ball.acceleration.y = -ball.acceleration.y;
        }

        if(ball.position.y + ball.box.top < -1.)
        {
            ball.velocity.clear();
            ball.acceleration.clear();
            ball.alive = false;
            -- engine->local.lives;
        }

        // PADDLE
        handleCollision(ball, paddle, options.ballAcceleration);

        // BRICKS
        for(unsigned int b = 0; b < engine->local.bricks; ++ b)
            if(engine->local.brick[b].alive && handleCollision(ball, engine->local.brick[b], options.ballAcceleration))
                engine->local.brick[b].alive = false;
    }

    if(!ball.alive)
    {
        ball.position.x = paddle.position.x;
        ball.position.y = paddle.position.y + 1. / 4 / 6;
    }
}

inline
bool Physics::handleCollision(Object &a, const Object &b, const float acceleration)
{
    float diff = 0;

    Box bBox = b.box;
    bBox += b.position;

    Box aBox = a.box;
    aBox += a.position;
    Collide collision = aBox.collide(bBox);
    switch(collision)
    {
        case COLLIDE_LEFT:
            a.position.x -= (aBox.right - bBox.left) + .0001;
            diff = -1;
            break;

        case COLLIDE_RIGHT:
            a.position.x += (bBox.right - aBox.left) + .0001;
            diff = -1;
            break;

        case COLLIDE_TOP:
            a.position.y += (bBox.top - aBox.bottom) + .0001;
            diff = 1;
            break;

        case COLLIDE_BOTTOM:
            a.position.y -= (aBox.top - bBox.bottom) + .0001;
            diff = 1;
            break;

        case COLLIDE_NONE:
            return false;

        case COLLIDE_INVALID:
            log.warning("Invalid collision");
            return false;

        case COLLIDE_TOPLEFT:
            if(a.velocity.x <= 0)
                diff = 1;

            else if(a.velocity.y >= 0)
                diff = -1;

            else
                diff = (aBox.right - bBox.left) - (bBox.top - aBox.bottom);

            if(a.velocity.x > 0)
                a.position.x -= (aBox.right - bBox.left) + .0001;

            if(a.velocity.y < 0)
                a.position.y += (bBox.top - aBox.bottom) + .0001;

            break;

        case COLLIDE_BOTTOMLEFT:
            if(a.velocity.x <= 0)
                diff = 1;

            else if(a.velocity.y <= 0)
                diff = -1;

            else
                diff = (aBox.right - bBox.left) - (aBox.top - bBox.bottom);

            if(a.velocity.x > 0)
                a.position.x -= (aBox.right - bBox.left) + .0001;

            if(a.velocity.y > 0)
                a.position.y -= (aBox.top - bBox.bottom) + .0001;

            break;

        case COLLIDE_TOPRIGHT:
            if(a.velocity.x >= 0)
                diff = 1;

            else if(a.velocity.y >= 0)
                diff = -1;

            else
                diff = (bBox.right - aBox.left) - (bBox.top - aBox.bottom);

            if(a.velocity.x < 0)
                a.position.x += (bBox.right - aBox.left) + .0001;

            if(a.velocity.y < 0)
                a.position.y += (bBox.top - aBox.bottom) + .0001;

            break;

        case COLLIDE_BOTTOMRIGHT:
            if(a.velocity.x >= 0)
                diff = 1;

            else if(a.velocity.y <= 0)
                diff = -1;

            else
                diff = (bBox.right - aBox.left) - (aBox.top - bBox.bottom);

            if(a.velocity.x < 0)
                a.position.x += (bBox.right - aBox.left) + .0001;

            if(a.velocity.y > 0)
                a.position.y -= (aBox.top - bBox.bottom) + .0001;

            break;
    }

    if(diff > 0)
    {
        a.acceleration.y    = -a.acceleration.y;

        a.acceleration.x    += .3 * b.acceleration.x;
        a.acceleration.normalize(acceleration);

        a.velocity.y        = -a.velocity.y;
    }

    else if(diff < 0)
    {
        a.acceleration.x    = -a.acceleration.x;

        a.acceleration.x    += .3 * b.acceleration.x;
        a.acceleration.normalize(acceleration);

        a.velocity.x        = -a.velocity.x;
    }

    else
    {
        a.acceleration.x    = -a.acceleration.x;
        a.acceleration.y    = -a.acceleration.y;

        a.acceleration.x    += .3 * b.acceleration.x;
        a.acceleration.normalize(acceleration);

        a.velocity.x        = -a.velocity.x;
        a.velocity.y        = -a.velocity.y;
    }

    return true;
}

} // namespace arkanoid

#endif // __PHYSICS_H__
