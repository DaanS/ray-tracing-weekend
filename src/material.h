#ifndef MATERIAL_H
#define MATERIAL_H

#include <nlohmann/json.hpp>
#include "hittable.h"
#include "ray.h"
#include "texture.h"
#include "vec3.h"

using nlohmann::json;

struct lambertian;
struct metal;
struct dielectric;
struct material {
    virtual ~material() {}
    virtual std::tuple<bool, color, ray> scatter(ray const& r_in, hit_record const& h) const = 0;
    virtual json to_json() const = 0;
    virtual bool equals(material const& rhs) const = 0;
    virtual void print(std::ostream& os) const = 0;
    bool operator==(material const& rhs) const { return this->equals(rhs); }
    friend std::ostream& operator<<(std::ostream& os, material const& rhs) { rhs.print(os); return os; }

    static std::shared_ptr<material> make_from_json(json const& j) {
        auto type = j.at("type");
        if (type == "lambertian") return std::dynamic_pointer_cast<material>(std::make_shared<lambertian>(j));
        else if (type == "metal") return std::dynamic_pointer_cast<material>(std::make_shared<metal>(j));
        else if (type == "dielectric") return std::dynamic_pointer_cast<material>(std::make_shared<dielectric>(j));
        return nullptr;
    }
};

void to_json(json& j, material const& mat) {
    j = mat.to_json();
}

struct lambertian : public material {
    std::shared_ptr<texture> albedo;

    lambertian(color const& albedo) : lambertian(std::make_shared<solid_color>(albedo)) { }
    lambertian(std::shared_ptr<texture> albedo) : albedo(albedo) { }
    lambertian() : lambertian(color(1, 1, 1)) { }
    lambertian(json const& j) { albedo = texture::make_from_json(j.at("albedo")); }

    virtual std::tuple<bool, color, ray> scatter(ray const& r_in, hit_record const& h) const override {
        auto scatter_dir = h.n + vec3_random_unit();
        if (scatter_dir.near_zero()) scatter_dir = h.n;
        return {true, albedo->value(h.u, h.v, h.p), ray(h.p, scatter_dir, r_in.time)};
    }      

    virtual json to_json() const override {
        return json{
            {"type", "lambertian"},
            {"albedo", *albedo}
        };
    }

    virtual bool equals(material const& rhs) const override {
        if (typeid(*this) != typeid(rhs)) return false;
        auto that = static_cast<lambertian const&>(rhs);
        return *albedo == *that.albedo;
    }

    virtual void print(std::ostream& os) const override {
        os << "lambertian: " << albedo;
    }
};

struct metal : public material {
    color albedo;
    double fuzz;

    metal(color const& albedo, double fuzz) : albedo(albedo), fuzz(fuzz) { }
    metal(color const& albedo) : metal(albedo, 0) { }
    metal() : metal(color(1, 1, 1)) { }
    metal(json const& j) {
        j.at("albedo").get_to(albedo);
        j.at("fuzz").get_to(fuzz);
    }

    virtual std::tuple<bool, color, ray> scatter(ray const& r_in, hit_record const& h) const override {
        vec3 reflected = reflect(normalize(r_in.direction), h.n) + fuzz * vec3_random_sphere();
        return {dot(reflected, h.n) > 0, albedo, ray(h.p, reflected, r_in.time)};
    }

    virtual json to_json() const override {
        return json{
            {"type", "metal"},
            {"albedo", albedo},
            {"fuzz", fuzz}
        };
    }

    virtual bool equals(material const& rhs) const override {
        if (typeid(*this) != typeid(rhs)) return false;
        auto that = static_cast<metal const&>(rhs);
        return albedo == that.albedo && fuzz == that.fuzz;
    }

    virtual void print(std::ostream& os) const override {
        os << "metal: {albedo: " << albedo << ", fuzz: " << fuzz << "}";
    }
};

struct dielectric : public material {
    double ir;

    dielectric(double ir) : ir(ir) { }
    dielectric() : dielectric(1.5) { }
    dielectric(json const& j) { j.at("ir").get_to(ir); }

    virtual std::tuple<bool, color, ray> scatter(ray const& r_in, hit_record const& h) const override {
        auto ratio = h.front_face ? 1 / ir : ir;
        auto dir_n = normalize(r_in.direction);
        auto cos_theta = std::fmin(dot(-dir_n, h.n), 1);
        auto sin_theta = std::sqrt(1 - cos_theta * cos_theta);
        auto refracted = refract(dir_n, h.n, ratio);
        if (ratio * sin_theta > 1 || reflectance(cos_theta, ratio) > random_double()) {
            refracted = reflect(dir_n, h.n);
        }
        return {true, color(1, 1, 1), ray(h.p, refracted, r_in.time)};
    }

    static double reflectance(double cos_theta, double ir) {
        auto sqrt_r0 = (1 - ir) / (1 + ir);
        auto r0 = sqrt_r0 * sqrt_r0;
        return r0 + (1 - r0) * std::pow(1 - cos_theta, 5);
    }

    virtual json to_json() const override {
        return json{
            {"type", "dielectric"},
            {"ir", ir}
        };
    }

    virtual bool equals(material const& rhs) const override {
        if (typeid(*this) != typeid(rhs)) return false;
        auto that = static_cast<dielectric const&>(rhs);
        return ir == that.ir;
    }

    virtual void print(std::ostream& os) const override {
        os << "dielectric: " << ir;
    }
};

#endif