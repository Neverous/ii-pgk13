/* 2014
 * Maciej Szeptuch
 * II UWr
 */
#include "defines.h"
#include <csignal>
#include <exception>
#include <cstring>

#include "libs/logger/logger.h"
#include "libs/thread/thread.h"

#include "engine/engine.h"
#include "drawer/drawer.h"

using namespace std;

ThreadMonitor   threads;

Log             debug;
Logger          logger(debug, "MAIN");

terrain::engine::Engine engine(debug);
terrain::drawer::Drawer *drawer;

void sigbreak(int signal);

int main(int argc, char **argv)
{
    debug.setLevel(Log::DEBUG);

    drawer = new terrain::drawer::Drawer(debug);

    // Connect signals
    signal(SIGINT,  sigbreak);
    signal(SIGKILL, sigbreak);
    signal(SIGTERM, sigbreak);
    signal(SIGPIPE, sigbreak);

    try
    {
        logger.debug("Running engine");
        engine.run(argc, argv);
        logger.debug("Engine finished");
    }
    catch(const exception &err)
    {
        logger.critical("Error running engine: %s", err.what());
        return 3;
    }

    return 0;
}

void sigbreak(int signal)
{
    static int count = 0;
    if(!count)
    {
        logger.info("Caught %d(%s) signal. Closing", signal, strsignal(signal));
        engine.terminate();
        count = 1;
        return;
    }

    logger.warning("Caught %d(%s) signal twice. Forcing shutdown", signal, strsignal(signal));
    engine.terminate();
    this_thread::sleep_for(chrono::seconds(1));
    terminate();
    exit(0);
}
