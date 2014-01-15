#ifndef __HGT_MAP_H__
#define __HGT_MAP_H__

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

class Map
{
    int16_t data[1201][1201];

    public:
        Map(void);
        int16_t &get(int x, int y);
        void set(int x, int y, int16_t value);
}; // class Map

inline
Map::Map(void)
:data()
{
}

inline
int16_t &Map::get(int x, int y)
{
    assert(0 <= x && x <= 1200 && 0 <= y && y <= 1200);
    return data[y][x];
}

inline
void Map::set(int x, int y, int16_t value)
{
    assert(0 <= x && x <= 1200 && 0 <= y && y <= 1200);
    data[y][x] = value;
}

} // namespace hgt

} // namespace terrain

#endif // __HGT_MAP_H__
