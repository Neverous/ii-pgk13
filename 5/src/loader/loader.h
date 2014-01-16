#ifndef __LOADER_H__
#define __LOADER_H__

#include "libs/logger/logger.h"
#include "libs/thread/thread.h"

#include "engine/engine.h"
#include "engine/objects.h"

namespace terrain
{

namespace loader
{

class Loader: public Thread
{
    Logger          log;
    engine::Engine  &engine;
    uint32_t        divs[128];

    public:
        Loader(Log &_log, engine::Engine &_engine);
        ~Loader(void);

    protected:
        void start(void);
        void run(void);
        void stop(void);

        void terminate(void);

    private:
        void checkTiles(void);
        void markInvalidTiles(const objects::Tile::ID &_id, uint32_t tileSize);
        objects::Tile::ID getFirstTile(uint32_t &tileSize);
        bool loadTile(uint8_t t, const objects::Tile::ID &_id, uint32_t tileSize);
        bool swapTile(objects::Tile &tile, const objects::Tile::ID &_id, uint32_t tileSize, uint8_t t);
}; // class Loader

} // namespace loader

} // namespace terrain

#endif // __LOADER_H__
