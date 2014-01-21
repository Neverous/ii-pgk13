#ifndef __LOG_INL__
#define __LOG_INL__

#include <cassert>
#include <cstdio>
#include <cstdarg>
#include <string>
#include <mutex>
#include <chrono>

#include "log.h"

#ifdef INLINE_BUILD
    #define _inline inline
#else // INLINE_BUILD
    #define _inline
#endif // INLINE_BUILD

#define LOGGER(level) {\
va_list args; va_start(args, _fmt);\
message(level, _name, _fmt, args);\
va_end(args);\
}

// LOG
_inline
Log::Log(void)
:level(Log::Level::INFO)
,filename()
,file(stderr)
#if LOG_MULTITHREADED
,lock()
#endif // LOG_MULTITHREADED
{
}

_inline
Log::~Log(void)
{
    close();
}

_inline
void Log::setLevel(Log::Level _level)
{
    level = _level;
}

_inline
void Log::setFile(const std::string &_file)
{
    filename = _file;
}

_inline
void Log::debug(const char *_name, const char *_fmt, ...)
{
    LOGGER(Log::Level::DEBUG);
}

_inline
void Log::notice(const char *_name, const char *_fmt, ...)
{
    LOGGER(Log::Level::NOTICE);
}

_inline
void Log::info(const char *_name, const char *_fmt, ...)
{
    LOGGER(Log::Level::INFO);
}

_inline
void Log::warning(const char *_name, const char *_fmt, ...)
{
    LOGGER(Log::Level::WARNING);
}

_inline
void Log::error(const char *_name, const char *_fmt, ...)
{
    LOGGER(Log::Level::ERROR);
}

_inline
void Log::critical(const char *_name, const char *_fmt, ...)
{
    LOGGER(Log::Level::CRITICAL);
}

_inline
void Log::fatal(const char *_name, const char *_fmt, ...)
{
    LOGGER(Log::Level::FATAL);
}

_inline
bool Log::open(void)
{
    if(!filename.empty())
        file = fopen(filename.c_str(), "ab");

    if(!file)
    {
        perror("Cannot open log file");
        return false;
    }

    notice("LOG", "---------- Log opened! ----------");
    return true;
}

_inline
bool Log::close(void)
{
    if(!file)
        return false;

    notice("LOG", "---------- Log closed! ----------");
    if(file != stderr && fclose(file))
    {
        file = nullptr;
        perror("Cannot close log file");
        return false;
    }

    file = nullptr;
    return true;
}

_inline
void Log::message(Level _level, const char *_name, const char *_fmt, va_list args)
{
    const char *name[8] = {
        "NOLOG",
        "DEBUG",
        "NOTICE",
        "INFO",
        "WARNING",
        "ERROR",
        "CRITICAL",
        "FATAL",
    }; // level -> name mapping

    assert(file);
    if(_level < level)
        return;

    using namespace std::chrono;

    char output[1024] = {};
    auto now = system_clock::now();
    auto now_c = system_clock::to_time_t(now);
    uint32_t millisec = duration_cast<milliseconds>(now.time_since_epoch()).count() % 1000;
    int o = 0;

    o = strftime(output, 1023, "[%Y-%m-%d %H:%M:%S", localtime(&now_c));
    o += snprintf(output + o, 1023 - o, ":%03d|%s] %s: ", millisec, _name, name[_level]);
    if(o > 1023) o = 1024;
    o += vsnprintf(output + o, 1023 - o, _fmt, args);
    if(o > 1023) o = 1024;
    snprintf(output + o, 1023 - o, "\n");
    {
#if LOG_MULTITHREADED
        std::lock_guard<std::mutex> _lock(lock);
#endif // LOG_MULTITHREADED
        fputs(output, file);
        fflush(file);
    }
}

#undef LOGGER
#undef _inline
#endif // __LOG_INL__
