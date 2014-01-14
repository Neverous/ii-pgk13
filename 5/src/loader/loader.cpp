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
#include "libs/logger/logger.h"

using namespace std;
using namespace terrain;
using namespace terrain::loader;

extern engine::Engine   engine;

void Loader::start(void)
{
    pthread_setname_np(handle.native_handle(), "Drawer");
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
        checkTiles();
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
void Loader::checkTiles(void)
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
                    loadTile(::engine.local.tile[t], __id, tileSize);
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
    _id.h = min(64000000.0 - 3 * tileSize, max(0.0, 32000000.0 + view.z)) / tileSize;
    _id.w = min(64000000.0 - 3 * tileSize, max(0.0, 32000000.0 + view.x)) / tileSize;

    return _id;
}

inline
void Loader::loadTile(objects::Tile &tile, const objects::Tile::ID &_id, unsigned int tileSize)
{
    tile.id.d = _id.d;
    tile.box.left   = -32000000.0 + _id.w * tileSize;
    tile.box.right  = tile.box.left + tileSize;
    tile.box.bottom = -32000000.0 + _id.h * tileSize;
    tile.box.top    = tile.box.bottom + tileSize;
    log.debug("Loading tile (%u, %u) [%.2f, %.2f, %.2f, %.2f] {%u}", tile.id.w, tile.id.h, tile.box.left, tile.box.right, tile.box.bottom, tile.box.top, tileSize);

    const uint32_t density = (1 << TILE_DENSITY_BITS) + 1;
    const uint32_t size = density * density;
    static objects::TerrainPoint buffer[size] = {};
    for(unsigned int h = 0; h < density; ++ h)
        for(unsigned int w = 0; w < density; ++ w)
        {
            buffer[h * density + w].x = w;
            buffer[h * density + w].y = h;
            buffer[h * density + w].height = 0;
        }

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ::engine.gl.buffer[SWAPPING_BUFFER]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(objects::TerrainPoint) * size, buffer, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    swap(::engine.gl.buffer[SWAPPING_BUFFER], tile.buffer);

    tile.zoom           = ::engine.local.zoom;
    tile.synchronized   = objects::Tile::Status::SYNCHRONIZED;
}

Loader::Loader(Log &_log)
:log(_log, "LOADER")
{
}

Loader::~Loader(void)
{
}
