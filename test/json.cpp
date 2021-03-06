#include "test.h"
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

    auto checkers = checker(color(0, 0, 0), color(1, 1, 1));
    auto checkers_res = texture::make_from_json(json(checkers));
    EXPECT_EQ(*checkers_res, checkers);

    auto noise_tex = noise(color(0, 0, 0), color(1, 1, 1), 3);
    auto noise_res = texture::make_from_json(json(noise_tex));
    EXPECT_EQ(*noise_res, noise_tex);

    auto image_tex = image("../res/earthmap.jpg");
    auto image_res = texture::make_from_json(json(image_tex));
    EXPECT_EQ(*image_res, image_tex);
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

    auto light_mat = diffuse_light(color(1, 1, 1));
    auto light_res = material::make_from_json(json(light_mat));
    EXPECT_EQ(*light_res, light_mat);
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

    auto sphere_res = hittable::make_from_json(j2);
    EXPECT_EQ(s, *sphere_res);
}

template<typename Hittable, typename... Args>
void test_json(Args... args) {
    auto h = Hittable(std::forward<Args>(args)...);
    auto h_res = hittable::make_from_json(json(h));
    EXPECT_EQ(*h_res, h);
}

TEST(Json, Hittables) {
    auto mat = std::make_shared<lambertian>(color(1, 1, 1));
    test_json<sphere>(point(0, 0, 0), 1, mat);
    test_json<moving_sphere>(point(0, 0, 0), point(1, 1, 1), 0, 1, 2, mat);
    auto child = std::make_shared<sphere>(point(0, 0, 0), 1);
    test_json<translate>(child, vec3(0, 0, 0));
    test_json<rotate_y>(child, 15);
    hittable_list world;
    world.add(child);
    test_json<bvh_node>(world, 0, 1);
    test_json<xz_rect>(0, 1, 0, 1, 1, mat);
    test_json<box>(point(0, 0, 0), point(1, 1, 1), mat);
    test_json<constant_medium>(child, 0.1, color(1, 1, 1));
}