#ifndef RAY_H
#define RAY_H

#include "vec3.h"

struct ray {
    point origin;
    vec3 direction;

    ray(point const& origin, vec3 const& direction) : origin(origin), direction(direction) {}

    point at(double t) const {
        return origin + t * direction;
    }
};

#endif