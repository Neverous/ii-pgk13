#ifndef __ANIMATIONS_H__
#define __ANIMATIONS_H__

#include "libs/logger/logger.h"
#include "libs/config/config.h"
#include "libs/thread/thread.h"

#include "objects.h"

using namespace std;

namespace walker
{

class Engine;

class Animations: public Thread
{
    friend class Engine;

    Logger log;
    Engine *engine;

    void start(void);
    void run(void);
    void stop(void);

    void terminate(void);

    void step(void);

    public:
        Animations(Log &_log, Engine *_engine);
        ~Animations(void);
        unsigned int s;
}; // class Animations

} // namespace walker

#include "animations.inl"

#endif // __ANIMATIONS_H__
