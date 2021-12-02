#ifndef ONB_H
#define ONB_H

#include "vec3.h"

struct onb {
    vec3 u;
    vec3 v;
    vec3 w;

    onb(vec3 const& n) {
        w = normalize(n);
        auto a = (std::abs(w.x) > 0.9) ? vec3(0, 1, 0) : vec3(1, 0, 0);
        v = normalize(cross(w, a));
        u = -cross(w, v);
    }

    constexpr vec3 operator[](int i) const {
        if (i == 0) return u;
        else if (i == 1) return v;
        return w;
    }

    constexpr vec3 local(vec3 const& a) const {
        return a.x * u + a.y * v + a.z * w;
    }
};

#endif