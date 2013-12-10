#ifndef __LOGGER_H__
#define __LOGGER_H__

#include <cstdarg>
#include "log.h"

#define LOGGER(level) {\
va_list args; va_start(args, _fmt);\
log.message(level, name.c_str(), _fmt, args);\
va_end(args);\
}

class Logger
{
    public:
        Logger(Log &_log, const std::string &_name);

        void debug(const char *_fmt, ...);
        void notice(const char *_fmt, ...);
        void info(const char *_fmt, ...);
        void warning(const char *_fmt, ...);
        void error(const char *_fmt, ...);
        void critical(const char *_fmt, ...);
        void fatal(const char *_fmt, ...);

    private:
        Log &log;
        const std::string name;
}; // class Logger

// LOGGER
inline
Logger::Logger(Log &_log, const std::string &_name)
:log(_log)
,name(_name)
{
}

inline
void Logger::debug(const char *_fmt, ...)
{
    LOGGER(Log::Level::DEBUG);
}

inline
void Logger::notice(const char *_fmt, ...)
{
    LOGGER(Log::Level::NOTICE);
}

inline
void Logger::info(const char *_fmt, ...)
{
    LOGGER(Log::Level::INFO);
}

inline
void Logger::warning(const char *_fmt, ...)
{
    LOGGER(Log::Level::WARNING);
}

inline
void Logger::error(const char *_fmt, ...)
{
    LOGGER(Log::Level::ERROR);
}

inline
void Logger::critical(const char *_fmt, ...)
{
    LOGGER(Log::Level::CRITICAL);
}

inline
void Logger::fatal(const char *_fmt, ...)
{
    LOGGER(Log::Level::FATAL);
}

#endif // __LOGGER_H__
