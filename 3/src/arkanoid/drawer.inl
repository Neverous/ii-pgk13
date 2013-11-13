#ifndef __DRAWER_INL__
#define __DRAWER_INL__

#include "drawer.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "libs/logger/logger.h"
#include "libs/config/config.h"
#include "libs/thread/thread.h"

#define MIN_DRAWING_FPS 30

using namespace std;

namespace arkanoid
{

inline
void Drawer::start(void)
{
    pthread_setname_np(handle.native_handle(), "Drawer");

    log.debug("Starting drawer");
    glfwMakeContextCurrent(engine->glfw.window);
    if(glewInit() != GLEW_OK)
    {
        glfwDestroyWindow(engine->glfw.window);
        glfwTerminate();

        throw runtime_error("GLEWInit error");
    }
}

inline
void Drawer::run(void)
{
    log.debug("Running drawer");
    double lastFrame = 0;
    while(state == Thread::STARTED)
    {
        lastFrame = glfwGetTime();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        // drawObjects();
        glfwSwapBuffers(engine->glfw.window);

        double currentFrame = glfwGetTime();
        double diff = currentFrame - lastFrame;
        if(options.framelimit && diff < 1.L / options.framelimit)
            this_thread::sleep_for(chrono::milliseconds(static_cast<unsigned int>(1000.L / options.framelimit - diff * 1000.0L)));

        else if(diff > 1.L / MIN_DRAWING_FPS)
            log.warning("Rendering frame took: %.4lfs", diff);
    }
}

inline
void Drawer::stop(void)
{
}

inline
void Drawer::terminate(void)
{
}

inline
Drawer::Drawer(Log &_log, Engine *_engine)
:log(_log, "DRAWER")
,engine(_engine)
{
}

inline
Drawer::~Drawer(void)
{
}

inline
void Drawer::configure(Config &cfg)
{
    options.framelimit = cfg.get<unsigned int>("drawer", "framelimit", 0);
}

} // namespace arkanoid

#endif // __DRAWER_H__
