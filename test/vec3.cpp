#include <gtest/gtest.h>
#include <numbers>
#include "vec3.h"

using namespace std::numbers;

TEST(Vec3, Basics) {
    auto v = vec3{1, 2, 3};
    EXPECT_EQ(-v, vec3(-1, -2, -3));
    EXPECT_EQ(v[0], 1);
    EXPECT_EQ(v[1], 2);
    EXPECT_EQ(v[2], 3);

    auto v2 = vec3{3, 2, 1};
    v2 += v;
    EXPECT_EQ(v2, vec3(4, 4, 4));

    auto v3 = vec3{2, 1, 3};
    v3 *= 3;
    EXPECT_EQ(v3, vec3(6, 3, 9));

    EXPECT_EQ(v.length_squared(), 14);
    EXPECT_EQ(v3.length(), std::sqrt(36 + 9 + 81));

    auto u = vec3{2, 3, 4};
    EXPECT_EQ(v + u, vec3(3, 5, 7));
    EXPECT_EQ(u - v, vec3(1, 1, 1));
    EXPECT_EQ(v * u, vec3(2, 6, 12));
    EXPECT_EQ(u / 2, vec3(1, 1.5, 2));
    EXPECT_EQ(3 * v, vec3(3, 6, 9));
    EXPECT_EQ(normalize(u).length(), 1);
}

TEST(Vec3, Dot) {
    auto v = vec3(1, 2, 3);
    EXPECT_DOUBLE_EQ(dot(v, v), v.length_squared());

    auto v_x = vec3(1, 0, 0);
    auto v_y = vec3(0, 1, 0);
    auto v_half = normalize(vec3(1, 1, 0));
    EXPECT_DOUBLE_EQ(dot(v_x, v_x), 1);
    EXPECT_DOUBLE_EQ(dot(v_x, v_y), 0);
    EXPECT_DOUBLE_EQ(dot(v_x, -v_x), -1);
    EXPECT_DOUBLE_EQ(dot(v_x, v_half), std::cos(pi / 4));
}

TEST(Vec3, Cross) {
    auto v_x = vec3(1, 0, 0);
    auto v_y = vec3(0, 1, 0);
    EXPECT_EQ(cross(v_x, v_y), vec3(0, 0, 1));
    EXPECT_EQ(cross(v_y, v_x), vec3(0, 0, -1));
}
