#ifndef __CONFIG_INL__
#define __CONFIG_INL__

#include <string>
#include <list>
#include <unordered_map>

#include "config.h"
#include "configParser.h"
#include "libs/utils.h"

#define _inline inline

using namespace std;

// CONFIG
_inline
Config::Config(const char *_filename/* = nullptr*/)
:value()
{
    if(_filename)
        load(_filename);
}

_inline
void Config::load(const char *_filename)
{
    ConfigParser parser(_filename);
    parser.parse(value);
}

_inline
list<string> Config::getSections(void)
{
    list<string> result;
    for(const auto &_section: value)
        result.push_back(_section.first);

    return result;
}

_inline
list<string> Config::getKeys(const string &_section)
{
    list<string> result;
    if(!value.count(_section))
        return result;

    for(const auto &_option: value[_section])
        result.push_back(_option.first);

    return result;
}

template<typename Type/* = string*/>
_inline
Type Config::get(const string &_section, const string &_key)
{
    return parse<Type>(value[_section][_key]);
}

template<typename Type/* = string*/>
_inline
Type Config::get(const string &_section, const string &_key, const Type &_default)
{
    if(!value.count(_section) || !value[_section].count(_key))
        return _default;

    return parse<Type>(value[_section][_key]);
}

template<typename Type/* = string*/>
_inline
list<Type> Config::getList(const string &_section, const string &_key, const char delimiter/* = ','*/)
{
    list<Type> result;
    if(!value.count(_section) || !value[_section].count(_key))
        return result;

    string element; element.reserve(128);
    for(const char &v: value[_section][_key])
    {
        if(v == delimiter)
        {
            if(!element.empty())
                result.push_back(parse<Type>(element));

            element.clear();
            continue;
        }

        element += v;
    }

    if(!element.empty())
        result.push_back(parse<Type>(element));

    return result;
}

#undef _inline
#endif // __CONFIG_INL__
