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
    uint32_t divs[128];

    void start(void);
    void run(void);
    void stop(void);

    void terminate(void);

    void checkTiles(double zoom);
    void markInvalidTiles(const objects::Tile::ID &_id);
    objects::Tile::ID getFirstTile(uint32_t &tileSize);
    bool loadTile(objects::Tile &tile, const objects::Tile::ID &_id, uint32_t tileSize, double zoom);

    public:
        Loader(Log &_log);
        ~Loader(void);
}; // class Loader

} // namespace loader

} // namespace terrain

#endif // __LOADER_H__
