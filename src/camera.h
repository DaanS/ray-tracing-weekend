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
    double start_time;
    double end_time;

    camera(point from, point to, vec3 up, double vfov, double aspect_ratio, double aperture, double focus_dist, double start_time, double end_time) 
            : aspect_ratio(aspect_ratio), start_time(start_time), end_time(end_time) {
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

    camera() : camera(point(0, 0, 0), point(0, 0, -1), vec3(0, 1, 0), 90, 16.0 / 9.0, 0, 1, 0, 0) {}

    ray get_ray(double s, double t) const {
        //return ray(origin, lower_left + u * hor + v * ver - origin);
        auto rd = lens_radius * vec3_random_disk();
        auto offset = u * rd.x + v * rd.y;
        return ray(origin + offset, lower_left + s * hor + t * ver - origin - offset, random_double(start_time, end_time));
    }

    bool operator==(camera const& rhs) const {
        return origin == rhs.origin &&
                lower_left == rhs.lower_left &&
                hor == rhs.hor &&
                ver == rhs.ver &&
                aspect_ratio == rhs.aspect_ratio &&
                u == rhs.u &&
                v == rhs.v &&
                w == rhs.w &&
                lens_radius == rhs.lens_radius &&
                start_time == rhs.start_time &&
                end_time == rhs.end_time;
    }
};

void to_json(json& j, camera const& cam) {
    j = json{
        {"origin", cam.origin},
        {"lower_left", cam.lower_left},
        {"hor", cam.hor},
        {"ver", cam.ver},
        {"aspect_ratio", cam.aspect_ratio},
        {"u", cam.u},
        {"v", cam.v},
        {"w", cam.w},
        {"lens_radius", cam.lens_radius},
        {"start_time", cam.start_time},
        {"end_time", cam.end_time}
    };
}

void from_json(json const&j, camera& cam) {
    j.at("origin").get_to(cam.origin);
    j.at("lower_left").get_to(cam.lower_left);
    j.at("hor").get_to(cam.hor);
    j.at("ver").get_to(cam.ver);
    j.at("aspect_ratio").get_to(cam.aspect_ratio);
    j.at("u").get_to(cam.u);
    j.at("v").get_to(cam.v);
    j.at("w").get_to(cam.w);
    j.at("lens_radius").get_to(cam.lens_radius);
    j.at("start_time").get_to(cam.start_time);
    j.at("end_time").get_to(cam.end_time);
}

#endif