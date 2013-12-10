#ifndef __CONFIG_H__
#define __CONFIG_H__

#include <string>
#include <list>
#include <unordered_map>

using namespace std;

class Config
{
    public:
        // Options available in section from ini file
        typedef unordered_map<string, string> Section;
        // All sections from ini file
        typedef unordered_map<string, Section> Container;

        Config(const char *filename = nullptr);

        // Load ini-like configuration from given file
        void load(const char *filename);

        // List names of all sections in config
        list<string> getSections(void);

        // Get keys in specified section
        list<string> getKeys(const string &_section);

        // Get value for key in section
        template<typename Type = string>
        Type get(const string &_section, const string &_key);

        template<typename Type = string>
        Type get(const string &_section, const string &_key, const Type &_default);

        // Get list of values for key in section
        template<typename Type = string>
        list<Type> getList(const string &_section, const string &_key, const char delimiter = ',');

    private:
        // storage for read data
        Container value;
}; // class Config

#include "config.inl"

#endif // __CONFIG_H__
