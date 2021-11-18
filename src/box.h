#ifndef BOX_H
#define BOX_H

#include <memory>
#include "hittable.h"
#include "hittable_list.h"
#include "material.h"
#include "vec3.h"

struct box : public hittable {
    point min;
    point max;
    hittable_list sides;
    std::shared_ptr<material> mat_ptr;

    void make_sides() {
        sides.make<xy_rect>(min.x, max.x, min.y, max.y, max.z, mat_ptr);
        sides.make<xy_rect>(min.x, max.x, min.y, max.y, min.z, mat_ptr);
        sides.make<xz_rect>(min.x, max.x, min.z, max.z, max.y, mat_ptr);
        sides.make<xz_rect>(min.x, max.x, min.z, max.z, min.y, mat_ptr);
        sides.make<yz_rect>(min.y, max.y, min.z, max.z, max.x, mat_ptr);
        sides.make<yz_rect>(min.y, max.y, min.z, max.z, min.x, mat_ptr);
    }

    box(point const& min, point const& max, std::shared_ptr<material> mat_ptr) : min(min), max(max), mat_ptr(mat_ptr) {
        make_sides();
    }

    box(json const& j) {
        j.at("min").get_to(min);
        j.at("max").get_to(max);
        mat_ptr = material::make_from_json(j.at("material"));
        make_sides();
    }

    virtual bool hit(ray const& r, double t_min, double t_max, hit_record& h) const override {
        return sides.hit(r, t_min, t_max, h);
    }

    virtual std::tuple<bool, aabb> bounding_box(double t0, double t1) const override {
        return {true, aabb(min, max)};
    }

    json to_json() const override {
        return json{
            {"type", "box"},
            {"min", min},
            {"max", max},
            {"material", *mat_ptr}
        };
    }

    bool equals(hittable const& rhs) const override {
        if (typeid(*this) != typeid(rhs)) return false;
        auto that = static_cast<box const&>(rhs);
        return min == that.min && max == that.max && *mat_ptr == *that.mat_ptr;
    }

    void print(std::ostream& os) const override {
        os << "box: {min: " << min << ", max: " << max << ", material: " << *mat_ptr << "}";
    }
};

#endif