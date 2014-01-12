#ifndef __GLM_VECTOR_ANGLE_H__
#define __GLM_VECTOR_ANGLE_H__

#include <glm/glm.hpp>
#include <glm/gtx/vector_angle.hpp>

inline
float orientedAngle(const glm::vec2 &x, const glm::vec2 &y)
{
    float angle = min(360.0f, max(0.0f, glm::degrees(acosf(glm::dot(x, y)))));
    if(glm::cross(glm::vec3(x, 0.0f), glm::vec3(y, 0.0f)).z < 0)
        return -angle;

    return angle;
}

#endif // __GLM_VECTOR_ANGLE_H__
