#ifndef COLOR_H
#define COLOR_H

#include <iostream>
#include "spectrum.h"
#include "util.h"
#include "vec3.h"

color_rgb scale_color(color_rgb const& color, size_t samples) {
    auto res = color * (1.0 / samples);
    auto gamma = 2.1;
    res.r = std::pow(clamp(res.r, epsilon, 1.0 - epsilon), 1.0 / gamma);
    res.g = std::pow(clamp(res.g, epsilon, 1.0 - epsilon), 1.0 / gamma);
    res.b = std::pow(clamp(res.b, epsilon, 1.0 - epsilon), 1.0 / gamma);
    return res;
}

#endif