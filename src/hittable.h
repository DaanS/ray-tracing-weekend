#ifndef HITTABLE_H
#define HITTABLE_H

#include "aabb.h"
#include "ray.h"
#include "vec3.h"

struct material;

struct hit_record {
    point p;
    vec3 n;
    std::shared_ptr<material> mat_ptr;
    double t;
    double u;
    double v;
    bool front_face;

    void set_face_normal(ray const& r, vec3 const& n_out) {
        front_face = dot(r.direction, n_out) < 0;
        n = front_face ? n_out : -n_out;
    }
};

struct hittable {
    virtual bool hit(ray const& r, double t_min, double t_max, hit_record& rec) const = 0;
    virtual std::tuple<bool, aabb> bounding_box(double t0, double t1) const = 0;

    virtual std::string name() const {
        return "";
    }

    bool operator==(hittable const& rhs) const { return this->equals(rhs); }
    virtual bool equals(hittable const& rhs) const { throw std::logic_error("equals not implemented for this hittable type"); }
};

#endif