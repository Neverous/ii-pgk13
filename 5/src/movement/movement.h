#ifndef __MOVEMENT_H__
#define __MOVEMENT_H__

#include "libs/logger/logger.h"
#include "libs/thread/thread.h"

namespace terrain
{

namespace movement
{

class Movement: public Thread
{
    Logger log;

    void start(void);
    void run(void);
    void stop(void);

    void terminate(void);

    void move(void);

    public:
        Movement(Log &_log);
        ~Movement(void);
}; // class Movement

} // namespace movement

} // namespace terrain

#endif // __MOVEMENT_H__
