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
        // physicsStep();

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
void Physics::configure(Config &/*cfg*/)
{
}

} // namespace arkanoid

#endif // __PHYSICS_H__
