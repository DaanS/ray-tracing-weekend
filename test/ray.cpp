#include <gtest/gtest.h>
#include "vec3.h"
#include "ray.h"

TEST(Ray, At) {
    auto r = ray(point(0, 0, 0), vec3(1, 0, 0));
    EXPECT_EQ(r.at(0), point(0, 0, 0));
    EXPECT_EQ(r.at(2), point(2, 0, 0));
    EXPECT_EQ(r.at(-1.5), point(-1.5, 0, 0));
}