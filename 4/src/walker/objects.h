#ifndef __OBJECTS_H__
#define __OBJECTS_H__

#include <cmath>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

namespace walker
{

enum Collide
{
    COLLIDE_NONE        = 0,
    COLLIDE_TOP         = 1,
    COLLIDE_BOTTOM      = 2,
    COLLIDE_LEFT        = 4,
    COLLIDE_RIGHT       = 8,
    COLLIDE_TOPLEFT     = 5,
    COLLIDE_TOPRIGHT    = 9,
    COLLIDE_BOTTOMLEFT  = 6,
    COLLIDE_BOTTOMRIGHT = 10,
    COLLIDE_INVALID     = 16,
}; // enum Collide

struct Position: glm::vec3
{
    Position(void) :glm::vec3() {}
    Position(const glm::vec3 &src);
    operator const GLfloat *(void);

    Position &normalize(const float &max);
    void clear(void);
}; // struct Position

struct Color
{
    GLfloat R;
    GLfloat G;
    GLfloat B;

    operator const GLfloat *(void);
}; // struct Color

struct Point: public Position, public Color
{
}; // struct Point

struct Object
{
    Position        position;

    Point           *local;
    GLuint          points;
    GLuint          index;
}; // struct Object


struct Figure: public Object
{
    Position        bone[12];
    int             var;
}; // struct Figure

inline
Position::Position(const glm::vec3 &src)
{
    x = src.x;
    y = src.y;
    z = src.z;
}

inline
Position::operator const GLfloat *(void)
{
    return &x;
}

inline
Position &Position::normalize(const float &mx)
{
    float _len = length();
    x *= mx / _len;
    y *= mx / _len;
    z *= mx / _len;

    return *this;
}

inline
void Position::clear(void)
{
    x = 0;
    y = 0;
    z = 0;
}

inline
Color::operator const GLfloat *(void)
{
    return &R;
}

}; // namespace walker

#endif // __OBJECTS_H__
