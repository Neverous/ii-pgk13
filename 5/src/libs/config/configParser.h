#ifndef __CONFIGPARSER_H__
#define __CONFIGPARSER_H__

#include <cstdlib>
#include <cstring>
#include <string>
#include <stdexcept>
#include <libgen.h>
#include "libs/file/chunkFileReader.h"
#include "libs/utils.h"

#ifndef CONFIG_IMPORT_EXTENSION
    #define CONFIG_IMPORT_EXTENSION true
#endif // CONFIG_IMPORT_EXTENSION

#ifndef CONFIG_INLINE_COMMENTS
    #define CONFIG_INLINE_COMMENTS  true
#endif // CONFIG_INLINE_COMMENTS

using namespace std;

class ConfigParser
{
    public:
        ConfigParser(const char *_filename);

        // Parse given file into output structure
        template<typename Type>
        void parse(Type &output);

    private:
        string section,
               key,
               value;
#if CONFIG_IMPORT_EXTENSION
        string basedir;
#endif // __IMPORT_EXTENSTION__
        ChunkFileReader reader;

        // Parse ini section
        template<typename Type>
        void parseSection(Type &output);

        // Parse ini option
        template<typename Type>
        void parseOption(Type &output);

#if CONFIG_IMPORT_EXTENSION
        // Parse #import directive - extension to support importing from other files
        template<typename Type>
        void parseImport(Type &output);
#endif // CONFIG_IMPORT_EXTENSION

        // Parse comment - starting with ;
        void parseComment(void);

        // Parse section name
        void parseSectionName(void);

        // Parse key
        void parseKey(void);

        // Parse value
        void parseValue(void);

        // Skip spaces
        void skipSpaces(void);
}; // class ConfigParser

// CONFIGPARSER
inline
ConfigParser::ConfigParser(const char *_filename)
:section()
,key()
,value()
#if CONFIG_IMPORT_EXTENSION
,basedir()
#endif // CONFIG_IMPORT_EXTENSION
,reader(_filename)
{
    section.reserve(128);
    key.reserve(128);
    value.reserve(128);
#if CONFIG_IMPORT_EXTENSION
    char buffer[1024] = {};
    strncpy(buffer, _filename, 1023);
    basedir = dirname(buffer);
#endif // CONFIG_IMPORT_EXTENSION
}

template<typename Type>
inline
void ConfigParser::parse(Type &output)
{
    while(*reader != EOF)
    {
        skipSpaces();
        switch(*reader)
        {
            case EOF:
                return;

            case ';':
                parseComment();
                break;

            case '[':
                parseSection(output);
                break;

#if CONFIG_IMPORT_EXTENSION
            case '#':
                parseImport(output);
                break;
#endif // CONFIG_IMPORT_EXTENSION

            default:
                throwError("Error: unknown token '%c'(%d) in file %s on line %d at column %d!", *reader, *reader, reader.filename.c_str(), reader.line, reader.column);
                break;
        }
    }
}

template<typename Type>
inline
void ConfigParser::parseSection(Type &output)
{
    parseSectionName();
    while(*reader != EOF)
    {
        skipSpaces();
        switch(*reader)
        {
            case EOF:
                return;

            case '[':
                return;

            case ';':
                parseComment();
                break;

#if CONFIG_IMPORT_EXTENSION
            case '#':
                parseImport(output);
                break;
#endif // CONFIG_IMPORT_EXTENSION

            default:
                parseOption(output);
                break;
        }
    }
}

template<typename Type>
inline
void ConfigParser::parseOption(Type &output)
{
    parseKey();
    if(*reader != '=')
        throwError("Error: invalid token '%c'(%d) in file %s on line %d at column %d! '=' expected.", *reader, *reader, reader.filename.c_str(), reader.line, reader.column);

    ++ reader;
    skipSpaces();
    parseValue();
    output[section][key] = value;
}

#if CONFIG_IMPORT_EXTENSION
template<typename Type>
inline
void ConfigParser::parseImport(Type &output)
{
    ++ reader;
    for(const char &v: "import")
    {
        if(!v) break;

        if(*reader != v)
            throwError("Error: invalid directive in file %s on line %d at column %d!", reader.filename.c_str(), reader.line, reader.column);

        ++ reader;
    }

    skipSpaces();
    string filename;
    filename.reserve(128);
#if CONFIG_INLINE_COMMENTS
    while(*reader != EOF && *reader != '\r' && *reader != '\n' && *reader != ';')
#else // CONFIG_INLINE_COMMENTS
    while(*reader != EOF && *reader != '\r' && *reader != '\n')
#endif // CONFIG_INLINE_COMMENTS
    {
        filename += *reader;
        ++ reader;
    }

    if(filename[0] != '/')
    {
        char name[1024] = {};
        snprintf(name, 1023, "%s/%s", basedir.c_str(), filename.c_str());
        filename = name;
    }

    ConfigParser import(filename.c_str());
    import.parse(output);
}
#endif // CONFIG_IMPORT_EXTENSION

inline
void ConfigParser::parseComment()
{
    while(*reader != EOF && *reader != '\n' && *reader != '\r')
        ++ reader;
}

inline
void ConfigParser::parseSectionName(void)
{
    section.clear();
    ++ reader;
    skipSpaces();
    while(*reader != EOF && isalnum(*reader))
    {
        section += *reader;
        ++ reader;
    }

    skipSpaces();
    if(*reader != ']')
        throwError("Error: invalid token '%c'(%d) in file %s on line %d at column %d! ']' expected.", *reader, *reader, reader.filename.c_str(), reader.line, reader.column);

    ++ reader;
    if(section.empty())
        throwError("Error: empty section name in file %s on line %d at column %d!", reader.filename.c_str(), reader.line, reader.column);
}

inline
void ConfigParser::parseKey(void)
{
    if(!isalnum(*reader))
        throwError("Error: invalid token '%c'(%d) in file %s on line %d at column %d! Only alphanumeric characters allowed in key name!", *reader, *reader, reader.filename.c_str(), reader.line, reader.column);

    key.clear();
    while(isalnum(*reader))
    {
        key += *reader;
        ++ reader;
    }

    skipSpaces();
    if(key.empty())
        throwError("Error: empty key name in file %s on line %d at column %d!", reader.filename.c_str(), reader.line, reader.column);
}

inline
void ConfigParser::parseValue(void)
{
    value.clear();
#if CONFIG_INLINE_COMMENTS
    while(*reader != EOF && *reader != '\r' && *reader != '\n' && *reader != ';')
#else // CONFIG_INLINE_COMMENTS
    while(*reader != EOF && *reader != '\r' && *reader != '\n')
#endif // CONFIG_INLINE_COMMENTS
    {
        value += *reader;
        ++ reader;
    }

    while(!value.empty() && isspace(value.back()))
        value.pop_back();
}

inline
void ConfigParser::skipSpaces(void)
{
    while(isspace(*reader))
        ++ reader;
}

#endif // __CONFIGPARSER_H__
