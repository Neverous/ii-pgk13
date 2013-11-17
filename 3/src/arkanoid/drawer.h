#ifndef __DRAWER_H__
#define __DRAWER_H__

#include "libs/logger/logger.h"
#include "libs/config/config.h"
#include "libs/thread/thread.h"

using namespace std;

namespace arkanoid
{

class Engine;

class Drawer: public Thread
{
    friend class Engine;

    Logger log;
    Engine *engine;

    struct Options
    {
        unsigned int framelimit;
    } options;

    void start(void);
    void run(void);
    void stop(void);

    void terminate(void);

    void drawBackground(void);
    void drawObjects(void);

    void loadShaders(void);

    public:
        Drawer(Log &_log, Engine *_engine);
        ~Drawer(void);
        void configure(Config &cfg);
}; // class Drawer

} // namespace arkanoid

#include "drawer.inl"

#endif // __DRAWER_H__
