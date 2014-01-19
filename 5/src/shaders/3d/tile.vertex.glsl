#version 120
#extension GL_EXT_gpu_shader4: enable

uniform vec4 box;
uniform mat4 MVP;

attribute float height;

varying vec4 fragmentColor;

/* PROJECTION */
#define M_PI    3.14159265358979323846
#define M_PI_2  1.57079632679489661923

const float EQUATORIAL_RADIUS    = 6378137.0;
const float POLAR_RADIUS         = 6356752.3142;
const float RADIUS_RATIO         = POLAR_RADIUS / EQUATORIAL_RADIUS;
const float ECCENT               = sqrt(1.0 - RADIUS_RATIO * RADIUS_RATIO);
const float COM                  = 0.5 * ECCENT;

float metToLon(float x)
{
    return 180.0 * x / EQUATORIAL_RADIUS / M_PI;
}

float metToLat(float y)
{
    float ts    = exp(-y / EQUATORIAL_RADIUS);
    float phi   = M_PI_2 - 2.0 * atan(ts);
    float dphi  = 1.0;
    for(int i = 0; abs(dphi) > 0.000000001 && i < 15; ++ i)
    {
        float con = ECCENT * sin(phi);
        dphi = M_PI_2 - 2.0 * atan(ts * pow((1.0 - con) / (1.0 + con), COM)) - phi;
        phi += dphi;
    }

    return 180.0 * phi / M_PI;
}

void main()
{
    int y = gl_VertexID / 1025;
    int x = gl_VertexID - y * 1025;
    vec4 vertex = vec4(x, y, height, 1);
    vertex.x = box.x + (box.y - box.x) * vertex.x / 1024.0;
    vertex.y = box.z + (box.w - box.z) * vertex.y / 1024.0;
    if(vertex.z == 32768.0)
        vertex.z = 0;

    float altitude = vertex.z - 1000.0;
    vec2 lonlat = vec2(metToLon(vertex.x) * M_PI / 180.0, metToLat(vertex.y) * M_PI / 180.0);
    vertex.x = (EQUATORIAL_RADIUS + altitude) * cos(lonlat.y) * cos(lonlat.x);
    vertex.y = (EQUATORIAL_RADIUS + altitude) * cos(lonlat.y) * sin(lonlat.x);
    vertex.z = (EQUATORIAL_RADIUS + altitude) * sin(lonlat.y);
    gl_Position = MVP * vertex;

    float ht = height - 1000.0;
    if(ht < 0.0)
        fragmentColor = vec4(0.0, 0.0, 1.0, 1.0);

    else if(ht < 500.0)
        fragmentColor = vec4(0.0, ht / 500.0, 0.0, 1.0);

    else if(ht < 1000.0)
        fragmentColor = vec4(ht / 500.0 - 1.0, 1.0, 0.0, 1.0);

    else if(ht < 1500.0)
        fragmentColor = vec4(1.0, 2.0 - ht / 500.0, 0.0, 1.0);

    else if(ht < 10000.0)
        fragmentColor = vec4(1.0, 1.0, 1.0, 1.0);

    else
        fragmentColor = vec4(0.0, 0.0, 0.0, 0.0);
}
