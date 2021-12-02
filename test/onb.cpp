#include <gtest/gtest.h>
#include "onb.h"

TEST(Onb, Basic) {
    onb from_z(vec3(0, 0, 1));
    vec3 v(2, 3, 4);
    EXPECT_EQ(v, from_z.local(v));

    onb from_whatevs(vec3(5, 3, 16));
    EXPECT_TRUE(compare(v.length(), from_whatevs.local(v).length()));

    EXPECT_TRUE(compare(dot(from_whatevs.u, from_whatevs.v), 0));
    EXPECT_TRUE(compare(dot(from_whatevs.u, from_whatevs.w), 0));
    EXPECT_TRUE(compare(dot(from_whatevs.v, from_whatevs.w), 0));
    EXPECT_TRUE(compare(from_whatevs.u.length(), 1));
    EXPECT_TRUE(compare(from_whatevs.v.length(), 1));
    EXPECT_TRUE(compare(from_whatevs.w.length(), 1));
}