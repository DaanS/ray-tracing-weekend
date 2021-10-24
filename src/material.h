#ifndef MATERIAL_H
#define MATERIAL_H

#include "hittable.h"
#include "ray.h"
#include "vec3.h"

struct material {
    virtual ~material() {}
    virtual bool scatter(ray const& r_in, hit_record const& h, color& attenuation, ray& scattered) const = 0;
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
};

#endif