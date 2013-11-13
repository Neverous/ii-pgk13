#ifndef __CHUNKFILEREADER_INL__
#define __CHUNKFILEREADER_INL__

#include <cstdio>
#include <cstdlib>
#include <libgen.h>
#include <climits>
#include <cassert>
#include <cstring>
#include <string>
#include <stdexcept>
#include <cerrno>
#include "libs/utils.h"

#include "chunkFileReader.h"

#define _inline inline

// CHUNKFILEREADER
_inline
ChunkFileReader::ChunkFileReader(const char *_filename)
:filename(_filename)
#if FILE_TRACK_POSITION
,line(1)
,column(1)
#endif // FILE_TRACK_POSITION
,file(nullptr)
,buffer()
,pos(0)
{
    char __filename[PATH_MAX + 1] = {};
    if(!realpath(_filename, __filename))
        throwError("%s: %s", _filename, strerror(errno));

    file = fopen(__filename, "rb");
    if(!file)
        throwError("%s: %s", __filename, strerror(errno));

    readChunk();
}

_inline
ChunkFileReader::~ChunkFileReader(void)
{
    assert(file);
    fclose(file);
}

_inline
void ChunkFileReader::operator++(void)
{
    assert(buffer[pos] != EOF);

#if FILE_TRACK_POSITION
    if(buffer[pos] == '\n')
    {
        column = 0;
        ++ line;
    }

    ++ column;
    ++ pos;
#endif // FILE_TRACK_POSITION
    if(!buffer[pos])
    {
        pos = 0;
        readChunk();
    }
}

_inline
char ChunkFileReader::operator*(void)
{
    return buffer[pos];
}

inline
void ChunkFileReader::readChunk(void)
{
    assert(file);
    int read = fread(buffer, 1, FILE_BUFFER_SIZE - 1, file);
    if(read <= 0)
        buffer[0] = EOF;

    else
        buffer[read] = 0;
}

#undef _inline
#endif // __CHUNKFILEREADER_H__
