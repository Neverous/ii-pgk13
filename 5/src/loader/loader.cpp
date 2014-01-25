#include "defines.h"
#include "loader.h"

#include <cstring>
#include <cassert>
#include <algorithm>
#include <chrono>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "engine/engine.h"
#include "engine/objects.h"
#include "projection/mercator.h"
#include "libs/logger/logger.h"

using namespace std;
using namespace terrain;
using namespace terrain::loader;
using namespace terrain::projection;

Loader::Loader(Log &_log, engine::Engine &_engine)
:log(_log, "LOADER")
,engine(_engine)
{
}

Loader::~Loader(void)
{
}

void Loader::start(void)
{
    pthread_setname_np(handle.native_handle(), "Loader");

    log.debug("Starting loader");
    glfwMakeContextCurrent(engine.gl.loader);
    if(glewInit() != GLEW_OK)
    {
        glfwDestroyWindow(engine.gl.loader);
        glfwTerminate();

        throw runtime_error("GLEWInit error");
    }

    // divisiors of MERCATOR_BOUNDS * 2.0
    int d = 0;
    for(int t = 0; t < TWO_POWER; ++ t)
    {
        divs[d ++] = (1 << t);
        for(int f = 1; f < FIVE_POWER; ++ f)
        {
            divs[d] = divs[d - 1] * 5;
            ++ d;
        }
    }

    assert(d == TWO_POWER * FIVE_POWER);
    sort(divs, divs + d);
}

void Loader::run(void)
{
    log.debug("Running loader");
    double lastFrame = 0;
    while(state == Thread::STARTED)
    {
        lastFrame = glfwGetTime();
        checkTiles();
        glFlush();

        double currentFrame = glfwGetTime();
        double diff = currentFrame - lastFrame;

        if(diff < 1.0L / LOADER_FPS)
            this_thread::sleep_for(chrono::milliseconds(static_cast<uint32_t>(1000.0L / (LOADER_FPS - 1) - diff * 1000.0L)));

        else if(diff > 1.0L / (LOADER_FPS - 1))
            log.warning("Checking tiles took: %.4lfs", diff);
    }
}

void Loader::stop(void)
{
}

void Loader::terminate(void)
{
}

inline
void Loader::checkTiles(void)
{
    if(!engine.gl.buffer[::engine::SWAP_BUFFER_1])
        return;

    uint32_t tileSize = 0;
    objects::Tile::ID _id = getFirstTile(tileSize);
    markInvalidTiles(_id, tileSize);

    for(int t = 0; t < 9; ++ t)
        if(!engine.local.tile[t].valid)
        {
            objects::Tile::ID __id;
            __id.h = _id.h + engine.local.tile[t].order / 3;
            __id.w = _id.w + engine.local.tile[t].order % 3;
            if(!loadTile(t, __id, tileSize))
                return;
        }

    for(int t = 0; t < 9; ++ t)
        if(!engine.local.tile[t].valid)
        {
            objects::Tile::ID __id;
            __id.h = _id.h + engine.local.tile[t].order / 3;
            __id.w = _id.w + engine.local.tile[t].order % 3;
            if(!swapTile(engine.local.tile[t], __id, tileSize, t))
                return;
        }
}

inline
void Loader::markInvalidTiles(const objects::Tile::ID &_id, uint32_t tileSize)
{
    bool used[9]    = {};
    bool ordered[9] = {};
    for(int h = 0; h < 3; ++ h)
        for(int w = 0; w < 3; ++ w)
        {
            objects::Tile *tile = find_if(engine.local.tile, engine.local.tile + 9, [tileSize, _id, h, w](const objects::Tile &_tile) {return _tile.id.h == _id.h + h && _tile.id.w == _id.w + w && _tile.size == tileSize;});
            if(tile && tile != engine.local.tile + 9)
            {
                tile->order = h * 3 + w;
                used[h * 3 + w] = true;
                ordered[tile - engine.local.tile] = true;
            }
        }

    for(int t = 0; t < 9; ++ t)
        if(!ordered[t] && !used[engine.local.tile[t].order])
        {
            ordered[t] = true;
            used[engine.local.tile[t].order] = true;
            engine.local.tile[t].valid = false;
        }

    for(int t = 0; t < 9; ++ t)
        for(int o = 0; !ordered[t] && o < 9; ++ o)
            if(!used[o])
            {
                ordered[t] = true;
                used[o] = true;
                engine.local.tile[t].order = o;
                engine.local.tile[t].valid = false;
            }
}

inline
objects::Tile::ID Loader::getFirstTile(uint32_t &tileSize)
{
    glm::dvec4 view = engine.getBoundingRect();
    tileSize = *lower_bound(divs, divs + TWO_POWER * FIVE_POWER, (int) (sqrt((view.y - view.x) * (view.y - view.x) + (view.w - view.z) * (view.w - view.z)) * 3 / 5));
    objects::Tile::ID _id;
    _id.w = max(0.0, min(MERCATOR_BOUNDS * 2.0 - 3 * tileSize, MERCATOR_BOUNDS + view.x)) / tileSize;
    _id.h = max(0.0, min(MERCATOR_BOUNDS * 2.0 - 3 * tileSize, MERCATOR_BOUNDS + view.z)) / tileSize;

    return _id;
}

inline
bool Loader::loadTile(uint8_t t, const objects::Tile::ID &_id, uint32_t tileSize)
{
    const uint32_t density  = (1 << DETAIL_LEVELS) + 1;
    const uint32_t size     = density * density;
    glBindBuffer(GL_ARRAY_BUFFER, engine.gl.buffer[engine::SWAP_BUFFER_1 + t]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(objects::TerrainPoint) * size, nullptr, GL_DYNAMIC_DRAW);
    objects::TerrainPoint *buffer = (objects::TerrainPoint *) glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
    objects::Tile::ID   ID;
    glm::vec4           box;

    ID.d    = _id.d;
    box.x   = -MERCATOR_BOUNDS + _id.w * tileSize;
    box.y   = box.x + tileSize;
    box.z   = -MERCATOR_BOUNDS + _id.h * tileSize;
    box.w   = box.z + tileSize;
    log.debug("Tile (%u %u) box: [%.2f, %.2f, %.2f, %.2f] size: %d", ID.w, ID.h, box.x, box.y, box.z, box.w, tileSize);

    int16_t lon = -32768;
    int16_t lat = -32768;

    unordered_map<int16_t, hgt::Map>    *row    = nullptr;
    hgt::Map                            *chunk  = nullptr;
    for(uint16_t h = 0; h < density; ++ h)
    {
        const double    y       = box.z + (box.w - box.z) * h / (density - 1);
        const double    _lat    = mercator::metToLat(y);
        const int16_t   __lat   = floor(_lat);
        const int16_t   cy      = floor((_lat - __lat) * 1200);

        assert(0 <= cy && cy <= 1200);
        if(__lat != lat)
        {
            lat     = __lat;
            row     = engine.local.world.count(lat) ? &engine.local.world[lat] : nullptr;
            chunk   = nullptr;
            lon     = -32768;
        }

        for(uint16_t w = 0; w < density; ++ w)
        {
            const double    x       = box.x + (box.y - box.x) * w / (density - 1);
            const double    _lon    = mercator::metToLon(x);
            const int16_t   __lon   = floor(_lon);
            const int16_t   cx      = floor((_lon - __lon) * 1200);
            const uint32_t  b       = h * density + w;

            assert(0 <= cx && cx <= 1200);
            if(__lon != lon)
            {
                lon     = __lon;
                chunk   = row && row->count(lon) ? &(*row)[lon] : nullptr;
            }

            assert(lon != -32768 && lat != -32768);

            assert(b < size);
            buffer[b].height = chunk ? chunk->get(cx, cy) : 32768;
        }
    }

    glUnmapBuffer(GL_ARRAY_BUFFER);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    return true;
}

inline
bool Loader::swapTile(objects::Tile &tile, const objects::Tile::ID &_id, uint32_t tileSize, uint8_t t)
{
    swap(engine.gl.buffer[engine::SWAP_BUFFER_1 + t], tile.buffer);
    tile.id.d   = _id.d;
    tile.box.x  = -MERCATOR_BOUNDS + _id.w * tileSize;
    tile.box.y  = tile.box.x + tileSize;
    tile.box.z  = -MERCATOR_BOUNDS + _id.h * tileSize;
    tile.box.w  = tile.box.z + tileSize;

    tile.size   = tileSize;
    tile.valid  = true;
    return true;
}
