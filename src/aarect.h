#ifndef AARECT_H
#define AARECT_H

#include <memory>
#include "hittable.h"
#include "material.h"

struct xy_rect : public hittable {
    std::shared_ptr<material> mat_ptr;
    double x0, x1, y0, y1, k;

    xy_rect(double x0, double x1, double y0, double y1, double k, std::shared_ptr<material> mat_ptr) :
            x0(x0), x1(x1), y0(y0), y1(y1), k(k), mat_ptr(mat_ptr) { }
    xy_rect() : xy_rect(0, 0, 0, 0, 0, nullptr) { }

    virtual bool hit(ray const& r, double t_min, double t_max, hit_record& h) const override {
        auto t = (k-r.origin.z) / r.direction.z;
        if (t < t_min || t > t_max)
            return false;
        auto x = r.origin.x + t*r.direction.x;
        auto y = r.origin.y + t*r.direction.y;
        if (x < x0 || x > x1 || y < y0 || y > y1)
            return false;
        h.u = (x-x0)/(x1-x0);
        h.v = (y-y0)/(y1-y0);
        h.t = t;
        auto outward_normal = vec3(0, 0, 1);
        h.set_face_normal(r, outward_normal);
        h.mat_ptr = mat_ptr;
        h.p = r.at(t);
        return true;
    }

    virtual std::tuple<bool, aabb> bounding_box(double t0, double t1) const override {
        return {true, aabb(point(x0, y0, k - 0.0001), point(x1, y1, k + 0.0001))};
    }
};

class xz_rect : public hittable {
    public:
        xz_rect() {}

        xz_rect(double _x0, double _x1, double _z0, double _z1, double _k,
            std::shared_ptr<material> mat)
            : x0(_x0), x1(_x1), z0(_z0), z1(_z1), k(_k), mp(mat) {};
        xz_rect(json const& j) {
            j.at("x0").get_to(x0);
            j.at("x1").get_to(x1);
            j.at("z0").get_to(z0);
            j.at("z1").get_to(z1);
            j.at("k").get_to(k);
            mp = material::make_from_json(j.at("material"));
        }

        virtual json to_json() const override {
            return json{
                {"type", "xz_rect"},
                {"x0", x0},
                {"x1", x1},
                {"z0", z0},
                {"z1", z1},
                {"k", k},
                {"material", *mp}
            };
        }

        virtual bool hit(const ray& r, double t_min, double t_max, hit_record& rec) const override;

        virtual std::tuple<bool, aabb> bounding_box(double time0, double time1) const override {
            // The bounding box must have non-zero width in each dimension, so pad the Y
            // dimension a small amount.
            auto output_box = aabb(point(x0,k-0.0001,z0), point(x1, k+0.0001, z1));
            return {true, output_box};
        }

        virtual bool equals(hittable const& rhs) const override {
            if (typeid(*this) != typeid(rhs)) return false;
            auto that = static_cast<decltype(*this)>(rhs);
            return x0 == that.x0 && x1 == that.x1 && z0 == that.z0 && z1 == that.z1 && k == that.k;
        }

    public:
        std::shared_ptr<material> mp;
        double x0, x1, z0, z1, k;
};

class yz_rect : public hittable {
    public:
        yz_rect() {}

        yz_rect(double _y0, double _y1, double _z0, double _z1, double _k,
            std::shared_ptr<material> mat)
            : y0(_y0), y1(_y1), z0(_z0), z1(_z1), k(_k), mp(mat) {};

        virtual bool hit(const ray& r, double t_min, double t_max, hit_record& rec) const override;

        virtual std::tuple<bool, aabb> bounding_box(double time0, double time1) const override {
            // The bounding box must have non-zero width in each dimension, so pad the X
            // dimension a small amount.
            auto output_box = aabb(point(k-0.0001, y0, z0), point(k+0.0001, y1, z1));
            return {true, output_box};
        }

    public:
        std::shared_ptr<material> mp;
        double y0, y1, z0, z1, k;
};
bool xz_rect::hit(const ray& r, double t_min, double t_max, hit_record& rec) const {
    auto t = (k-r.origin.y) / r.direction.y;
    if (t < t_min || t > t_max)
        return false;
    auto x = r.origin.x + t*r.direction.x;
    auto z = r.origin.z + t*r.direction.z;
    if (x < x0 || x > x1 || z < z0 || z > z1)
        return false;
    rec.u = (x-x0)/(x1-x0);
    rec.v = (z-z0)/(z1-z0);
    rec.t = t;
    auto outward_normal = vec3(0, 1, 0);
    rec.set_face_normal(r, outward_normal);
    rec.mat_ptr = mp;
    rec.p = r.at(t);
    return true;
}

bool yz_rect::hit(const ray& r, double t_min, double t_max, hit_record& rec) const {
    auto t = (k-r.origin.x) / r.direction.x;
    if (t < t_min || t > t_max)
        return false;
    auto y = r.origin.y + t*r.direction.y;
    auto z = r.origin.z + t*r.direction.z;
    if (y < y0 || y > y1 || z < z0 || z > z1)
        return false;
    rec.u = (y-y0)/(y1-y0);
    rec.v = (z-z0)/(z1-z0);
    rec.t = t;
    auto outward_normal = vec3(1, 0, 0);
    rec.set_face_normal(r, outward_normal);
    rec.mat_ptr = mp;
    rec.p = r.at(t);
    return true;
}

#endif