#ifndef UTIL_H
#define UTIL_H

#include <cmath>
#include <numbers>
#include <limits>
#include <random>

static const double half_sqrt_2 = std::sqrt(2) / 2;
static constexpr double inf = std::numeric_limits<double>::infinity();

double radians(double degrees) { return degrees * std::numbers::pi / 180; }

double random_double() {
    static std::mt19937 gen;
    static std::uniform_real_distribution<double> dist(0, 1);
    return dist(gen);
}

double random_double(double min, double max) {
    return min + (max - min) * random_double();
}

double clamp(double x, double min, double max) {
    if (x < min) return min;
    if (x > max) return max;
    return x;
}

#endif