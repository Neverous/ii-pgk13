/* 2013
 * Maciej Szeptuch
 * II UWr
 */
#include "defines.h"
#include <csignal>
#include <exception>

#include "libs/logger/logger.h"
#include "libs/config/config.h"
#include "libs/thread/thread.h"

#include "walker/engine.h"

using namespace std;
using namespace walker;

ThreadMonitor   threads;

Log             debug;
Logger          logger(debug, "MAIN");

Engine          engine(debug);

void sigbreak(int signal);

int main(void)
{
    debug.setLevel(Log::DEBUG);

    // Connect signals
    signal(SIGINT,  sigbreak);
    signal(SIGKILL, sigbreak);
    signal(SIGTERM, sigbreak);
    signal(SIGPIPE, sigbreak);

    try
    {
        logger.debug("Running engine");
        engine.run();
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
        logger.info("Catched %d(%s) signal. Closing", signal, strsignal(signal));
        engine.terminate();
        count = 1;
        return;
    }

    logger.warning("Catched %d(%s) signal twice. Forcing shutdown", signal, strsignal(signal));
    engine.terminate();
    this_thread::sleep_for(chrono::seconds(1));
    terminate();
    exit(0);
}