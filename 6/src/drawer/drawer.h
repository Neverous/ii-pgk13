#ifndef __DRAWER_H__
#define __DRAWER_H__

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "libs/logger/logger.h"
#include "libs/thread/thread.h"

#include "engine/engine.h"

namespace viewer
{

namespace drawer
{

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
        void drawModel(void);

        void loadPrograms(void);
        GLuint loadProgram(const char *vertex, const char *fragment);
        void loadShader(const GLuint shader, const char *filename);

        void throwError(const char *message);

}; // class Drawer

} // namespace drawer

} // namespace viewer

#endif // __DRAWER_H__
