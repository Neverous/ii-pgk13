#ifndef __DRAWER_H__
#define __DRAWER_H__

#include "libs/logger/logger.h"
#include "libs/thread/thread.h"

#include "engine/objects.h"

namespace terrain
{

namespace drawer
{

class Drawer: public Thread
{
    Logger log;

    void start(void);
    void run(void);
    void stop(void);

    void terminate(void);

    void drawTerrain(int lod);
    void drawFoundation(void);
    void drawTile(objects::Tile &tile, int lod);

    void loadShaders(void);
    GLuint loadShader(const char *vertex, const char *fragment);

    public:
        Drawer(Log &_log);
        ~Drawer(void);
}; // class Drawer

} // namespace drawer

} // namespace terrain

#endif // __DRAWER_H__
