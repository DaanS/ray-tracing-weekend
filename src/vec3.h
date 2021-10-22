#ifndef VEC3_H
#define VEC3_H

#include <cmath>

struct vec3 {
    double x, y, z;

    constexpr vec3(double x, double y, double z) : x(x), y(y), z(z) {}

    vec3 operator-() const { return vec3{-x, -y, -z}; }
    double& operator[](int i) {
        if (i == 0) return x;
        else if (i == 1) return y;
        return z;
    }
    double operator[](int i) const { return (*this)[i]; }

    constexpr bool operator==(vec3 const& rhs) const {
        return x == rhs.x && y == rhs.y && z == rhs.z;
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

    constexpr double length() const { return std::sqrt(length_squared()); }
    constexpr double length_squared() const {
        return x * x + y * y + z * z;
    }
};

std::ostream& operator<<(std::ostream& os, vec3 const& v) {
    os << "{" << v.x << ", " << v.y << ", " << v.z << "}";
    return os;
}

constexpr vec3 operator+(vec3 const& u, vec3 const& v) { return {u.x + v.x, u.y + v.y, u.z + v.z}; }
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

using color = vec3;
using point = vec3;

#endif