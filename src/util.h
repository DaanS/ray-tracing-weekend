#ifndef UTIL_H
#define UTIL_H

#include <cmath>
#include <numbers>
#include <limits>

static const double half_sqrt_2 = std::sqrt(2) / 2;

static constexpr double inf = std::numeric_limits<double>::infinity();

double radians(double degrees) {
    return degrees * std::numbers::pi / 180;
}

#endif