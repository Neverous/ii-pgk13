#ifndef __OBJECTS_H__
#define __OBJECTS_H__

#include "defines.h"

#include <GL/glew.h>

namespace terrain
{

namespace objects
{

struct Position
{
    GLfloat x;
    GLfloat y;
    GLfloat z;

    Position(float _x = 0.0f, float _y = 0.0f, float _z = 0.0f);
    operator GLfloat *(void);
}; // struct Position

struct Color
{
    GLfloat R;
    GLfloat G;
    GLfloat B;

    Color(float _R = 0.0f, float _G = 0.0f, float _B = 0.0f);
    operator GLfloat *(void);
}; // struct Color

struct Point: public Position, public Color
{
    Point(float _x = 0.0f, float _y = 0.0f, float _z = 0.0f, float _R = 0.0f, float _G = 0.0f, float _B = 0.0f);

    Point &operator=(const Position& position);
    Point &operator=(const Color& color);
    operator GLfloat *(void);
}; // struct Point

struct TerrainPoint
{
    uint32_t    x       :TILE_DENSITY_BITS;
    uint32_t    y       :TILE_DENSITY_BITS;
    uint32_t    height  :14;

    TerrainPoint(uint32_t _x = 0, uint32_t _y = 0, uint32_t _height = 0);
}; // struct TerrainPoint

struct Tile
{
    union ID
    {
        uint64_t d;
        struct
        {
            uint32_t h;
            uint32_t w;
        };
    } id;

    enum Status
    {
        SYNCHRONIZED    = 0,
        DESYNCHRONIZED  = 1,
        SCALED          = 2,
    } synchronized;

    struct BoundingBox
    {
        float   top;
        float   left;
        float   bottom;
        float   right;
    } box;

    GLuint  buffer;
    float   zoom;

    Tile(uint64_t _id = 0, Status _synchronized = DESYNCHRONIZED, BoundingBox _box = {0, 0, 0, 0}, uint32_t _buffer = 0, float _zoom = 1.0f);
}; // struct Tile

inline
Position::Position(float _x/* = 0.0f*/, float _y/* = 0.0f*/, float _z/* = 0.0f*/)
:x(_x)
,y(_y)
,z(_z)
{
}

inline
Position::operator GLfloat *(void)
{
    return (GLfloat *) this;
}

inline
Color::Color(float _R/* = 0.0f*/, float _G/* = 0.0f*/, float _B/* = 0.0f*/)
:R(_R)
,G(_G)
,B(_B)
{
}

inline
Color::operator GLfloat *(void)
{
    return (GLfloat *) this;
}

inline
Point::Point(float _x/* = 0.0f*/, float _y/* = 0.0f*/, float _z/* = 0.0f*/, float _R/* = 0.0f*/, float _G/* = 0.0f*/, float _B/* = 0.0f*/)
:Position(_x, _y, _z)
,Color(_R, _G, _B)
{
}

inline
Point &Point::operator=(const Position& position)
{
    x = position.x;
    y = position.y;
    z = position.z;

    return *this;
}

inline
Point &Point::operator=(const Color& color)
{
    R = color.R;
    G = color.G;
    B = color.B;

    return *this;
}

inline
Point::operator GLfloat *(void)
{
    return (GLfloat *) this;
}

inline
TerrainPoint::TerrainPoint(uint32_t _x/* = 0*/, uint32_t _y/* = 0*/, uint32_t _height/* = 0*/)
:x(_x)
,y(_y)
,height(_height)
{
}

inline
Tile::Tile(uint64_t _id/* = 0*/, Status _synchronized/* = DESYNCHORNIZED*/, BoundingBox _box/* = {0, 0, 0, 0}*/, uint32_t _buffer/* = 0*/, float _zoom/* = 1.0f*/)
:synchronized(_synchronized)
,box(_box)
,buffer(_buffer)
,zoom(_zoom)
{
    id.d = _id;
}

} // namespace objects

} // namespace terrain

#endif // __OBJECTS_H__
