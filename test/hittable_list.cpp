#include <gtest/gtest.h>
#include "hittable_list.h"
#include "sphere.h"

TEST(HittableList, Basic) {
    auto s1 = std::make_shared<sphere>(point(-1, 0, 0), 2);
    auto s2 = std::make_shared<sphere>(point(1, 0, 0), 2);
    hittable_list list;
    list.add(s1);
    list.add(s2);
    hit_record h;
    auto r = ray(point(-5, 0, 0), vec3(1, 0, 0));
    EXPECT_TRUE(list.hit(r, -10, 10, h));
    EXPECT_EQ(h.t, 2);
    EXPECT_EQ(h.p, point(-3, 0, 0));
    EXPECT_EQ(h.n, vec3(-1, 0, 0));
    EXPECT_TRUE(h.front_face);

    r = ray(point(0, 0, 0), vec3(1, 0, 0));
    EXPECT_TRUE(list.hit(r, 0, 10, h));
    EXPECT_EQ(h.t, 1);
    EXPECT_EQ(h.p, point(1, 0, 0));
    EXPECT_EQ(h.n, vec3(-1, 0, 0));
    EXPECT_FALSE(h.front_face);

    r = ray(point(5, 0, 0), vec3(-1, 0, 0));
    EXPECT_TRUE(list.hit(r, 0, 10, h));
    EXPECT_EQ(h.t, 2);
    EXPECT_EQ(h.p, point(3, 0, 0));
    EXPECT_EQ(h.n, vec3(1, 0, 0));
    EXPECT_TRUE(h.front_face);
}