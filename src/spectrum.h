#ifndef SPECTRUM_H
#define SPECTRUM_H

#include "vec3.h"

template<size_t SampleCount>
struct color_spectrum {
    static constexpr double lambda_start = 400;
    static constexpr double lambda_end = 700;

    std::array<double, SampleCount> samples;

    constexpr bool operator==(color_spectrum const& rhs) const {
        return samples == rhs.samples;
    }

    constexpr double const& operator[](int i) const { return samples[i]; }
    constexpr double& operator[](int i) { return samples[i]; }

    color_spectrum& operator+=(color_spectrum const& rhs) {
        for (int i = 0; i < SampleCount; ++i) samples[i] += rhs.samples[i];
        return *this;
    }

    constexpr static color_spectrum from_samples(std::vector<double> const& lambas, std::vector<double> const& values) {
        assert(lambdas.size() == values.size());
    }
};

template<size_t SampleCount>
constexpr color_spectrum<SampleCount> operator+(color_spectrum<SampleCount> const& c, double d) {
    color_spectrum<SampleCount> res; for (int i = 0; i < SampleCount; ++i) res[i] = c[i] + d; return res;
}
template<size_t SampleCount>
constexpr color_spectrum<SampleCount> operator+(color_spectrum<SampleCount> const& c, color_spectrum<SampleCount> const& d) {
    color_spectrum<SampleCount> res; for (int i = 0; i < SampleCount; ++i) res[i] = c[i] + d[i]; return res;
}
template<size_t SampleCount>
constexpr color_spectrum<SampleCount> operator*(color_spectrum<SampleCount> const& c, double d) {
    color_spectrum<SampleCount> res; for (int i = 0; i < SampleCount; ++i) res[i] = c[i] * d; return res;
}
template<size_t SampleCount>
constexpr color_spectrum<SampleCount> operator*(double d, color_spectrum<SampleCount> const& c) {
    color_spectrum<SampleCount> res; for (int i = 0; i < SampleCount; ++i) res[i] = c[i] * d; return res;
}
template<size_t SampleCount>
constexpr color_spectrum<SampleCount> operator*(color_spectrum<SampleCount> const& c, color_spectrum<SampleCount> const& d) {
    color_spectrum<SampleCount> res; for (int i = 0; i < SampleCount; ++i) res[i] = c[i] * d[i]; return res;
}
template<size_t SampleCount>
constexpr color_spectrum<SampleCount> operator/(color_spectrum<SampleCount> const& c, double d) {
    color_spectrum<SampleCount> res; for (int i = 0; i < SampleCount; ++i) res[i] = c[i] / d; return res;
}

struct color_rgb {
    double r, g, b;

    constexpr color_rgb(double r, double g, double b) : r(r), g(g), b(b) { }
    constexpr color_rgb() : color_rgb(0, 0, 0) { }

    constexpr bool operator==(color_rgb const& rhs) const {
        return r == rhs.r && g == rhs.g && b == rhs.b;
    }

    constexpr double const& operator[](int i) const {
        if (i == 0) return r;
        else if (i == 1) return g;
        else return b;
    }

    constexpr double& operator[](int i) {
        if (i == 0) return r;
        else if (i == 1) return g;
        else return b;
    }

    color_rgb& operator+=(color_rgb const& rhs) {
        r += rhs.r;
        g += rhs.g;
        b += rhs.b;
        return *this;
    }

    static color_rgb random() {
        return {random_double(), random_double(), random_double()};
    }

    static color_rgb random(double min, double max) {
        return {random_double(min, max), random_double(min, max), random_double(min, max)};
    }
};

constexpr color_rgb operator+(color_rgb const& c, color_rgb const& d) { return {c.r + d.r, c.g + d.g, c.b + d.b}; }
constexpr color_rgb operator+(color_rgb const& c, double d) { return {c.r + d, c.g + d, c.b + d}; }
constexpr color_rgb operator*(color_rgb const& c, double d) { return {c.r * d, c.g * d, c.b * d}; }
constexpr color_rgb operator*(double d, color_rgb const& c) { return {c.r * d, c.g * d, c.b * d}; }
constexpr color_rgb operator*(color_rgb const& c, color_rgb const& d) { return {c.r * d.r, c.g * d.g, c.b * d.b}; }
constexpr color_rgb operator/(color_rgb const& c, double d) { return {c.r / d, c.g / d, c.b / d}; }

std::ostream& operator<<(std::ostream& os, color_rgb const& c) {
    os << "{" << c.r << ", " << c.g << ", " << c.b << "}";
    return os;
}

using nlohmann::json;
void to_json(json& j, color_rgb const& c) {
    j = json{
        {"r", c.r},
        {"g", c.g},
        {"b", c.b}
    };
}
void from_json(json const& j, color_rgb& c) {
    j.at("r").get_to(c.r);
    j.at("g").get_to(c.g);
    j.at("b").get_to(c.b);
}

using color = color_rgb;

#endif