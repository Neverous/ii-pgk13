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

extern engine::Engine   engine;

void Loader::start(void)
{
    pthread_setname_np(handle.native_handle(), "Loader");
    log.debug("Starting loader");
    glfwMakeContextCurrent(::engine.gl.loader);
    if(glewInit() != GLEW_OK)
    {
        glfwDestroyWindow(::engine.gl.loader);
        glfwTerminate();

        throw runtime_error("GLEWInit error");
    }

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
        checkTiles(::engine.local.zoom);
        glFlush();

        double currentFrame = glfwGetTime();
        double diff = currentFrame - lastFrame;

        if(diff < 1.0L / LOADER_FPS)
            this_thread::sleep_for(chrono::milliseconds(static_cast<unsigned int>(1000.0L / (LOADER_FPS - 1) - diff * 1000.0L)));

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
void Loader::checkTiles(double zoom)
{
    if(!::engine.gl.buffer[SWAPPING_BUFFER])
        return;

    unsigned int tileSize = 0;
    objects::Tile::ID _id = getFirstTile(tileSize);
    markInvalidTiles(_id);

    for(int h = 0; h < 3; ++ h)
        for(int w = 0; w < 3; ++ w)
        {
            objects::Tile::ID __id = _id;
            __id.w += w;
            __id.h += h;
            bool cont = false;
            for(int t = 0; !cont && t < 9; ++ t)
                cont = ::engine.local.tile[t].id.d == __id.d && ::engine.local.tile[t].synchronized == objects::Tile::Status::SYNCHRONIZED;

            if(cont)
                continue;

            for(int t = 0; t < 9; ++ t)
                if(::engine.local.tile[t].synchronized != objects::Tile::Status::SYNCHRONIZED)
                {
                    log.debug("Loading into %d", t);
                    if(!loadTile(::engine.local.tile[t], __id, tileSize, zoom))
                        return;

                    break;
                }
        }
}

inline
void Loader::markInvalidTiles(const objects::Tile::ID &_id)
{
    for(int t = 0; t < 9; ++ t)
    {
        objects::Tile &tile = ::engine.local.tile[t];
        if(tile.zoom != ::engine.local.zoom)
            tile.synchronized = objects::Tile::Status::SCALED;

        else
        {
            bool exist = false;
            for(int h = 0; !exist && h < 3; ++ h)
                for(int w = 0; !exist && w < 3; ++ w)
                    exist = tile.id.h == _id.h + h && tile.id.w == _id.w + w;

            if(!exist)
                tile.synchronized = objects::Tile::Status::DESYNCHRONIZED;
        }

        //if(tile.synchronized != objects::Tile::Status::SYNCHRONIZED)
        //    log.debug("Tile %d marked (%u, %u) %d", t, tile.id.w, tile.id.h, tile.synchronized);
    }
}

inline
objects::Tile::ID Loader::getFirstTile(unsigned int &tileSize)
{
    glm::dvec4 view = ::engine.getView();
    tileSize = *lower_bound(divs, divs + 91, (view.y - view.x) * 3 / 5);
    objects::Tile::ID _id;
    _id.w = min(64000000.0 - 3 * tileSize, max(0.0, 32000000.0 + view.x)) / tileSize;
    _id.h = min(64000000.0 - 3 * tileSize, max(0.0, 32000000.0 + view.z)) / tileSize;

    return _id;
}

inline
bool Loader::loadTile(objects::Tile &tile, const objects::Tile::ID &_id, unsigned int tileSize, double zoom)
{
    const uint32_t density = (1 << TILE_DENSITY_BITS) + 1;
    const uint32_t size = density * density;
    static objects::TerrainPoint buffer[size];
    objects::Tile::ID ID;
    objects::Tile::BoundingBox box;

    ID.d = _id.d;
    box.left   = -32000000.0 + _id.w * tileSize;
    box.right  = box.left + tileSize;
    box.bottom = -32000000.0 + _id.h * tileSize;
    box.top    = box.bottom + tileSize;
    log.debug("Loading tile (%u, %u) [%.2f, %.2f, %.2f, %.2f] {%u}", ID.w, ID.h, box.left, box.right, box.bottom, box.top, tileSize);

    int16_t lon = -32768;
    int16_t lat = -32768;

    unordered_map<int16_t, vector<vector<uint16_t> > >  *row    = nullptr;
    vector<vector<uint16_t> >                           *chunk  = nullptr;
    for(unsigned int h = 0; h < density; ++ h)
    {
        const double    y       = box.bottom + (box.top - box.bottom) * h / (density - 1);
        const double    _lat    = mercator::metToLat(y);
        const int16_t   __lat   = floor(_lat);
        const int16_t   cy      = floor((_lat - __lat) * 1200);
        assert(0 <= cy && cy <= 1200);
        if(__lat != lat)
        {
            lat = __lat;
            if(::engine.local.world.count(lat))
                row = &::engine.local.world[lat];

            else
                row = nullptr;

            chunk = nullptr;
            lon = -32768;
        }

        for(unsigned int w = 0; w < density; ++ w)
        {
            const double    x       = box.left + (box.right - box.left) * w / (density - 1);
            const double    _lon    = mercator::metToLon(x);
            const int16_t   __lon   = floor(_lon);
            const int16_t   cx      = floor((_lon - __lon) * 1200);
            assert(0 <= cx && cx <= 1200);
            if(__lon != lon)
            {
                lon = __lon;
                if(row && row->count(lon))
                    chunk = &(*row)[lon];

                else
                    chunk = nullptr;
            }

            assert(lon != -32768 && lat != -32768);

            const uint32_t b = h * density + w;
            assert(b < size);
            buffer[b].x = w;
            buffer[b].y = h;
            buffer[b].height = chunk ? (*chunk)[cy][cx] : 32768;
            if(zoom != ::engine.local.zoom)
                return false;
        }
    }

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ::engine.gl.buffer[SWAPPING_BUFFER]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(objects::TerrainPoint) * size, buffer, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    tile.synchronized = objects::Tile::Status::DESYNCHRONIZED;
    swap(::engine.gl.buffer[SWAPPING_BUFFER], tile.buffer);
    tile.id.d = ID.d;
    tile.box.left   = box.left;
    tile.box.right  = box.right;
    tile.box.bottom = box.bottom;
    tile.box.top    = box.top;

    tile.zoom           = ::engine.local.zoom;
    tile.synchronized   = objects::Tile::Status::SYNCHRONIZED;
    return true;
}

Loader::Loader(Log &_log)
:log(_log, "LOADER")
{
}

Loader::~Loader(void)
{
}
