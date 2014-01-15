#ifndef __MOVEMENT_H__
#define __MOVEMENT_H__

#include "libs/logger/logger.h"
#include "libs/thread/thread.h"

#include "engine/engine.h"

namespace terrain
{

namespace movement
{

class Movement: public Thread
{
    Logger          log;
    engine::Engine  &engine;

    public:
        Movement(Log &_log, engine::Engine &_engine);
        ~Movement(void);

    protected:
        void start(void);
        void run(void);
        void stop(void);

        void terminate(void);

    private:
        void move(void);
}; // class Movement

} // namespace movement

} // namespace terrain

#endif // __MOVEMENT_H__
