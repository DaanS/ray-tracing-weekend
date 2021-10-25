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
    vec3 u, v, w;
    double lens_radius;

    camera(point from, point to, vec3 up, double vfov, double aspect_ratio, double aperture, double focus_dist) {
        auto theta = radians(vfov);
        auto h = tan(theta / 2);
        auto view_h = 2 * h;
        auto view_w = aspect_ratio * view_h;

        w = normalize(from - to);
        u = normalize(cross(up, w));
        v = cross(w, u);

        origin = from;
        hor = focus_dist * view_w * u;
        ver = focus_dist * view_h * v;
        lower_left = origin - hor / 2 - ver / 2 - focus_dist * w;
        lens_radius = aperture / 2;
    }

    camera() : camera(point(0, 0, 0), point(0, 0, -1), vec3(0, 1, 0), 90, 16.0 / 9.0, 0, 1) {}

    ray get_ray(double s, double t) const {
        //return ray(origin, lower_left + u * hor + v * ver - origin);
        auto rd = lens_radius * vec3_random_disk();
        auto offset = u * rd.x + v * rd.y;
        return ray(origin + offset, lower_left + s * hor + t * ver - origin - offset);
    }
};

#endif