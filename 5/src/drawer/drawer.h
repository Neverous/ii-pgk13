#ifndef __DRAWER_H__
#define __DRAWER_H__

#include "libs/logger/logger.h"
#include "libs/thread/thread.h"

#include "engine/engine.h"

namespace terrain
{

namespace drawer
{

enum Program
{
    GRID_PROGRAM = 0,
    TILE_PROGRAM = 1,
}; // enum Program

class Drawer: public Thread
{
    private:
        Logger          log;
        engine::Engine  &engine;

    public:
        Drawer(Log &_log, engine::Engine &_engine);
        ~Drawer(void);

    protected:
        void start(void);
        void run(void);
        void stop(void);
        void terminate(void);

    private:
        void setupGL(void);
        void generateTile(void);
        void generateGrid(void);

        void drawGrid(int lod);
        void drawTerrain(int lod);
        void drawTile(const objects::Tile &tile, int lod);

        void loadPrograms(void);
        GLuint loadProgram(const char *vertex, const char *fragment);
        void loadShader(const GLuint shader, const char *filename);

        GLuint &getProgram(int view);
        GLuint &getMVP(int view);
        GLuint &getBOX(int view);

        void throwError(const char *message);

}; // class Drawer

} // namespace drawer

} // namespace terrain

#endif // __DRAWER_H__
