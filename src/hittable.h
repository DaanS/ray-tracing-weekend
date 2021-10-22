#ifndef HITTABLE_H
#define HITTABLE_H

#include "ray.h"
#include "vec3.h"

struct hit_record {
    point p;
    vec3 n;
    double t;
    bool front_face;

    void set_face_normal(ray const& r, vec3 const& n_out) {
        front_face = dot(r.direction, n_out) < 0;
        n = front_face ? n_out : -n_out;
    }
};

struct hittable {
    virtual bool hit(ray const& r, double t_min, double t_max, hit_record& rec) const = 0;
};

#endif