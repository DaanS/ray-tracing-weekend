#ifndef UTIL_H
#define UTIL_H

#include <cmath>
#include <numbers>
#include <limits>
#include <random>

static const double half_sqrt_2 = std::sqrt(2) / 2;
static constexpr double inf = std::numeric_limits<double>::infinity();

double radians(double degrees) { return degrees * std::numbers::pi / 180; }

template<size_t Size>
struct random_jar {
    std::array<double, Size> rng;

    random_jar() {
        static std::minstd_rand gen;
        static std::uniform_real_distribution<double> dist(0, 1);
        for (int i = 0; i < Size; ++i) {
            rng[i] = dist(gen);
        }
    }

    double get() {
        static int i = 0;
        i = (i + 1) % Size;
        return rng[i];
    }
};

double random_double() {
    //static std::mt19937 gen;
    //static std::minstd_rand gen;
    //static std::uniform_real_distribution<double> dist(0, 1);
    //return dist(gen);

    static constexpr size_t rng_size = 1000000;
    static random_jar<rng_size> jar;
    return jar.get();
}

double random_double(double min, double max) {
    return min + (max - min) * random_double();
}

double clamp(double x, double min, double max) {
    if (x < min) return min;
    if (x > max) return max;
    return x;
}

static constexpr double epsilon = 0.000001;

bool compare(double a, double b, double e) { return std::abs(a - b) < epsilon; }
bool compare(double a, double b) { return compare(a, b, epsilon); }

#endif