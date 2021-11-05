#ifndef AABB_H
#define AABB_H

#include "vec3.h"
#include "ray.h"

struct aabb {
    point min;
    point max;

    aabb() : min(point(0, 0, 0)), max(point(0, 0, 0)) { }
    aabb(point const& min, point const& max) : min(min), max(max) { }

    bool hit(ray const& r, double t_min, double t_max) const {
        for (int a = 0; a < 3; ++a) {
            if (compare(r.direction[a], 0)) continue; // XXX avoid NaN plague
            auto invD = 1.0 / r.direction[a];
            auto t0 = (min[a] - r.origin[a]) * invD;
            auto t1 = (max[a] - r.origin[a]) * invD;
            if (invD < 0) std::swap(t0, t1);
            t_min = std::max(t0, t_min);
            t_max = std::min(t1, t_max);
            if (t_max <= t_min) return false;
        }
        return true;
    }

    bool operator==(aabb const& rhs) const {
        return min == rhs.min && max == rhs.max;
    }
};

aabb surrounding_box(aabb const& lhs, aabb const& rhs) {
    assert(lhs.min.x < lhs.max.x);
    assert(lhs.min.y < lhs.max.y);
    assert(lhs.min.z < lhs.max.z);
    assert(rhs.min.x < rhs.max.x);
    assert(rhs.min.y < rhs.max.y);
    assert(rhs.min.z < rhs.max.z);
    point small(std::min(lhs.min.x, rhs.min.x), std::min(lhs.min.y, rhs.min.y), std::min(lhs.min.z, rhs.min.z));
    point large(std::max(lhs.max.x, rhs.max.x), std::max(lhs.max.y, rhs.max.y), std::max(lhs.max.z, rhs.max.z));
    return aabb(small, large);
}

#endif