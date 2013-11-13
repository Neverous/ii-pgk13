#ifndef __LOG_H__
#define __LOG_H__

#include <cstdio>
#include <cstdarg>
#include <string>
#include <mutex>

class Log
{
    friend class Logger;

    public:
        enum Level
        {
            NOLOG           = 0,
            DEBUG           = 1,
            NOTICE          = 2,
            INFO            = 3,
            WARNING         = 4,
            ERROR           = 5,
            CRITICAL        = 6,
            FATAL           = 7,
        }; // enum Level

        Log(void);
        ~Log(void);

        bool open(void);
        bool close(void);

        void setLevel(Level _level);
        void setFile(const std::string &_filename);

        void debug(const char *_name, const char *_fmt, ...);
        void notice(const char *_name, const char *_fmt, ...);
        void info(const char *_name, const char *_fmt, ...);
        void warning(const char *_name, const char *_fmt, ...);
        void error(const char *_name, const char *_fmt, ...);
        void critical(const char *_name, const char *_fmt, ...);
        void fatal(const char *_name, const char *_fmt, ...);

    private:
        int level;
        std::string filename;
        FILE *file;
        std::mutex lock;

        void message(Level _level, const char *_name, const char *_fmt, va_list args);
}; // class Log

#include "log.inl"

#endif // __LOG_H__
