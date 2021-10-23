#ifndef CAMERA_H
#define CAMERA_H

#include "ray.h"
#include "vec3.h"

struct camera {
    point origin;
    point lower_left;
    vec3 hor;
    vec3 ver;
    double aspect_ratio;

    camera() {
        aspect_ratio = 16.0 / 9.0;
        auto view_h = 2.0;
        auto view_w = aspect_ratio * view_h;
        auto focal_length = 1.0;

        origin = point(0, 0, 0);
        hor = vec3(view_w, 0, 0);
        ver = vec3(0, view_h, 0);
        lower_left = origin - hor / 2 - ver / 2 - vec3(0, 0, focal_length);
    }

    ray get_ray(double u, double v) const {
        return ray(origin, lower_left + u * hor + v * ver - origin);
    }
};

#endif