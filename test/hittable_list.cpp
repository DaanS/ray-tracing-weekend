#include "test.h"
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

TEST(HittableList, BoundingBox) {
    hittable_list list;
    auto [res, bb] = list.bounding_box(0, 1);
    EXPECT_FALSE(res);
    EXPECT_EQ(bb, aabb());

    auto s1 = std::make_shared<sphere>(point(0, 0, 0), 1);
    list.add(s1);
    auto [res1, bb1] = s1->bounding_box(0, 1);
    std::tie(res, bb) = list.bounding_box(0, 1);
    EXPECT_EQ(res, res1);
    EXPECT_EQ(bb, bb1);

    auto s2 = std::make_shared<sphere>(point(-1, -1, -1), 2);
    list.add(s2);
    auto [res2, bb2] = s2->bounding_box(0, 1);
    std::tie(res, bb) = list.bounding_box(0, 1);
    EXPECT_EQ(res, res1 && res2);
    EXPECT_EQ(bb, surrounding_box(bb1, bb2));
}