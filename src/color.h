#ifndef COLOR_H
#define COLOR_H

#include <iostream>
#include "util.h"
#include "vec3.h"

void write_color(std::ostream& os, vec3 const& color, size_t samples) {
    auto color_scaled = color * (1.0 / samples);
    os << static_cast<int>(256 * clamp(std::sqrt(color_scaled.x), 0, 0.99999)) << ' '
       << static_cast<int>(256 * clamp(std::sqrt(color_scaled.y), 0, 0.99999)) << ' '
       << static_cast<int>(256 * clamp(std::sqrt(color_scaled.z), 0, 0.99999)) << '\n';
}

color scale_color(vec3 const& color, size_t samples) {
    auto res = color * (1.0 / samples);
    res.x = std::sqrt(res.x);
    res.y = std::sqrt(res.y);
    res.z = std::sqrt(res.z);
    return res;
}

#endif