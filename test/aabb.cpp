#include <gtest/gtest.h>
#include "aabb.h"
#include "util.h"

TEST(Aabb, Basic) {
    aabb bb(point(0, 0, 0), point(1, 1, 1));
    EXPECT_TRUE(bb.hit(ray(point(0, 0, -1), vec3(0, 0, 1)), 0, inf));
    EXPECT_FALSE(bb.hit(ray(point(0, 0, 2), vec3(0, 0, 1)), 0.0, inf));
    EXPECT_TRUE(bb.hit(ray(point(0, 0, 2), vec3(0, 0, 1)), -inf, inf));
    EXPECT_TRUE(bb.hit(ray(point(0, 0, 2), vec3(0, 0, -1)), 0, inf));
    EXPECT_TRUE(bb.hit(ray(point(0.5, 0.5, 0.5), vec3(1, 1, 1)), 0, inf));
    EXPECT_FALSE(bb.hit(ray(point(0.5, 0.5, 5), vec3(1, 1, 1)), 0, inf));
}