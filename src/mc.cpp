#include <iostream>
#include <iomanip>
#include "util.h"
#include "vec3.h"

vec3 random_unit_sphere() {
    auto r1 = random_double();
    auto r2 = random_double();

    auto x = cos(2 * pi * r1) * 2 * sqrt(r2 * (1 - r2));
    auto y = sin(2 * pi * r1) * 2 * sqrt(r2 * (1 - r2));
    auto z = 1 - 2 * r2;

    return {x, y, z};
}

vec3 random_unit_hemisphere() {
    auto r1 = random_double();
    auto r2 = random_double();

    auto x = cos(2 * pi * r1) * 2 * sqrt(r2 * (1 - r2));
    auto y = sin(2 * pi * r1) * 2 * sqrt(r2 * (1 - r2));
    auto z = 1 - r2;

    return {x, y, z};
}

vec3 random_cosine() {
    auto r1 = random_double();
    auto r2 = random_double();
    auto z = sqrt(1 - r2);

    auto phi = 2 * pi * r1;
    auto x = cos(phi) * sqrt(r2);
    auto y = sin(phi) * sqrt(r2);

    return {x, y, z};
}

int main() {
    static constexpr int n = 1000000;
    double sum = 0;
    for (int i = 0; i < n; ++i) {
        auto v = random_cosine();
        sum += v.z * v.z * v.z / (v.z / pi);
    }
    std::cout << std::fixed << std::setprecision(12);
    std::cout << pi / 2 << std::endl;
    std::cout << sum / n << std::endl;
}