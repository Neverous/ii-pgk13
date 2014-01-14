#ifndef __GLM_VECTOR_ANGLE_H__
#define __GLM_VECTOR_ANGLE_H__

#include <glm/glm.hpp>
#include <glm/gtx/vector_angle.hpp>

inline
double orientedAngle(const glm::dvec2 &x, const glm::dvec2 &y)
{
    double angle = min(360.0, max(0.0, (double) glm::degrees(acos(glm::dot(x, y)))));
    if(glm::cross(glm::dvec3(x, 0.0), glm::dvec3(y, 0.0)).z < 0)
        return -angle;

    return angle;
}

#endif // __GLM_VECTOR_ANGLE_H__
