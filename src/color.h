#ifndef COLOR_H
#define COLOR_H

#include <iostream>
#include "spectrum.h"
#include "util.h"
#include "vec3.h"

void write_color(std::ostream& os, color const& color, size_t samples) {
    auto color_scaled = color * (1.0 / samples);
    os << static_cast<int>(256 * clamp(std::sqrt(color_scaled.r), epsilon, 0.99999)) << ' '
       << static_cast<int>(256 * clamp(std::sqrt(color_scaled.g), epsilon, 0.99999)) << ' '
       << static_cast<int>(256 * clamp(std::sqrt(color_scaled.b), epsilon, 0.99999)) << '\n';
}

color scale_color(color const& color, size_t samples) {
    auto res = color * (1.0 / samples);
    auto gamma = 2.1;
    res.r = std::pow(res.r, 1.0 / gamma);
    res.g = std::pow(res.g, 1.0 / gamma);
    res.b = std::pow(res.b, 1.0 / gamma);
    return res;
}

#endif