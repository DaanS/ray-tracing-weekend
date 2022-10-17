#ifndef VEC3_H
#define VEC3_H

#include <cmath>
#include <nlohmann/json.hpp>
#include "util.h"

struct vec3 {
    double x, y, z;

    constexpr vec3() : x(0), y(0), z(0) {}
    constexpr vec3(double x, double y, double z) : x(x), y(y), z(z) {}

    constexpr vec3 operator-() const { return vec3{-x, -y, -z}; }
    double& operator[](int i) {
        if (i == 0) return x;
        else if (i == 1) return y;
        return z;
    }
    double operator[](int i) const { 
        if (i == 0) return x;
        else if (i == 1) return y;
        return z;
    }

    // TODO make constexpr in c++23
    bool operator==(vec3 const& rhs) const {
        return compare(x, rhs.x) && compare(y, rhs.y) && compare(z, rhs.z);
    }

    vec3& operator+=(vec3 const& rhs) {
        x += rhs.x;
        y += rhs.y;
        z += rhs.z;
        return *this;
    }

    vec3& operator*=(double d) {
        x *= d;
        y *= d;
        z *= d;
        return *this;
    }

    double length() const { return std::sqrt(length_squared()); }
    constexpr double length_squared() const {
        return x * x + y * y + z * z;
    }

    bool near_zero() {
        return compare(x, 0) && compare(y, 0) && compare(z, 0);
    }

    static vec3 random() {
        return {random_double(), random_double(), random_double()};
    }

    static vec3 random(double min, double max) {
        return {random_double(min, max), random_double(min, max), random_double(min, max)};
    }
};

std::ostream& operator<<(std::ostream& os, vec3 const& v) {
    os << "{" << v.x << ", " << v.y << ", " << v.z << "}";
    return os;
}

constexpr vec3 operator+(vec3 const& u, vec3 const& v) { return {u.x + v.x, u.y + v.y, u.z + v.z}; }
constexpr vec3 operator+(vec3 const& u, double d) { return {u.x + d, u.y + d, u.z + d}; }
constexpr vec3 operator+(double d, vec3 const& u) { return {u.x + d, u.y + d, u.z + d}; }
constexpr vec3 operator-(vec3 const& u, vec3 const& v) { return {u.x - v.x, u.y - v.y, u.z - v.z}; }
constexpr vec3 operator*(vec3 const& u, vec3 const& v) { return {u.x * v.x, u.y * v.y, u.z * v.z}; }
constexpr vec3 operator*(double d, vec3 const& v) { return {v.x * d, v.y * d, v.z * d}; }
constexpr vec3 operator*(vec3 const& v, double d) { return d * v; }
constexpr vec3 operator/(vec3 const& v, double d) { return (1 / d) * v; }

constexpr vec3 normalize(vec3 const& v) { return v / v.length(); }

constexpr double dot(vec3 const& u, vec3 const& v) {
    return u.x * v.x
         + u.y * v.y
         + u.z * v.z;
}

constexpr vec3 cross(vec3 const& u, vec3 const& v) {
    return vec3(u.y * v.z - u.z * v.y,
                u.z * v.x - u.x * v.z,
                u.x * v.y - u.y * v.x);
}

constexpr vec3 reflect(vec3 const& v, vec3 const& n) {
    return v - 2 * dot(v, n) * n;
}

vec3 refract(vec3 const& v, vec3 const&n, double etai_over_etat) {
    auto cos_theta = std::fmin(dot(-v, n), 1);
    vec3 r_out_perp = etai_over_etat * (v + cos_theta * n);
    vec3 r_out_parallel = -std::sqrt(std::fabs(1 - r_out_perp.length_squared())) * n;
    return r_out_perp + r_out_parallel;
}

vec3 vec3_random(double min, double max) {
    return vec3(random_double(min, max), random_double(min, max), random_double(min, max));
}

vec3 vec3_random_sphere() {
    auto v = vec3_random(-1, 1);
    while (v.length_squared() >= 1) v = vec3_random(-1, 1);
    return v;
}

vec3 vec3_random_disk() {
    auto v = vec3(random_double(-1, 1), random_double(-1, 1), 0);
    while (v.length_squared() >= 1) v = vec3(random_double(-1, 1), random_double(-1, 1), 0);
    return v;
}

vec3 vec3_random_unit() {
    return normalize(vec3_random_sphere());
}

vec3 vec3_random_hemisphere(vec3 const& n) {
    auto in_sphere = vec3_random_sphere();
    if (dot(in_sphere, n) > 0) return in_sphere;
    else return -in_sphere;
}

vec3 vec3_random_cosine() {
    auto r1 = random_double();
    auto r2 = random_double();
    auto z = sqrt(1 - r2);

    auto phi = 2 * pi * r1;
    auto x = cos(phi) * sqrt(r2);
    auto y = sin(phi) * sqrt(r2);

    return {x, y, z};
}

using color = vec3;
using point = vec3;

using nlohmann::json;
void to_json(json& j, vec3 const& v) {
    j = json{
        {"x", v.x},
        {"y", v.y},
        {"z", v.z}
    };
}
void from_json(json const& j, vec3& v) {
    j.at("x").get_to(v.x);
    j.at("y").get_to(v.y);
    j.at("z").get_to(v.z);
}

#endif