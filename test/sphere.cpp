#include "test.h"
#include "sphere.h"

TEST(Sphere, Hit) {
    auto s = sphere(point(0, 0, 0), 1);
    hit_record h;

    auto r = ray(point(-2, 0, 0), vec3(1, 0, 0));
    EXPECT_TRUE(s.hit(r, -10, 10, h));
    EXPECT_EQ(h.t, 1);
    EXPECT_EQ(h.p, point(-1, 0, 0));
    EXPECT_EQ(h.n, vec3(-1, 0, 0));
    auto [u, v] = sphere::get_uv(point(-1, 0, 0));
    EXPECT_EQ(u, h.u);
    EXPECT_EQ(v, h.v);

    r = ray(point(0, 0, 0), vec3(1, 0, 0));
    EXPECT_TRUE(s.hit(r, 0, 10, h));
    EXPECT_EQ(h.t, 1);
    EXPECT_EQ(h.p, point(1, 0, 0));
    EXPECT_EQ(h.n, vec3(-1, 0, 0));

    r = ray(point(0.5, 2, 0), vec3(0, -1, 0));
    EXPECT_TRUE(s.hit(r, -10, 10, h));
}

TEST(Sphere, BoundingBox) {
    auto s = sphere(point(0, 0, 0), 1);
    auto exp = aabb(point(-1, -1, -1), point(1, 1, 1));
    auto [res, bb] = s.bounding_box(0, 0);
    EXPECT_TRUE(res);
    EXPECT_EQ(exp, bb);

    auto ms = moving_sphere(point(0, 0, 0), point(1, 1, 1), 0, 1, 1);
    auto [res1, bb1] = sphere(point(0, 0, 0), 1).bounding_box(0, 1);
    auto [res2, bb2] = sphere(point(1, 1, 1), 1).bounding_box(0, 1);
    std::tie(res, bb) = ms.bounding_box(0, 1);
    EXPECT_TRUE(res);
    EXPECT_EQ(surrounding_box(bb1, bb2), bb);
}

void test_uv(point const& p, std::tuple<double, double> exp) {
    auto uv = sphere::get_uv(p);
    EXPECT_EQ(uv, exp);
}

TEST(Sphere, Uv) {
    test_uv(point(1, 0, 0), {0.5, 0.5});
    test_uv(point(0, 1, 0), {0.5, 1});
    test_uv(point(0, 0, 1), {0.25, 0.5});
    test_uv(point(-1, 0, 0), {0, 0.5});
    test_uv(point(0, -1, 0), {0.5, 0});
    test_uv(point(0, 0, -1), {0.75, 0.5});
}