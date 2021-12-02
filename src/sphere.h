#ifndef SPHERE_H
#define SPHERE_H

#include <nlohmann/json.hpp>
#include <memory>
#include <numbers>
#include "material.h"
#include "hittable.h"
#include "vec3.h"
#include "ray.h"

using nlohmann::json;

struct sphere : public hittable {
    point center;
    double radius;
    std::shared_ptr<material> mat_ptr;

    sphere(point const& center, double radius, std::shared_ptr<material> mat_ptr) 
            : center(center), radius(radius), mat_ptr(mat_ptr) { }
    sphere(point const& center, double radius) : sphere(center, radius, std::make_shared<lambertian>()) { }
    sphere() : sphere(point(0, 0, 0), 1) { }
    sphere(json const& j) {
        j.at("center").get_to(center);
        j.at("radius").get_to(radius);
        mat_ptr = material::make_from_json(j.at("material"));
    }

    static std::tuple<double, double> get_uv(point const& p) {
        auto theta = acos(-p.y);
        auto phi = atan2(-p.z, p.x) + std::numbers::pi;

        return {phi / (2 * std::numbers::pi), theta / std::numbers::pi};
    }

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
        auto outward_n = (rec.p - center) / radius;
        rec.set_face_normal(r, outward_n);
        std::tie(rec.u, rec.v) = get_uv(outward_n);
        rec.mat_ptr = mat_ptr;
        return true;
    }

    virtual std::tuple<bool, aabb> bounding_box(double t0, double t1) const override {
        auto radius = std::abs(this->radius); // XXX to support the weird negative radius spheres
        return {true, aabb(center - vec3(radius, radius, radius), center + vec3(radius, radius, radius))};
    }

    virtual double pdf_value(point const& o, vec3 const& v) const override {
        hit_record h;
        if (!hit(ray(o, v), 0.001, inf, h)) return 0;

        auto cos_theta_max = sqrt(1 - radius * radius / (center - o).length_squared());
        if (!std::isfinite(cos_theta_max)) return inf; // XXX YIKES
        assert(std::isfinite(cos_theta_max));
        auto solid_angle = 2 * pi * (1 - cos_theta_max);
        assert(solid_angle != 0);
        assert(std::isfinite(solid_angle));

        return 1 / solid_angle;
    }

    static vec3 random_to_sphere(double radius, double dist_2) {
        auto r1 = random_double();
        auto r2 = random_double();
        auto z = 1 + r2 * (sqrt(1 - radius * radius / dist_2) -1);

        if (!std::isfinite(z)) return vec3(0, 0, 0);

        auto phi = 2 * pi * r1;
        auto x = cos(phi) * sqrt(1 - z * z);
        auto y = sin(phi) * sqrt(1 - z * z);

        return {x, y, z};
    }

    virtual vec3 random_ray(point const& o) const override {
        auto dir = center - o;
        auto dist_2 = dir.length_squared();
        onb base(dir);
        return base.local(random_to_sphere(radius, dist_2));
    }

    virtual json to_json() const override {
        return json{
            {"type", "sphere"},
            {"center", center},
            {"radius", radius},
            {"material", *mat_ptr}
        };
    }

    virtual bool equals(hittable const& rhs) const override {
        if (typeid(*this) != typeid(rhs)) return false;
        auto that = static_cast<sphere const&>(rhs);
        return center == that.center && radius == that.radius && *mat_ptr == *that.mat_ptr;
    }

    virtual void print(std::ostream& os) const override {
        os << "sphere: {center: " << center << ", radius: " << radius << ", material: " << *mat_ptr << "}";
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
    moving_sphere(json const& j) {
        j.at("center_start").get_to(center_start);
        j.at("center_end").get_to(center_end);
        j.at("time_start").get_to(time_start);
        j.at("time_end").get_to(time_end);
        j.at("radius").get_to(radius);
        mat_ptr = material::make_from_json(j.at("material"));
    }

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

    virtual json to_json() const override {
        return json{
            {"type", "moving_sphere"},
            {"center_start", center_start},
            {"center_end", center_end},
            {"time_start", time_start},
            {"time_end", time_end},
            {"radius", radius},
            {"material", *mat_ptr}
        };
    }

    virtual bool equals(hittable const& rhs) const override {
        if (typeid(*this) != typeid(rhs)) return false;
        auto that = static_cast<decltype(*this)>(rhs);
        return center_start == that.center_start && center_end == that.center_end && time_start == that.time_start && time_end == that.time_end && radius == that.radius && *mat_ptr == *that.mat_ptr;
    }
};

#endif