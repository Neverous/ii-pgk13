#include "defines.h"
#include "loader.h"

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
}

void Loader::run(void)
{
    log.debug("Running loader");
    double lastFrame = 0;
    while(state == Thread::STARTED)
    {
        lastFrame = glfwGetTime();
        checkTiles();

        double currentFrame = glfwGetTime();
        double diff = currentFrame - lastFrame;

        if(diff < 1.0L / LOADER_FPS)
            this_thread::sleep_for(chrono::milliseconds(static_cast<unsigned int>(1000.0L / (LOADER_FPS - 1) - diff * 1000.0L)));

        else if(diff > 1.0L / (LOADER_FPS - 1))
            log.warning("Checking frame took: %.4lfs", diff);
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
    // NO FUCKING IDEA
}

Loader::Loader(Log &_log)
:log(_log, "LOADER")
{
}

Loader::~Loader(void)
{
}
