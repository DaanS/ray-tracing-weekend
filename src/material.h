#ifndef MATERIAL_H
#define MATERIAL_H

#include "hittable.h"
#include "ray.h"
#include "vec3.h"

struct material {
    virtual ~material() {}
    virtual bool scatter(ray const& r_in, hit_record const& h, color& attenuation, ray& scattered) const = 0;
    virtual std::tuple<bool, color, ray> scatter(ray const& r_in, hit_record const& h) const {
        color c;
        ray r(point(0, 0, 0), vec3(0, 0, 0));
        bool res = scatter(r_in, h, c, r);
        return {res, c, r};
    }
};

struct lambertian : public material {
    color albedo;

    lambertian(color const& albedo) : albedo(albedo) { }

    // TODO see if this is nicer with structured bindings maybe?
    virtual bool scatter(ray const& r_in, hit_record const& h, color& attenuation, ray& scattered) const override {
        auto scatter_dir = h.n + vec3_random_unit();
        if (scatter_dir.near_zero()) scatter_dir = h.n;
        scattered = ray(h.p, scatter_dir);
        attenuation = albedo;
        return true;
    }

    virtual std::tuple<bool, color, ray> scatter(ray const& r_in, hit_record const& h) const override {
        auto scatter_dir = h.n + vec3_random_unit();
        if (scatter_dir.near_zero()) scatter_dir = h.n;
        return { true, albedo, ray(h.p, scatter_dir) };
    }      
};

struct metal : public material {
    color albedo;

    metal(color const& albedo) : albedo(albedo) { }

    virtual bool scatter(ray const& r_in, hit_record const& h, color& attenuation, ray& scattered) const override {
        vec3 reflected = reflect(normalize(r_in.direction), h.n);
        scattered = ray(h.p, reflected);
        attenuation = albedo;
        return dot(scattered.direction, h.n) > 0;
    }

    virtual std::tuple<bool, color, ray> scatter(ray const& r_in, hit_record const& h) const override {
        vec3 reflected = reflect(normalize(r_in.direction), h.n);
        return { dot(reflected, h.n) > 0, albedo, ray(h.p, reflected) };
    }
};

#endif