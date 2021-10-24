#include <gtest/gtest.h>
#include "material.h"
#include "sphere.h"
#include "hittable.h"
#include "ray.h"

TEST(Material, Lambertian) {
    point origin(0, 0, 0);
    auto mat = std::make_shared<lambertian>(color(1, 0.5, 0));
    sphere s(point(2, 0, 0), 1, mat);
    hit_record h;
    ray r(origin, s.center - origin);
    EXPECT_TRUE(s.hit(r, 0, inf, h));
    auto [res, attenuation, scattered] = h.mat_ptr->scatter(r, h);
    EXPECT_TRUE(res);
    EXPECT_EQ(attenuation, color(1, 0.5, 0));
    EXPECT_DOUBLE_EQ((scattered.direction - h.n).length(), 1);
}

TEST(Material, Metal) {
    point origin(0, 0, 0);
    auto mat = std::make_shared<metal>(color(1, 0.5, 0));
    sphere s(point(2, 0, 0), 1, mat);
    hit_record h;
    ray r(origin, s.center - origin);
    EXPECT_TRUE(s.hit(r, 0, inf, h));
    auto [res, attenuation, scattered] = h.mat_ptr->scatter(r, h);
    EXPECT_TRUE(res);
    EXPECT_EQ(attenuation, color(1, 0.5, 0));
    EXPECT_EQ((scattered.direction).length(), 1);
    EXPECT_EQ(dot(-normalize(r.direction), h.n), dot(scattered.direction, h.n));
}