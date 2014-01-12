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
    uint8_t data[1201][1201][2];

    public:
        File(const char *filename);
        int32_t get(int x, int y);
}; // class File

inline
File::File(const char *filename)
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
int32_t File::get(int x, int y)
{
    assert(0 <= x && x <= 1200 && 0 <= y && y <= 1200);
    return (int32_t) ((int32_t) data[y][x][1] << 16) + (int32_t) data[y][x][0];
}

} // namespace hgt

} // namespace terrain

#endif // __HGT_FILE_H__
