#include <gtest/gtest.h>
#include "sphere.h"

TEST(Sphere, Hit) {
    auto s = sphere(point(0, 0, 0), 1);
    hit_record h;

    auto r = ray(point(-2, 0, 0), vec3(1, 0, 0));
    EXPECT_TRUE(s.hit(r, -10, 10, h));
    EXPECT_EQ(h.t, 1);
    EXPECT_EQ(h.p, point(-1, 0, 0));
    EXPECT_EQ(h.n, vec3(-1, 0, 0));

    r = ray(point(0, 0, 0), vec3(1, 0, 0));
    EXPECT_TRUE(s.hit(r, 0, 10, h));
    EXPECT_EQ(h.t, 1);
    EXPECT_EQ(h.p, point(1, 0, 0));
    EXPECT_EQ(h.n, vec3(-1, 0, 0));

    r = ray(point(0.5, 2, 0), vec3(0, -1, 0));
    EXPECT_TRUE(s.hit(r, -10, 10, h));
}