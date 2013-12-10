#ifndef __PROFILER_INL__
#define __PROFILER_INL__

#include <cstdint>
#include <cstdio>
#include <chrono>
#include <unistd.h>

#include "profiler.h"

#define _inline inline

using namespace std::chrono;

// TIMEGUARD
_inline
TimeGuard::TimeGuard(const char *_name)
:name(_name)
,start(steady_clock::now())
{
    fprintf(stderr, "TimeGuard::%s started\n", _name);
}

_inline
TimeGuard::~TimeGuard(void)
{
    const char *unit = nullptr;
    double value = calcUnit(duration_cast<microseconds>(steady_clock::now() - start).count(), unit);
    fprintf(stderr, "TimeGuard::%s took %0.5lf%s\n", name, value, unit);
}

inline
double TimeGuard::calcUnit(size_t _value, const char *&unit) const
{
    double value = _value;
#define TIME_POINT(Limit, Unit) \
    if(value < Limit)           \
    {                           \
        unit = Unit;            \
        return value;           \
    }                           \
                                \
    value /= Limit;

    TIME_POINT(1000,    "us");
    TIME_POINT(1000,    "ms");
    TIME_POINT(60,      "s");
    TIME_POINT(60,      "m");
    TIME_POINT(24,      "h");
    unit = "d";
    return value;
#undef TIME_POINT
}

#undef _inline
#endif // __PROFILER_INL__
