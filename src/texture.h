#ifndef TEXTURE_H
#define TEXTURE_H

#include "vec3.h"

struct texture {
    virtual color value(double u, double v, point const& p) const = 0;
};

struct solid_color : public texture {
    color c;

    solid_color(color const& c) : c(c) { }
    solid_color(double r, double g, double b) : solid_color(color(r, g, b)) { }

    virtual color value(double u, double v, point const& p) const override {
        return c;
    }
};

struct checker : public texture {
    std::shared_ptr<texture> odd;
    std::shared_ptr<texture> even;

    checker(std::shared_ptr<texture> odd, std::shared_ptr<texture> even) : odd(odd), even(even) { }
    checker(color const& odd, color const& even) : odd(std::make_shared<solid_color>(odd)), even(std::make_shared<solid_color>(even)) { }

    virtual color value(double u, double v, point const& p) const override {
        auto sines = sin(10 * p.x) * sin(10 * p.y) * sin(10 * p.z);
        return (sines < 0 ? odd : even)->value(u, v, p);
    }
};

#endif