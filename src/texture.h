#ifndef TEXTURE_H
#define TEXTURE_H

#include <nlohmann/json.hpp>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "perlin.h"
#include "vec3.h"

using nlohmann::json;

struct solid_color;
struct checker;
struct noise;
struct image;
struct texture {
    virtual color value(double u, double v, point const& p) const = 0;
    virtual json to_json() const = 0;
    virtual bool equals(texture const& rhs) const = 0;
    virtual void print(std::ostream& os) const = 0;
    bool operator==(texture const& rhs) const { return this->equals(rhs); }
    friend std::ostream& operator<<(std::ostream& os, texture const& tex) { tex.print(os); return os; }

    static std::shared_ptr<texture> make_from_json(json const& j) {
        auto type = j.at("type");
        if (type == "solid_color") return std::dynamic_pointer_cast<texture>(std::make_shared<solid_color>(j));
        else if (type == "checker") return std::dynamic_pointer_cast<texture>(std::make_shared<checker>(j));
        else if (type == "noise") return std::dynamic_pointer_cast<texture>(std::make_shared<noise>(j));
        else if (type == "image") return std::dynamic_pointer_cast<texture>(std::make_shared<image>(j));
        return nullptr;
    }
};

void to_json(json& j, texture const& tex) {
    j = tex.to_json();
}

struct solid_color : public texture {
    color c;

    solid_color(color const& c) : c(c) { }
    solid_color(double r, double g, double b) : solid_color(color(r, g, b)) { }
    solid_color() : solid_color(1, 1, 1) { }
    solid_color(json const& j) { j.at("albedo").get_to(c); }

    virtual color value(double u, double v, point const& p) const override {
        return c;
    }

    virtual json to_json() const override {
        return json{
            {"type", "solid_color"},
            {"albedo", c}
        };
    }

    bool equals(texture const& rhs) const override {
        if (typeid(*this) != typeid(rhs)) return false;
        auto that = static_cast<solid_color const&>(rhs);
        return c == that.c;
    }

    void print(std::ostream& os) const override {
        os << "solid_color: " << c;
    }
};

struct checker : public texture {
    std::shared_ptr<texture> odd;
    std::shared_ptr<texture> even;

    checker(std::shared_ptr<texture> odd, std::shared_ptr<texture> even) : odd(odd), even(even) { }
    checker(color const& odd, color const& even) : odd(std::make_shared<solid_color>(odd)), even(std::make_shared<solid_color>(even)) { }
    checker() : checker(color(0, 0, 0), color(1, 1, 1)) { }
    checker(json const& j) { 
        odd = texture::make_from_json(j.at("odd"));
        even = texture::make_from_json(j.at("even"));
    }

    virtual color value(double u, double v, point const& p) const override {
        auto sines = sin(10 * p.x) * sin(10 * p.y) * sin(10 * p.z);
        return (sines < 0 ? odd : even)->value(u, v, p);
    }

    virtual json to_json() const override {
        return json{
            {"type", "checker"},
            {"odd", odd->to_json()},
            {"even", even->to_json()}
        };
    }

    bool equals(texture const& rhs) const override {
        if (typeid(*this) != typeid(rhs)) return false;
        auto that = static_cast<checker const&>(rhs);
        return *odd == *that.odd && *even == *that.even;
    }

    void print(std::ostream& os) const override {
        os << "checker: {odd: " << *odd << ", even: " << *even << "}";
    }
};

struct noise : public texture {
    perlin prln;
    color c1;
    color c2;
    double scale;

    noise(color c1, color c2, double scale) : c1(c1), c2(c2), scale(scale) { }
    noise() : noise(color(1, 1, 1), color(0, 0, 0), 1) { }
    noise(json const& j) { 
        j.at("color1").get_to(c1); 
        j.at("color2").get_to(c2);
        j.at("scale").get_to(scale);
    }


    virtual color value(double u, double v, point const& p) const override {
        auto factor = 0.5 * (1 + sin(scale * p.z + 10 * prln.turbulence(p)));
        return c1 * factor + c2 * (1 - factor);
    }

    virtual json to_json() const override {
        return json{
            {"type", "noise"},
            {"color1", c1},
            {"color2", c2},
            {"scale", scale}
        };
    }

    bool equals(texture const& rhs) const override {
        if (typeid(*this) != typeid(rhs)) return false;
        auto that = static_cast<noise const&>(rhs);
        return c1 == that.c1 && c2 == that.c2;
    }

    void print(std::ostream& os) const override {
        os << "noise: {color1: " << c1 << ", color2: " << c2 << ", scale: " << scale << "}";
    }
};

struct image : public texture {
    static constexpr int bytes_per_pixel = 3;
    std::shared_ptr<unsigned char> data;
    int width;
    int height;
    int bytes_per_line;
    std::string path;

    image() : data(nullptr), width(0), height(0), bytes_per_line(0), path("") { }
    image(std::string path) : path(path) {
        auto components_per_pixel = bytes_per_pixel;
        data = std::shared_ptr<unsigned char>(stbi_load(path.c_str(), &width, &height, &components_per_pixel, components_per_pixel));

        if (!data) {
            std::cerr << "ERROR loading image texture from " << path << std::endl;
            width = 0;
            height = 0;
        }

        bytes_per_line = width * bytes_per_pixel;
    }
    image(char const* c) : image(std::string(c)) { }
    image(json const& j) : image(j.at("path").get<std::string>()) { }

    virtual color value(double u, double v, point const& p) const override {
        if (!data) return color(0, 1, 1);

        u = clamp(u, 0, 1);
        v = 1 - clamp(v, 0, 1);

        int i = u * width;
        int j = v * height;

        if (i >= width) i = width - 1;
        if (j >= height) j = height - 1;

        static constexpr double color_scale = 1.0 / 255.0;
        auto pixel = data.get() + j * bytes_per_line + i * bytes_per_pixel;
        auto c = color(pixel[0], pixel[1], pixel[2]);
        return c * color_scale;
    }

    virtual json to_json() const override {
        return json{
            {"type", "image"},
            {"path", path}
        };
    }

    virtual bool equals(texture const& rhs) const override {
        if (typeid(*this) != typeid(rhs)) return false;
        auto that = static_cast<image const&>(rhs);
        return path == that.path;
    }

    virtual void print(std::ostream& os) const override {
        os << "image: " << path;
    }
};

#endif