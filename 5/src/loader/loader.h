#ifndef __LOADER_H__
#define __LOADER_H__

#include "libs/logger/logger.h"
#include "libs/thread/thread.h"

#include "engine/objects.h"

namespace terrain
{

namespace loader
{

class Loader: public Thread
{
    Logger log;

    void start(void);
    void run(void);
    void stop(void);

    void terminate(void);

    void checkTiles(void);
    void loadTile(objects::Tile &tile, int something);

    public:
        Loader(Log &_log);
        ~Loader(void);
}; // class Loader

} // namespace loader

} // namespace terrain

#endif // __LOADER_H__
