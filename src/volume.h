#ifndef VOLUME_H
#define VOLUME_H

#include <memory>
#include "hittable.h"
#include "material.h"
#include "texture.h"
#include "vec3.h"

struct isotropic : public material {
    std::shared_ptr<texture> albedo;

    isotropic(std::shared_ptr<texture> albedo) : albedo(albedo) { }
    isotropic(color const& c) : isotropic(std::make_shared<solid_color>(c)) { }
    isotropic() : isotropic(color(0, 0, 0)) { }

    virtual std::tuple<bool, color, ray, double> scatter(ray const& r_in, hit_record const& h) const override {
        ray r_out(h.p, vec3_random_sphere(), r_in.time);
        return {true, albedo->value(h.u, h.v, h.p), r_out, scattering_pdf(r_in, h, r_out)};
    }

    virtual json to_json() const override {
        return json{
            {"type", "isotropic"},
            {"albedo", *albedo}
        };
    }

    virtual bool equals(material const& rhs) const override {
        if (typeid(*this) != typeid(rhs)) return false;
        auto that = static_cast<isotropic const&>(rhs);
        return *albedo == *that.albedo;
    }

    virtual void print(std::ostream& os) const override {
        os << "isotropic: " << albedo;
    }
};

struct constant_medium : public hittable {
    std::shared_ptr<hittable> boundary;
    std::shared_ptr<isotropic> phase_function; // XXX if we're only ever using isotropic I'm not gonna bother with casts
    double neg_inv_density;
    double density;

    void construct() { neg_inv_density = -1 / density; }

    constant_medium(std::shared_ptr<hittable> boundary, double density, std::shared_ptr<texture> albedo) 
            : boundary(boundary), density(density), phase_function(std::make_shared<isotropic>(albedo)) { construct(); }
    constant_medium() : constant_medium(nullptr, 0, nullptr) { }
    constant_medium(std::shared_ptr<hittable> boundary, double density, color const& c) : constant_medium(boundary, density, std::make_shared<solid_color>(c)) { }
    constant_medium(json const& j) {
        boundary = hittable::make_from_json(j.at("boundary"));
        phase_function = std::make_shared<isotropic>(texture::make_from_json(j.at("albedo")));
        j.at("density").get_to(density);
        construct();
    }

    virtual bool hit(ray const& r, double t_min, double t_max, hit_record& h) const override {
        hit_record h1, h2;

        if (!boundary->hit(r, -inf, inf, h1)) return false;
        if (!boundary->hit(r, h1.t + 0.0001, inf, h2)) return false;

        if (h1.t < t_min) h1.t = t_min;
        if (h2.t > t_max) h2.t = t_max;

        if (h1.t >= h2.t) return false;

        if (h1.t < 0) h1.t = 0;

        auto const ray_length = r.direction.length();
        auto const distance_inside_boundary = (h2.t - h1.t) * ray_length;
        auto const hit_distance = neg_inv_density * log(random_double());

        if (hit_distance > distance_inside_boundary) return false;

        h.t = h1.t + hit_distance / ray_length;
        h.p = r.at(h.t);

        h.n = vec3(1, 0, 0);
        h.front_face = true;
        h.mat_ptr = phase_function;

        return true;
    }

    virtual std::tuple<bool, aabb> bounding_box(double t0, double t1) const override {
        return boundary->bounding_box(t0, t1);
    }

    virtual json to_json() const override {
        return json{
            {"type", "constant_medium"},
            {"boundary", *boundary},
            {"albedo", *phase_function->albedo},
            {"density", density}
        };
    }

    virtual bool equals(hittable const& rhs) const override {
        if (typeid(*this) != typeid(rhs)) return false;
        auto that = static_cast<constant_medium const&>(rhs);
        return *boundary == *that.boundary && *phase_function->albedo == *that.phase_function->albedo && density == that.density;
    }

    virtual void print(std::ostream& os) const override {
        os << "constant medium: {boundary: " << *boundary << ", albedo: " << *phase_function->albedo << ", density: " << density << "}";
    }
};

#endif