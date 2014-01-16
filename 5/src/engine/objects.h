#ifndef __OBJECTS_H__
#define __OBJECTS_H__

#include "defines.h"

#include <GL/glew.h>
#include <glm/glm.hpp>

namespace terrain
{

namespace objects
{

struct Position
{
    GLfloat x;
    GLfloat y;
    GLfloat z;

    Position(double _x = 0.0, double _y = 0.0, double _z = 0.0);
    operator GLfloat *(void);
}; // struct Position

struct Color
{
    GLfloat R;
    GLfloat G;
    GLfloat B;

    Color(double _R = 0.0, double _G = 0.0, double _B = 0.0);
    operator GLfloat *(void);
}; // struct Color

struct Point: public Position, public Color
{
    Point(double _x = 0.0, double _y = 0.0, double _z = 0.0, double _R = 0.0, double _G = 0.0, double _B = 0.0);

    Point &operator=(const Position& position);
    Point &operator=(const Color& color);
    operator GLfloat *(void);
}; // struct Point

struct TerrainPoint
{
    uint16_t    x;
    uint16_t    y;
    uint16_t    height;

    TerrainPoint(uint16_t _x = 0, uint16_t _y = 0, uint16_t _height = 0);
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
    }           id;

    bool        valid;
    glm::vec4   box;
    GLuint      buffer;
    uint32_t    size;
    uint8_t     order;

    Tile(uint64_t _id = 0, bool _valid = false, glm::dvec4 _box = glm::dvec4(), uint32_t _buffer = 0, uint32_t _size = 0, uint8_t _order = 0);
}; // struct Tile

inline
Position::Position(double _x/* = 0.0*/, double _y/* = 0.0*/, double _z/* = 0.0*/)
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
Color::Color(double _R/* = 0.0*/, double _G/* = 0.0*/, double _B/* = 0.0*/)
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
Point::Point(double _x/* = 0.0*/, double _y/* = 0.0*/, double _z/* = 0.0*/, double _R/* = 0.0*/, double _G/* = 0.0*/, double _B/* = 0.0*/)
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
TerrainPoint::TerrainPoint(uint16_t _x/* = 0*/, uint16_t _y/* = 0*/, uint16_t _height/* = 0*/)
:x(_x)
,y(_y)
,height(_height)
{
}

inline
Tile::Tile(uint64_t _id/*= 0*/, bool _valid/* = false*/, glm::dvec4 _box/* = glm::dvec4()*/, uint32_t _buffer/* = 0*/, uint32_t _size/* = 0*/, uint8_t _order/* = 0*/)
:valid(_valid)
,box(_box)
,buffer(_buffer)
,size(_size)
,order(_order)
{
    id.d = _id;
}

} // namespace objects

} // namespace terrain

#endif // __OBJECTS_H__
