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

    // divisiors of 64000000
    int d = 0;
    for(int t = 0; t < 13; ++ t)
    {
        divs[d ++] = (1 << t);
        for(int f = 1; f < 7; ++ f)
        {
            divs[d] = divs[d - 1] * 5;
            ++ d;
        }
    }

    assert(d == 91);
    sort(divs, divs + 91);
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
    if(!engine.gl.buffer[::engine::SWAP_BUFFER])
        return;

    uint32_t tileSize = 0;
    objects::Tile::ID _id = getFirstTile(tileSize);
    markInvalidTiles(_id, tileSize);

    for(int h = 0; h < 3; ++ h)
        for(int w = 0; w < 3; ++ w)
        {
            objects::Tile::ID __id = _id;
            __id.w += w;
            __id.h += h;

            if(any_of(engine.local.tile, engine.local.tile + 9, [__id](const objects::Tile &tile) {return tile.id.d == __id.d && tile.valid;}))
                continue;

            objects::Tile *tile = find_if(engine.local.tile, engine.local.tile + 9, [](const objects::Tile &_tile) {return !_tile.valid;});
            if(tile && tile != engine.local.tile + 9)
            {
                log.debug("Loading (%u %u) into %d tile", __id.w, __id.h, tile - engine.local.tile);
                if(!loadTile(*tile, __id, tileSize))
                    return;
            }
        }
}

inline
void Loader::markInvalidTiles(const objects::Tile::ID &_id, uint32_t tileSize)
{
    for(int t = 0; t < 9; ++ t)
    {
        objects::Tile &tile = engine.local.tile[t];
        tile.valid = tile.size == tileSize
            &&  _id.h <= tile.id.h && tile.id.h <= _id.h + 2
            &&  _id.w <= tile.id.w && tile.id.w <= _id.w + 2;
    }
}

inline
objects::Tile::ID Loader::getFirstTile(uint32_t &tileSize)
{
    glm::dvec4 view = engine.getBoundingRect();
    tileSize = *lower_bound(divs, divs + 91, (int) ((view.y - view.x) * 3 / 5));
    objects::Tile::ID _id;
    _id.w = max(0.0, min(64000000.0 - 3 * tileSize, 32000000.0 + view.x)) / tileSize;
    _id.h = max(0.0, min(64000000.0 - 3 * tileSize, 32000000.0 + view.z)) / tileSize;

    return _id;
}

inline
bool Loader::loadTile(objects::Tile &tile, const objects::Tile::ID &_id, uint32_t tileSize)
{
    const uint32_t density  = (1 << DETAIL_LEVELS) + 1;
    const uint32_t size     = density * density;
    static objects::TerrainPoint buffer[size];
    objects::Tile::ID   ID;
    glm::vec4           box;

    ID.d    = _id.d;
    box.x   = -32000000.0 + _id.w * tileSize;
    box.y   = box.x + tileSize;
    box.z   = -32000000.0 + _id.h * tileSize;
    box.w   = box.z + tileSize;
    log.debug("Tile (%u %u) box: [%.2f, %.2f, %.2f, %.2f]", ID.w, ID.h, box.x, box.y, box.z, box.w);

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
            buffer[b].x = w;
            buffer[b].y = h;
            buffer[b].height = chunk ? chunk->get(cx, cy) : 32768;
        }
    }

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, engine.gl.buffer[engine::SWAP_BUFFER]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(objects::TerrainPoint) * size, buffer, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    swap(engine.gl.buffer[engine::SWAP_BUFFER], tile.buffer);
    tile.id.d   = ID.d;
    tile.box.x  = box.x;
    tile.box.y  = box.y;
    tile.box.z  = box.z;
    tile.box.w  = box.w;

    tile.size   = tileSize;
    tile.valid  = true;
    return true;
}
