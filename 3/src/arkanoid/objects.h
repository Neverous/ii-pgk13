#ifndef __OBJECTS_H__
#define __OBJECTS_H__

#include <cmath>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

namespace arkanoid
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

struct Position
{
    GLfloat x;
    GLfloat y;
    GLfloat z;

    operator const GLfloat *(void);

    Position &operator+=(const Position& position);
    float len(void);
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

struct Box
{
    GLfloat left;
    GLfloat top;
    GLfloat right;
    GLfloat bottom;

    Box &operator+=(const Position &position);
    Collide collide(const Box &collision);
}; // struct Box

struct Point: public Position, public Color
{
}; // struct Point

struct Acceleration: public Position
{
}; // struct Acceleration

struct Velocity: public Position
{
    Velocity &operator+=(const Acceleration &acceleration);
}; // struct Velocity

struct Object
{
    bool            alive;
    Box             box;
    Position        position;
    Velocity        velocity;
    Acceleration    acceleration;

    Point           *local;
    GLuint          points;
    GLuint          index;
}; // struct Object

inline
Position::operator const GLfloat *(void)
{
    return &x;
}

inline
Position &Position::operator+=(const Position& position)
{
    x += position.x;
    y += position.y;
    z += position.z;

    return *this;
}

inline
float Position::len(void)
{
    return sqrtf(x*x + y*y + z*z);
}


inline
Position &Position::normalize(const float &mx)
{
    float length = len();
    if(length < mx)
        return *this;

    x *= mx / length;
    y *= mx / length;
    z *= mx / length;

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

inline
Box &Box::operator+=(const Position &position)
{
    left    += position.x;
    right   += position.x;
    top     += position.y;
    bottom  += position.y;

    return *this;
}

inline
Collide Box::collide(const Box &collision)
{
    if( left    > collision.right
    ||  right   < collision.left
    ||  top     < collision.bottom
    ||  bottom  > collision.top)
        return COLLIDE_NONE;

    int result = COLLIDE_NONE;
    if(right >= collision.left && left < collision.left)
        result |= COLLIDE_LEFT;

    if(left <= collision.right && right > collision.right)
        result |= COLLIDE_RIGHT;

    if(bottom <= collision.top && top > collision.top)
        result |= COLLIDE_TOP;

    if(top >= collision.bottom && bottom < collision.bottom)
        result |= COLLIDE_BOTTOM;

    if( ((result & COLLIDE_LEFT) && (result & COLLIDE_RIGHT))
    ||  ((result & COLLIDE_TOP) && (result & COLLIDE_BOTTOM)))
         return COLLIDE_INVALID;

    return (Collide) result;
}

inline
Velocity &Velocity::operator+=(const Acceleration &acceleration)
{
    x += acceleration.x;
    y += acceleration.y;
    z += acceleration.z;

    return *this;
}

}; // namespace arkanoid

#endif // __OBJECTS_H__
