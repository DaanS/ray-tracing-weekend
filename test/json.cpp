#include <gtest/gtest.h>
#include <nlohmann/json.hpp>
#include "sphere.h"
#include "texture.h"
#include "material.h"

using nlohmann::json;

TEST(Json, Texture) {
    auto tex1 = solid_color(0, 0, 0);
    json j1 = {
        {"type", "solid_color"},
        {"albedo", {
            {"x", tex1.c.x},
            {"y", tex1.c.y},
            {"z", tex1.c.z}
        }}
    };
    EXPECT_EQ(json(tex1), j1);
    auto res = texture::make_from_json(json(tex1));
    EXPECT_EQ(*res, tex1);

    auto tex2 = solid_color(1, 1, 1);
    auto checkers = checker(color(0, 0, 0), color(1, 1, 1));
    auto checkers_res = texture::make_from_json(json(checkers));
    EXPECT_EQ(*checkers_res, checkers);
}

TEST(Json, Material) {
    auto mat1 = lambertian(color(1, 1, 1));
    json mat1_json = {
        {"type", "lambertian"},
        {"albedo", {
            {"type", "solid_color"},
            {"albedo", {
                {"x", 1},
                {"y", 1},
                {"z", 1}
            }}
        }}
    };
    EXPECT_EQ(json(mat1), mat1_json);
    auto mat1_res = material::make_from_json(json(mat1));
    EXPECT_EQ(*mat1_res, mat1);

    auto mat2 = metal(color(1, 1, 1), 0.1);
    auto mat2_res = material::make_from_json(json(mat2));
    EXPECT_EQ(*mat2_res, mat2);

    auto mat3 = dielectric(1.5);
    auto mat3_res = material::make_from_json(json(mat3));
    EXPECT_EQ(*mat3_res, mat3);
}

TEST(Json, Sphere) {
    auto s = sphere(point(0, 0, 0), 1);

    json j = {
        {"type", "sphere"},
        {"radius", s.radius},
        {"center", {
            {"x", s.center.x},
            {"y", s.center.y},
            {"z", s.center.z}
        }},
        {"material", s.mat_ptr->to_json()}
    };
    json j2 = s;
    EXPECT_EQ(j, j2);

    sphere s2;
    j2.get_to(s2);
    EXPECT_EQ(s, s2);
}