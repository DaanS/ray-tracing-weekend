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

    virtual std::tuple<bool, aabb> bounding_box(double t0, double t1) const override {
        return {true, aabb(center - vec3(radius, radius, radius), center + vec3(radius, radius, radius))};
    }

    bool operator==(sphere const& rhs) const {
        return center == rhs.center && radius == rhs.radius && mat_ptr == rhs.mat_ptr;
    }
};

struct moving_sphere : public hittable {
    point center_start, center_end;
    double time_start, time_end;
    double radius;
    std::shared_ptr<material> mat_ptr;

    moving_sphere(point const& center_start, point const& center_end, double time_start, double time_end, double radius, std::shared_ptr<material> mat_ptr)
            : center_start(center_start), center_end(center_end), time_start(time_start), time_end(time_end), radius(radius), mat_ptr(mat_ptr) { }
    moving_sphere(point const& center_start, point const& center_end, double time_start, double time_end, double radius)
            : moving_sphere(center_start, center_end, time_start, time_end, radius, nullptr) { }

    virtual bool hit(ray const& r, double t_min, double t_max, hit_record& rec) const override {
        vec3 oc = r.origin - center(r.time);
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
        rec.set_face_normal(r, (rec.p - center(r.time)) / radius);
        rec.mat_ptr = mat_ptr;
        return true;
    }

    virtual std::tuple<bool, aabb> bounding_box(double t0, double t1) const override {
        auto [res1, start] = sphere(center(t0), radius).bounding_box(t0, t1);
        auto [res2, end] = sphere(center(t1), radius).bounding_box(t0, t1);
        return {true, surrounding_box(start, end)};
    }

    point center(double time) const {
        return center_start + ((time - time_start) / (time_end - time_start)) * (center_end - center_start);
    }
};

#endif