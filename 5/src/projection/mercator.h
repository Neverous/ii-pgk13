/*
 * Elliptical Mercator
 * http://wiki.openstreetmap.org/wiki/Mercator#Elliptical_Mercator
 */
#ifndef __MERCATOR_H__
#define __MERCATOR_H__

#include <cmath>

namespace terrain
{

namespace projection
{

namespace mercator
{

static const double EQUATORIAL_RADIUS    = 6378137.0;
static const double POLAR_RADIUS         = 6356752.3142;
static const double RADIUS_RATIO         = POLAR_RADIUS / EQUATORIAL_RADIUS;
static const double ECCENT               = sqrt(1.0 - RADIUS_RATIO * RADIUS_RATIO);
static const double COM                  = 0.5 * ECCENT;

inline
double lonToMet(double lon)
{
    return EQUATORIAL_RADIUS * lon * M_PI / 180.0;
}

inline
double latToMet(double lat)
{
    lat = fmin(89.5, fmax(lat, -89.5));
    double phi       = lat * M_PI / 180.0;
    double con       = ECCENT * sin(phi);
    con = pow((1.0 - con) / (1.0 + con), COM);
    return 0.0 - EQUATORIAL_RADIUS * log(tan(0.5 * (M_PI * 0.5 - phi)) / con);
}

inline
double metToLon(double x)
{
    return 180.0 * x / EQUATORIAL_RADIUS / M_PI;
}

inline
double metToLat(double y)
{
    double ts    = exp(-y / EQUATORIAL_RADIUS);
    double phi   = M_PI_2 - 2.0 * atan(ts);
    double dphi  = 1.0;
    for(int i = 0; fabs(dphi) > 0.000000001 && i < 15; ++ i)
    {
        double con = ECCENT * sin(phi);
        dphi = M_PI_2 - 2.0 * atan(ts * pow((1.0 - con) / (1.0 + con), COM)) - phi;
        phi += dphi;
    }

    return 180.0 * phi / M_PI;
}

} // namespace mercator

} // namespace projection

} // namespace terrain

#endif // __MERCATOR_H__
