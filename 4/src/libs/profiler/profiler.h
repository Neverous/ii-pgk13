#ifndef __PROFILER_H__
#define __PROFILER_H__

#include <unistd.h>
#include <chrono>

#ifndef NDEBUG
    #define PGUARD(Name)    \
TGUARD(Name);

    #define TGUARD(Name)    \
pizza::TimeGuard __tg(Name);

#else // NDEBUG
    #define PGUARD(Name) static_cast<void>(0);
    #define TGUARD(Name) static_cast<void>(0);
#endif // NDEBUG

class TimeGuard
{
    const char                              *name;
    std::chrono::steady_clock::time_point   start;

    public:
        TimeGuard(const char *_name);
        ~TimeGuard(void);

    private:
        double calcUnit(size_t value, const char *&unit) const;
}; // class TimeGuard

#include "profiler.inl"

#endif // __PROFILER_H__
