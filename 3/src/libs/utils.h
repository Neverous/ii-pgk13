#ifndef __UTILS_H__
#define __UTILS_H__

#include <cstdarg>
#include <exception>
#include <stdexcept>
#include <string>
#include <cstdio>

using namespace std;

template<typename Type>
Type parse(const string &_in);

inline
char hex(char c)
{
    if(c >= 'a')
        return c - 'a' + 10;

    return c - '0';
}

inline
void throwError(const char *_fmt, ...)
{
    va_list args; va_start(args, _fmt);
    char buffer[131072] = {};
    vsnprintf(buffer, 131071, _fmt, args);
    va_end(args);

    throw runtime_error(buffer);
}

template<>
inline
int parse<int>(const string &_in)
{
    int result = 0;
    sscanf(_in.c_str(), "%d", &result);
    return result;
}

template<>
inline
unsigned int parse<unsigned int>(const string &_in)
{
    int result = 0;
    sscanf(_in.c_str(), "%u", &result);
    return result;
}

template<>
inline
double parse<double>(const string &_in)
{
    double result = 0;
    sscanf(_in.c_str(), "%lf", &result);
    return result;
}

template<>
inline
float parse<float>(const string &_in)
{
    float result = 0;
    sscanf(_in.c_str(), "%f", &result);
    return result;
}

template<>
inline
string parse<string>(const string &_in)
{
    return _in;
}

#endif // __UTILS_H__
