#ifndef MATERIAL_H
#define MATERIAL_H

#include "hittable.h"
#include "ray.h"
#include "vec3.h"

struct material {
    virtual ~material() {}
    virtual std::tuple<bool, color, ray> scatter(ray const& r_in, hit_record const& h) const = 0;
};

struct lambertian : public material {
    color albedo;

    lambertian(color const& albedo) : albedo(albedo) { }

    virtual std::tuple<bool, color, ray> scatter(ray const& r_in, hit_record const& h) const override {
        auto scatter_dir = h.n + vec3_random_unit();
        if (scatter_dir.near_zero()) scatter_dir = h.n;
        return {true, albedo, ray(h.p, scatter_dir)};
    }      
};

struct metal : public material {
    color albedo;

    metal(color const& albedo) : albedo(albedo) { }

    virtual std::tuple<bool, color, ray> scatter(ray const& r_in, hit_record const& h) const override {
        vec3 reflected = reflect(normalize(r_in.direction), h.n);
        return {dot(reflected, h.n) > 0, albedo, ray(h.p, reflected)};
    }
};

struct dielectric : public material {
    double ir;

    dielectric(double ir) : ir(ir) { }

    virtual std::tuple<bool, color, ray> scatter(ray const& r_in, hit_record const& h) const override {
        auto ratio = h.front_face ? 1 / ir : ir;
        auto dir_n = normalize(r_in.direction);
        auto cos_theta = std::fmin(dot(-dir_n, h.n), 1);
        auto sin_theta = std::sqrt(1 - cos_theta * cos_theta);
        auto refracted = refract(dir_n, h.n, ratio);
        if (ratio * sin_theta > 1 || reflectance(cos_theta, ratio) > random_double()) {
            refracted = reflect(dir_n, h.n);
        }
        return {true, color(1, 1, 1), ray(h.p, refracted)};
    }

    static double reflectance(double cos_theta, double ir) {
        auto sqrt_r0 = (1 - ir) / (1 + ir);
        auto r0 = sqrt_r0 * sqrt_r0;
        return r0 + (1 - r0) * std::pow(1 - cos_theta, 5);
    }
};

#endif