#ifndef MATERIAL_H
#define MATERIAL_H

#include <nlohmann/json.hpp>
#include "hittable.h"
#include "onb.h"
#include "pdf.h"
#include "ray.h"
#include "texture.h"
#include "vec3.h"

using nlohmann::json;

struct scatter_record {
    color attenuation;
    ray specular;
    std::shared_ptr<pdf> pdf_ptr;
    bool is_specular;
};

struct lambertian;
struct metal;
struct dielectric;
struct diffuse_light;
struct isotropic;
struct material {
    virtual ~material() {}
    virtual std::tuple<bool, scatter_record> scatter(ray const& r_in, hit_record const& h, double lambda) const {
        return {false, {color(0), ray(point(0, 0, 0), vec3(0, 0, 1)), nullptr, false}};
    }
    virtual double scattering_pdf(ray const& r_in, hit_record const& h, ray const& scattered) const {
        return 0;
    }
    virtual color emitted(ray const& r_in, hit_record const& h, double u, double v, point const& p) const {
        return color(0);
    }
    virtual json to_json() const = 0;
    virtual bool equals(material const& rhs) const = 0;
    void print(std::ostream& os) const { os << to_json(); }
    bool operator==(material const& rhs) const { return this->equals(rhs); }
    friend std::ostream& operator<<(std::ostream& os, material const& rhs) { rhs.print(os); return os; }

    static std::shared_ptr<material> make_from_json(json const& j) {
        auto type = j.at("type");
        if (type == "lambertian") return std::dynamic_pointer_cast<material>(std::make_shared<lambertian>(j));
        else if (type == "metal") return std::dynamic_pointer_cast<material>(std::make_shared<metal>(j));
        else if (type == "dielectric") return std::dynamic_pointer_cast<material>(std::make_shared<dielectric>(j));
        else if (type == "diffuse_light") return std::dynamic_pointer_cast<material>(std::make_shared<diffuse_light>(j));
        else if (type == "isotropic") return std::dynamic_pointer_cast<material>(std::make_shared<isotropic>(j));
        std::cerr << "unknown material: " << type << std::endl;
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
    lambertian() : lambertian(color(1)) { }
    lambertian(json const& j) { albedo = texture::make_from_json(j.at("albedo")); }

    virtual std::tuple<bool, scatter_record> scatter(ray const& r_in, hit_record const& h, double lambda) const override {
        //onb base(h.n);
        //auto scatter_dir = base.local(vec3_random_cosine());
        //ray r_out(h.p, normalize(scatter_dir), r_in.time);
        //return {true, albedo->value(h.u, h.v, h.p), r_out, dot(base.w, r_out.direction) / std::numbers::pi};
        return {true, {albedo->value(h.u, h.v, h.p), ray(), std::make_shared<cosine_pdf>(h.n), false}};
    }

    virtual double scattering_pdf(ray const& r_in, hit_record const& h, ray const& scattered) const override {
        auto cos = dot(h.n, normalize(scattered.direction));
        return cos < 0 ? 0 : cos / std::numbers::pi;
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
};

struct metal : public material {
    color albedo;
    double fuzz;

    metal(color const& albedo, double fuzz) : albedo(albedo), fuzz(fuzz) { }
    metal(color const& albedo) : metal(albedo, 0) { }
    metal() : metal(color(1)) { }
    metal(json const& j) {
        j.at("albedo").get_to(albedo);
        j.at("fuzz").get_to(fuzz);
    }

    virtual std::tuple<bool, scatter_record> scatter(ray const& r_in, hit_record const& h, double lambda) const override {
        vec3 reflected = reflect(normalize(r_in.direction), h.n) + fuzz * vec3_random_sphere();
        ray r_out(h.p, reflected, r_in.time);
        return {true, {albedo, r_out, nullptr, true}};
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
};

double ir_baf10(double lambda) {
    auto lambda_um_2 = std::pow(lambda / 1000, 2);
    return std::sqrt(1
            + 1.5851495 / (1 - 0.00926681282 / lambda_um_2)
            + 0.143559385 / (1 - 0.0424489805 / lambda_um_2)
            + 1.08521269 / (1 - 105.613573 / lambda_um_2));
}

double ir_bk7(double lambda) {
    //return ir0;
    auto lambda_um_2 = std::pow(lambda / 1000, 2);
    return std::sqrt(1
            + 1.03961212 / (1 - 0.00600069867 / lambda_um_2)
            + 0.231792344 / (1 - 0.0200179144 / lambda_um_2)
            + 1.01046945 / (1 - 103.560653 / lambda_um_2));
}

struct dielectric : public material {
    double ir0;

    dielectric(double ir) : ir0(ir) { }
    dielectric() : dielectric(1.5) { }
    dielectric(json const& j) { j.at("ir").get_to(ir0); }

    virtual std::tuple<bool, scatter_record> scatter(ray const& r_in, hit_record const& h, double lambda) const override {
        //static auto ir_550 = ir(550);
        //auto ir_lambda = ir_550 + 5 * (ir(lambda) - ir_550);
        auto ir_lambda = ir_baf10(lambda);
        auto ratio = h.front_face ? 1 / ir_lambda : ir_lambda;
        auto dir_n = normalize(r_in.direction);
        auto cos_theta = std::fmin(dot(-dir_n, h.n), 1);
        auto sin_theta = std::sqrt(1 - cos_theta * cos_theta);
        auto refracted = refract(dir_n, h.n, ratio);
        if (ratio * sin_theta > 1 || reflectance(cos_theta, ratio) > random_double()) {
            refracted = reflect(dir_n, h.n);
        }
        ray r_out(h.p, refracted, r_in.time);
        return {true, {color(1), r_out, nullptr, true}};
    }

    static double reflectance(double cos_theta, double ir) {
        auto sqrt_r0 = (1 - ir) / (1 + ir);
        auto r0 = sqrt_r0 * sqrt_r0;
        return r0 + (1 - r0) * std::pow(1 - cos_theta, 5);
    }

    virtual json to_json() const override {
        return json{
            {"type", "dielectric"},
            {"ir", ir0}
        };
    }

    virtual bool equals(material const& rhs) const override {
        if (typeid(*this) != typeid(rhs)) return false;
        auto that = static_cast<dielectric const&>(rhs);
        return ir0 == that.ir0;
    }
};

struct diffuse_light : public material {
    std::shared_ptr<texture> light;

    diffuse_light(std::shared_ptr<texture> light) : light(light) { }
    diffuse_light(color const& c) : diffuse_light(std::make_shared<solid_color>(c)) { }
    diffuse_light() : diffuse_light(color(1)) { }
    diffuse_light(json const& j) {
        light = texture::make_from_json(j.at("light"));
    }

    virtual json to_json() const override {
        return json{
            {"type", "diffuse_light"},
            {"light", *light}
        };
    }

    virtual color emitted(ray const& r_in, hit_record const& h, double u, double v, point const& p) const override {
        return h.front_face ? light->value(u, v, p) : color(0);
    }

    virtual bool equals(material const& rhs) const override {
        if (typeid(*this) != typeid(rhs)) return false;
        auto that = static_cast<diffuse_light const&>(rhs);
        return *light == *that.light;
    }
};

#endif