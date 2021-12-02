#ifndef RAY_H
#define RAY_H

#include "vec3.h"

struct ray {
    point origin;
    vec3 direction;
    double time;

    ray(point const& origin, vec3 const& direction, double time) : origin(origin), direction(direction), time(time) { }
    ray(point const& origin, vec3 const& direction) : ray(origin, direction, 0) { }
    ray() : ray(point(0, 0, 0), vec3(0, 0, 0)) { }

    point at(double t) const {
        return origin + t * direction;
    }
};

#endif