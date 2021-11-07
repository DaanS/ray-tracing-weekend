#ifndef TEXTURE_H
#define TEXTURE_H

#include <nlohmann/json.hpp>
#include "vec3.h"

using nlohmann::json;

struct solid_color;
struct checker;
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

#endif