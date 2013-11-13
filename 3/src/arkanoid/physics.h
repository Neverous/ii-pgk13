#ifndef __PHYSICS_H__
#define __PHYSICS_H__

#include "libs/logger/logger.h"
#include "libs/config/config.h"
#include "libs/thread/thread.h"

using namespace std;

namespace arkanoid
{

class Engine;

class Physics: public Thread
{
    Logger log;
    Engine *engine;

    struct Options
    {
    } options;

    void start(void);
    void run(void);
    void stop(void);

    void terminate(void);

    public:
        Physics(Log &_log, Engine *_engine);
        ~Physics(void);
        void configure(Config &cfg);
}; // class Physics

} // namespace arkanoid

#include "physics.inl"

#endif // __PHYSICS_H__
