#ifndef __CHUNKFILEREADER_H__
#define __CHUNKFILEREADER_H__

#include <cstdio>
#include <string>

#ifndef FILE_BUFFER_SIZE
    #define FILE_BUFFER_SIZE    1048576
#endif // FILE_BUFFER_SIZE

#ifndef FILE_TRACK_POSITION
    #define FILE_TRACK_POSITION true
#endif // FILE_TRACK_POSITION

class ChunkFileReader
{
    public:
        std::string filename;
#if FILE_TRACK_POSITION
        int line,
            column;
#endif // FILE_TRACK_POSITION

        ChunkFileReader(const char *_filename);
        ~ChunkFileReader(void);

        void operator++(void);
        char operator*(void);

    private:
        FILE *file;
        char buffer[FILE_BUFFER_SIZE];
        int pos;

        void readChunk(void);
}; // class ChunkFileReader

#include "chunkFileReader.inl"

#endif // __CHUNKFILEREADER_H__
