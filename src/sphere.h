#ifndef SPHERE_H
#define SPHERE_H

#include <memory>
#include "hittable.h"
#include "vec3.h"
#include "ray.h"

struct sphere : public hittable {
    point center;
    double radius;
    std::shared_ptr<material> mat_ptr;

    sphere(point const& center, double radius, std::shared_ptr<material> mat_ptr) 
            : center(center), radius(radius), mat_ptr(mat_ptr) {}
    sphere(point const& center, double radius) : sphere(center, radius, nullptr) {}

    virtual bool hit(ray const& r, double t_min, double t_max, hit_record& rec) const override {
        vec3 oc = r.origin - center;
        auto a = r.direction.length_squared();
        auto half_b = dot(oc, r.direction);
        auto c = oc.length_squared() - radius * radius;

        auto d = half_b * half_b - a * c;
        if (d < 0) return false;

        auto sqrt_d = std::sqrt(d);
        auto root = (-half_b - sqrt_d) / a;
        if (root < t_min || root > t_max) {
            root = (-half_b + sqrt_d) / a;
            if (root < t_min || root > t_max) {
                return false;
            }
        }

        rec.t = root;
        rec.p = r.at(rec.t);
        rec.set_face_normal(r, (rec.p - center) / radius);
        rec.mat_ptr = mat_ptr;
        return true;
    }
};

#endif