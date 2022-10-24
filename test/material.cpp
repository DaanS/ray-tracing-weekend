#include "test.h"
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
    auto [res, scat] = h.mat_ptr->scatter(r, h);
    EXPECT_TRUE(res);
    EXPECT_EQ(scat.attenuation, color(1, 0.5, 0));
    auto scattered = scat.pdf_ptr->generate();
    EXPECT_DOUBLE_EQ(scattered.length(), 1);
    EXPECT_EQ(scat.pdf_ptr->value(scattered), h.mat_ptr->scattering_pdf(r, h, ray(h.p, scattered)));
}

TEST(Material, Metal) {
    point origin(0, 0, 0);
    auto mat = std::make_shared<metal>(color(1, 0.5, 0));
    sphere s(point(2, 0, 0), 1, mat);
    hit_record h;
    ray r(origin, s.center - origin);
    EXPECT_TRUE(s.hit(r, 0, inf, h));
    auto [res, scat] = h.mat_ptr->scatter(r, h);
    EXPECT_TRUE(res);
    EXPECT_EQ(scat.attenuation, color(1, 0.5, 0));
    EXPECT_TRUE(scat.is_specular);
    EXPECT_EQ(scat.specular.direction.length(), 1);
    EXPECT_EQ(dot(-normalize(r.direction), h.n), dot(scat.specular.direction, h.n));

    //ray scattered(h.p, scat.pdf_ptr->generate());
    //EXPECT_EQ((scattered.direction).length(), 1);
    //EXPECT_EQ(dot(-normalize(r.direction), h.n), dot(scattered.direction, h.n));
    //EXPECT_EQ(scat.pdf_ptr->value(scattered.direction), h.mat_ptr->scattering_pdf(r, h, scattered));

    auto fuzz = 0.5;
    auto mat2 = std::make_shared<metal>(color(1, 0.5, 0), fuzz);
    sphere s2(point(2, 0, 0), 1, mat2);
    EXPECT_TRUE(s2.hit(r, 0, inf, h));
    auto [res2, scat2] = h.mat_ptr->scatter(r, h);
    EXPECT_LE((-normalize(r.direction) - scat.specular.direction).length(), fuzz);
}

TEST(Material, Dielectric) {
    point origin(0, 0, 0);
    auto mat = std::make_shared<dielectric>(1.5);
    sphere s(point(2, 0, 0), 1, mat);
    hit_record h;
    ray r(origin, s.center - origin);
    EXPECT_TRUE(s.hit(r, 0, inf, h));
    auto [res, scat] = h.mat_ptr->scatter(r, h);
    EXPECT_TRUE(res);
    EXPECT_EQ(scat.attenuation, color(1));
    EXPECT_TRUE(scat.is_specular);
    EXPECT_GT((scat.specular.direction - h.n).length(), 1);
    //ray scattered(h.p, scat.pdf_ptr->generate());
    //EXPECT_GT((scattered.direction - h.n).length(), 1);
    //EXPECT_EQ(scat.pdf_ptr->value(scattered.direction), h.mat_ptr->scattering_pdf(r, h, scattered));
}

TEST(Material, Equality) {
    auto l1 = lambertian(color(1, 1, 1));
    auto l2 = lambertian(color(0, 0, 0));
    EXPECT_EQ(l1, l1);
    EXPECT_NE(l1, l2);

    auto m1 = metal(color(1, 1, 1), 0.1);
    EXPECT_EQ(m1, m1);
    EXPECT_NE(l1, m1);

    auto d1 = dielectric(1.5);
    EXPECT_EQ(d1, d1);
    EXPECT_NE(d1, l1);
}