#ifndef SPECTRUM_H
#define SPECTRUM_H

#include <ranges>
#include <fstream>
#include "vec3.h"
#include "spectrum_consts.h"

constexpr vec3 rgb_to_xyz(vec3 rgb) {
    return {
        0.412453f*rgb[0] + 0.357580f*rgb[1] + 0.180423f*rgb[2],
        0.212671f*rgb[0] + 0.715160f*rgb[1] + 0.072169f*rgb[2],
        0.019334f*rgb[0] + 0.119193f*rgb[1] + 0.950227f*rgb[2]
    };
}

constexpr vec3 xyz_to_rgb(vec3 xyz) {
    return {
         3.240479f*xyz[0] - 1.537150f*xyz[1] - 0.498535f*xyz[2],
        -0.969256f*xyz[0] + 1.875991f*xyz[1] + 0.041556f*xyz[2],
         0.055648f*xyz[0] - 0.204043f*xyz[1] + 1.057311f*xyz[2]
    };
}

enum class spectrum_type { reflectance, illuminant };
enum class rgb_base { white, red, blue, green, cyan, magenta, yellow };

struct color_rgb {
    double r, g, b;

    constexpr color_rgb(vec3 rgb) : r(rgb.x), g(rgb.y), b(rgb.z) { }
    constexpr color_rgb(double d) : r(d), g(d), b(d) { }
    constexpr color_rgb(double r, double g, double b) : r(r), g(g), b(b) { }
    //constexpr color_rgb(uint32_t hex) : r(((hex & 0x00ff0000) >> 16) / 255.0), g(((hex & 0x0000ff00) >> 8) / 255.0), b((hex & 0x000000ff) / 255.0) { }
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

    color_rgb to_rgb() const {
        return *this;
    }

    vec3 to_rgb_v() const {
        return {r, g, b};
    }

    static color_rgb from_rgb(double r, double g, double b, spectrum_type) {
        return {r, g, b};
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

struct spectrum_sample {
    double lambda;
    double value;
};

struct color_spectrum;
constexpr color_spectrum operator*(double d, color_spectrum const& c);

struct color_spectrum {
    static constexpr double lambda_start = 400;
    static constexpr double lambda_end = 700;
    static constexpr size_t sample_count = 60;

    std::array<double, sample_count> samples;

    constexpr color_spectrum() { }
    color_spectrum(double d) {
        samples.fill(d);
    }
    color_spectrum(double r, double g, double b, spectrum_type type = spectrum_type::reflectance) {
        samples = from_rgb(r, g, b, type).samples;
    }

    constexpr bool operator==(color_spectrum const& rhs) const {
        return samples == rhs.samples;
    }

    constexpr double const& operator[](int i) const { return samples[i]; }
    constexpr double& operator[](int i) { return samples[i]; }

    color_spectrum& operator+=(color_spectrum const& rhs) {
        for (int i = 0; i < sample_count; ++i) samples[i] += rhs.samples[i];
        return *this;
    }

    static double sample_average_in_range(std::vector<spectrum_sample> const& samples, int range_idx) {
        // get start and end of range based on range_idx
        auto range_start = std::lerp(lambda_start, lambda_end, (double) range_idx / (sample_count));
        auto range_end = std::lerp(lambda_start, lambda_end, (double) (range_idx + 1) / (sample_count));

        // check if the range is before the first sample, or after the last sample
        if (range_end < samples[0].lambda) return samples[0].value;
        if (range_start > samples.back().lambda) return samples.back().value;

        double sum = 0;

        // range isn't fully outside the samples, but might still be partially outside
        if (range_start < samples[0].lambda) sum += samples[0].value * (samples[0].lambda - range_start);
        if (range_end > samples.back().lambda) sum += samples.back().value * (range_end - samples.back().lambda);

        int i = 0;
        // if we have samples before the start of the range, we're only interested in the last one of those
        while (samples[i + 1].lambda < range_start) ++i;
        while (samples[i].lambda <= range_end && (i + 1) < samples.size()) {
            // get the endpoints of the overlap between spectrum range and the sampled range
            auto start_lambda = std::max(samples[i].lambda, range_start);
            auto end_lambda = std::min(samples[i + 1].lambda, range_end);
            // interpolate the samples to get values for these endpoints
            auto start_val = std::lerp(samples[i].value, samples[i + 1].value, (start_lambda - samples[i].lambda) / (samples[i + 1].lambda - samples[i].lambda));
            auto end_val = std::lerp(samples[i].value, samples[i + 1].value, (end_lambda - samples[i].lambda) / (samples[i + 1].lambda - samples[i].lambda));
            sum += 0.5 * (end_val + start_val) * (end_lambda - start_lambda);
            ++i;
        }
        return sum / (range_end - range_start);
    }

    template<typename T>
    requires std::ranges::range<T> && std::same_as<std::ranges::range_value_t<T>, spectrum_sample>
    static color_spectrum from_samples(T samples) {
        std::ranges::sort(samples, [](auto lhs, auto rhs) { return lhs.lambda < rhs.lambda; });

        color_spectrum res;

        if (samples.size() == 0) { res.samples.fill(0); return res; }
        if (samples.size() == 1) { res.samples.fill(samples[0].value); return res; }

        for (int i = 0; i < sample_count; ++i) {
            res[i] = sample_average_in_range(samples, i);
        }

        return res;
    }

    static color_spectrum from_pbrt(std::string const& path) {
        std::vector<spectrum_sample> samples;
        std::ifstream ifs(path);
        std::string line;
        while (getline(ifs, line)) {
            if (line.starts_with('#')) continue;
            double lambda, value;
            ifs >> lambda >> value;
            samples.push_back({lambda, value});
        }
        return from_samples(samples);
    }

    static std::vector<spectrum_sample> sample_zip(double const* lambdas, double const* vals, size_t count) {
        std::vector<spectrum_sample> res;
        res.reserve(count);
        for (int i = 0; i < count; ++i) res.push_back({lambdas[i], vals[i]});
        return res;
    }

    static color_spectrum const& X() {
        static color_spectrum x = color_spectrum::from_samples(sample_zip(cie_lambda, cie_x, cie_sample_count));
        return x;
    }
    static color_spectrum const& Y() {
        static color_spectrum y = color_spectrum::from_samples(sample_zip(cie_lambda, cie_y, cie_sample_count));
        return y;
    }
    static color_spectrum const& Z() {
        static color_spectrum z = color_spectrum::from_samples(sample_zip(cie_lambda, cie_z, cie_sample_count));
        return z;
    }

    static color_spectrum const& refl_white() {
        static color_spectrum instance = color_spectrum::from_samples(sample_zip(rgb2_lambda, rgb2_refl_white, rgb2_sample_count));
        return instance;
    }
    static color_spectrum const& refl_red() {
        static color_spectrum instance = color_spectrum::from_samples(sample_zip(rgb2_lambda, rgb2_refl_red, rgb2_sample_count));
        return instance;
    }
    static color_spectrum const& refl_blue() {
        static color_spectrum instance = color_spectrum::from_samples(sample_zip(rgb2_lambda, rgb2_refl_blue, rgb2_sample_count));
        return instance;
    }
    static color_spectrum const& refl_green() {
        static color_spectrum instance = color_spectrum::from_samples(sample_zip(rgb2_lambda, rgb2_refl_green, rgb2_sample_count));
        return instance;
    }
    static color_spectrum const& refl_cyan() {
        static color_spectrum instance = color_spectrum::from_samples(sample_zip(rgb2_lambda, rgb2_refl_cyan, rgb2_sample_count));
        return instance;
    }
    static color_spectrum const& refl_magenta() {
        static color_spectrum instance = color_spectrum::from_samples(sample_zip(rgb2_lambda, rgb2_refl_magenta, rgb2_sample_count));
        return instance;
    }
    static color_spectrum const& refl_yellow() {
        static color_spectrum instance = color_spectrum::from_samples(sample_zip(rgb2_lambda, rgb2_refl_white, rgb2_sample_count));
        return instance;
    }

    static color_spectrum const& illum_white() {
        static color_spectrum instance = color_spectrum::from_samples(sample_zip(rgb2_lambda, rgb2_illum_white, rgb2_sample_count));
        return instance;
    }
    static color_spectrum const& illum_red() {
        static color_spectrum instance = color_spectrum::from_samples(sample_zip(rgb2_lambda, rgb2_illum_red, rgb2_sample_count));
        return instance;
    }
    static color_spectrum const& illum_blue() {
        static color_spectrum instance = color_spectrum::from_samples(sample_zip(rgb2_lambda, rgb2_illum_blue, rgb2_sample_count));
        return instance;
    }
    static color_spectrum const& illum_green() {
        static color_spectrum instance = color_spectrum::from_samples(sample_zip(rgb2_lambda, rgb2_illum_green, rgb2_sample_count));
        return instance;
    }
    static color_spectrum const& illum_cyan() {
        static color_spectrum instance = color_spectrum::from_samples(sample_zip(rgb2_lambda, rgb2_illum_cyan, rgb2_sample_count));
        return instance;
    }
    static color_spectrum const& illum_magenta() {
        static color_spectrum instance = color_spectrum::from_samples(sample_zip(rgb2_lambda, rgb2_illum_magenta, rgb2_sample_count));
        return instance;
    }
    static color_spectrum const& illum_yellow() {
        static color_spectrum instance = color_spectrum::from_samples(sample_zip(rgb2_lambda, rgb2_illum_white, rgb2_sample_count));
        return instance;
    }

    vec3 to_xyz() const {
        vec3 res(0, 0, 0);
        for (int i = 0; i < sample_count; ++i) {
            res[0] += X()[i] * samples[i];
            res[1] += Y()[i] * samples[i];
            res[2] += Z()[i] * samples[i];
        }
        res *= (lambda_end - lambda_start) / (sample_count * cie_y_integral);
        return res;
    }

    color_rgb to_rgb() const {
        return xyz_to_rgb(to_xyz());
    }

    vec3 to_rgb_v() const {
        auto rgb = to_rgb();
        return {rgb.r, rgb.g, rgb.b};
    }

    static color_spectrum const& get_rgb_base(spectrum_type type, rgb_base base) {
        switch (type) {
            case spectrum_type::reflectance:
                switch(base) {
                    case rgb_base::white: return refl_white();
                    case rgb_base::red: return refl_red();
                    case rgb_base::blue: return refl_blue();
                    case rgb_base::green: return refl_green();
                    case rgb_base::cyan: return refl_cyan();
                    case rgb_base::magenta: return refl_magenta();
                    case rgb_base::yellow: return refl_yellow();
                }
                break;
            case spectrum_type::illuminant:
                switch(base) {
                    case rgb_base::white: return illum_white();
                    case rgb_base::red: return illum_red();
                    case rgb_base::blue: return illum_blue();
                    case rgb_base::green: return illum_green();
                    case rgb_base::cyan: return illum_cyan();
                    case rgb_base::magenta: return illum_magenta();
                    case rgb_base::yellow: return illum_yellow();
                }
                break;
        }
    }

    static color_spectrum from_rgb(vec3 rgb, spectrum_type type) {
        color_spectrum res;
        res.samples.fill(0);
        if (rgb[0] <= rgb[1] && rgb[0] <= rgb[2]) {
            res += rgb[0] * get_rgb_base(type, rgb_base::white);
            if (rgb[1] <= rgb[2]) {
                res += (rgb[1] - rgb[0]) * get_rgb_base(type, rgb_base::cyan);
                res += (rgb[2] - rgb[1]) * get_rgb_base(type, rgb_base::blue);
            } else {
                res += (rgb[2] - rgb[0]) * get_rgb_base(type, rgb_base::cyan);
                res += (rgb[1] - rgb[2]) * get_rgb_base(type, rgb_base::green);
            }
        } else if (rgb[1] <= rgb[0] && rgb[1] <= rgb[2]) {
            res += rgb[1] * get_rgb_base(type, rgb_base::white);
            if (rgb[0] <= rgb[1]) {
                res += (rgb[0] - rgb[1]) * get_rgb_base(type, rgb_base::magenta);
                res += (rgb[2] - rgb[0]) * get_rgb_base(type, rgb_base::blue);
            } else {
                res += (rgb[2] - rgb[1]) * get_rgb_base(type, rgb_base::magenta);
                res += (rgb[0] - rgb[2]) * get_rgb_base(type, rgb_base::red);
            }
        } else if (rgb[2] <= rgb[0] && rgb[2] <= rgb[1]) {
            res += rgb[2] * get_rgb_base(type, rgb_base::white);
            if (rgb[0] <= rgb[2]) {
                res += (rgb[0] - rgb[2]) * get_rgb_base(type, rgb_base::yellow);
                res += (rgb[1] - rgb[0]) * get_rgb_base(type, rgb_base::green);
            } else {
                res += (rgb[1] - rgb[2]) * get_rgb_base(type, rgb_base::yellow);
                res += (rgb[0] - rgb[1]) * get_rgb_base(type, rgb_base::red);
            }
        }
        return res;
    }

    static color_spectrum from_rgb(double r, double g, double b, spectrum_type type = spectrum_type::reflectance) {
        return from_rgb(vec3{r, g, b}, type);
    }

    static color_spectrum from_rgb(color_rgb c, spectrum_type type = spectrum_type::reflectance) {
        return from_rgb(vec3{c.r, c.g, c.b}, type);
    }

    static color_spectrum from_xyz(vec3 xyz, spectrum_type type) {
        return from_rgb(xyz_to_rgb(xyz), type);
    }

    static color_spectrum random() {
        return from_rgb(color_rgb::random());
    }

    static color_spectrum random(double min, double max) {
        return from_rgb(color_rgb::random(min, max));
    }
};

constexpr color_spectrum operator+(color_spectrum const& c, double d) {
    color_spectrum res; for (int i = 0; i < color_spectrum::sample_count; ++i) res[i] = c[i] + d; return res;
}
constexpr color_spectrum operator+(color_spectrum const& c, color_spectrum const& d) {
    color_spectrum res; for (int i = 0; i < color_spectrum::sample_count; ++i) res[i] = c[i] + d[i]; return res;
}
template<size_t sample_count>constexpr color_spectrum operator*(color_spectrum const& c, double d) {
    color_spectrum res; for (int i = 0; i < color_spectrum::sample_count; ++i) res[i] = c[i] * d; return res;
}
constexpr color_spectrum operator*(double d, color_spectrum const& c) {
    color_spectrum res; for (int i = 0; i < color_spectrum::sample_count; ++i) res[i] = c[i] * d; return res;
}
constexpr color_spectrum operator*(color_spectrum const& c, color_spectrum const& d) {
    color_spectrum res; for (int i = 0; i < color_spectrum::sample_count; ++i) res[i] = c[i] * d[i]; return res;
}
constexpr color_spectrum operator/(color_spectrum const& c, double d) {
    color_spectrum res; for (int i = 0; i < color_spectrum::sample_count; ++i) res[i] = c[i] / d; return res;
}

void to_json(json& j, color_spectrum const& c) {
    auto samples = json::array();
    for (int i = 0; i < color_spectrum::sample_count; ++i) {
        samples.push_back(c.samples[i]);
    }
    j = json{
        {"sample_count", color_spectrum::sample_count},
        {"lambda_start", color_spectrum::lambda_start},
        {"lambda_end", color_spectrum::lambda_end},
        {"samples", samples}
    };
}

void from_json(json const& j, color_spectrum& c) {
    double sample_count, lambda_start, lambda_end;
    j.at("sample_count").get_to(sample_count);
    j.at("lambda_start").get_to(lambda_start);
    j.at("lambda_end").get_to(lambda_end);
    auto samples_json = j.at("samples");
    
    if (sample_count == color_spectrum::sample_count && lambda_start == color_spectrum::lambda_start && lambda_end == color_spectrum::lambda_end) {
        for (int i = 0; i < sample_count; ++i) samples_json[i].get_to(c.samples[i]);
    } else {
        std::vector<spectrum_sample> samples;
        for (int i = 0; i < sample_count; ++i) {
            samples.push_back({lambda_start + (i + 0.5) / sample_count * (lambda_end - lambda_start), samples_json[i].get<double>()});
        }
        c = color_spectrum::from_samples(samples);
    }
}

using color = color_spectrum;


#endif