#ifndef __HGT_FILE_H__
#define __HGT_FILE_H__

#include <cstdlib>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <cstdint>
#include <cassert>

namespace terrain
{

namespace hgt
{

class File
{
    int8_t data[1201][1201][2];

    public:
        File(const char *filename);
        int16_t get(int x, int y);
}; // class File

inline
File::File(const char *filename)
:data()
{
    int fd = open(filename, O_RDONLY);
    if(fd == -1)
        throw runtime_error("Couldn't open map file!");

    if(read(fd, this, sizeof(File)) != sizeof(File))
    {
        close(fd);
        throw runtime_error("Couldn't read map data!");
    }

    close(fd);
}

inline
int16_t File::get(int x, int y)
{
    y = 1200 - y;
    assert(0 <= x && x <= 1200 && 0 <= y && y <= 1200);
    union
    {
        int16_t word;
        struct
        {
            int8_t byte1;
            int8_t byte2;
        };
    } conv;

    conv.byte1 = data[y][x][1];
    conv.byte2 = data[y][x][0];
    if(conv.word == -32768)
        return -500;

    assert(-500 <= conv.word && conv.word <= 9000);
    return conv.word;
}

} // namespace hgt

} // namespace terrain

#endif // __HGT_FILE_H__
