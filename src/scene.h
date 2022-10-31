#ifndef SCENE_H
#define SCENE_H

#include <fmt/core.h>
#include <fstream>
#include <nlohmann/json.hpp>
#include "aarect.h"
#include "box.h"
#include "bvh.h"
#include "camera.h"
#include "hittable.h"
#include "hittable_list.h"
#include "sphere.h"
#include "volume.h"

struct scene;
void to_json(json& j, scene const& s);
void from_json(json const& j, scene& s);
struct scene {
    hittable_list world;
    camera cam;
    std::shared_ptr<hittable> lights;

    bool operator==(scene const& rhs) const {
        // XXX remove this awful nullptr at some point
        if (lights && !rhs.lights) return false;
        if (!lights && rhs.lights) return false;
        return world == rhs.world && cam == rhs.cam && (lights == rhs.lights || *lights == *rhs.lights);
    }

    void save(std::string path) {
        json j = *this;
        std::ofstream ofs(path);
        ofs << j;
        ofs.close();
    }

    static scene load(std::string path) {
        json j;
        scene s;

        std::ifstream ifs(path);
        ifs >> j;
        j.get_to(s);
        return s;
    }
};

void to_json(json& j, scene const& s) {
    j = json{
        {"world", s.world},
        {"camera", s.cam},
        (s.lights ? json{"lights", *s.lights} : json{"lights", nullptr})
    };
}

void from_json(json const& j, scene& s) {
    j.at("world").get_to(s.world);
    j.at("camera").get_to(s.cam);
    //if (j.at("lights").is_null()) s.lights = nullptr;
    //else s.lights = hittable::make_from_json(j.at("lights"));
    s.lights = j.at("lights").is_null() ? nullptr : hittable::make_from_json(j.at("lights"));
}

scene four_sphere_scene() {
    // world
    hittable_list world;
    auto mat_ground = std::make_shared<lambertian>(color(0.8, 0.8, 0.0));
    //auto mat_ground = std::make_shared<lambertian>(std::make_shared<noise>(color(0, 1, 1), color(1, 0, 1), 2));
    auto mat_center = std::make_shared<lambertian>(color(0.7, 0.3, 0.3));
    auto mat_left = std::make_shared<dielectric>(1.5);
    auto mat_right = std::make_shared<metal>(color(0.8, 0.6, 0.2), 0.0);
    world.make<sphere>(point(0, -100.5, -1), 100, mat_ground);
    world.make<sphere>(point(0, 0, -1), 0.5, mat_center);
    //world.make<moving_sphere>(point(0, 0, -1), point(0, 0.2, -1), 0, 1, 0.5, mat_center);
    world.make<sphere>(point(-1, 0, -1), 0.5, mat_left);
    world.make<sphere>(point(-1, 0, -1), -0.45, mat_left);
    world.make<sphere>(point(1, 0, -1), 0.5, mat_right);
    //world.make<moving_sphere>(point(1, 0, -1), point(1, 0.2, -1), 0, 1, 0.5, mat_right);

    // camera
    point from(0, 0, 0);
    point to(0, 0, -1);
    vec3 up(0, 1, 0);
    camera cam(from, to, up, 90, 16.0 / 9.0, 0, (from - to).length(), 0, 1);

    return {world, cam, nullptr};
}

scene macbeth_spd() {
    hittable_list world;
    static std::array<color_spectrum, 24> macbeth_spds;
    static std::array<color_rgb, 24> macbeth_rgbs;
    for (int i = 0; i < macbeth_spds.max_size(); ++i) {
        macbeth_spds[i] = color_spectrum::from_pbrt(fmt::format("/home/schuld/projects/other/pbrt-v3-scenes/spds/macbeth-{}.spd", i + 1));
        macbeth_rgbs[i] = macbeth_spds[i].to_rgb();
        //fmt::print("macbeth-{}: ({}, {}, {})\n", i + 1, macbeth_rgbs[i].r, macbeth_rgbs[i].g, macbeth_rgbs[i].b);
    }

    for (int y = 0; y < 4; ++y) {
        for (int x = 0; x < 6; ++x) {
            if constexpr (std::is_same_v<color, color_rgb>) {
                auto mat = std::make_shared<lambertian>(macbeth_rgbs[x + 6 * y]);
                world.make<sphere>(point(-3.125 + 1.25 * x, 1.875 - 1.25 * y, 0), 0.5, mat);
            } else {
                auto mat = std::make_shared<lambertian>(macbeth_spds[x + 6 * y]);
                world.make<sphere>(point(-3.125 + 1.25 * x, 1.875 - 1.25 * y, 0), 0.5, mat);
            }
        }
    }
    //world.make<sphere>(point(0, 0, -1000.5), 1000, std::make_shared<lambertian>(color(0.1)));
    auto ground_col = color_spectrum::get_rgb_base(spectrum_type::reflectance, rgb_base::white) * 0.1;
    world.make<sphere>(point(0, 0, -1000.5), 1000, std::make_shared<lambertian>(ground_col));

    point from(0, 0, 7.5);
    point to(0, 0, 0);
    vec3 up(0, 1, 0);
    //point from(4, -4.5, 2.5);
    //point to(0, 1, -1.0);
    //vec3 up(0, 0, 1);
    camera cam(from, to, up, 40, 16.0 / 9.0, 0, (from - to).length(), 0, 1);

    return {world, cam, nullptr};
}

scene random_scene_noise() {
    hittable_list world;
    //auto ground_mat = std::make_shared<lambertian>(color(0.8, 0.8, 0));
    auto ground_mat = std::make_shared<lambertian>(std::make_shared<noise>(color(0.1, 0.9, 0.9), color(0.9, 0.1, 0.9), 2));
    world.make<sphere>(point(0, -1000, 0), 1000, ground_mat);

    for (int a = -11; a < 11; ++a) {
        for (int b = -11; b < 11; ++b) {
            auto mat_rng = random_double();
            point center(a + 0.9 * random_double(), 0.2, b + 0.9 * random_double());

            if ((center - point(4, 0.2, 0)).length() > 0.9) {
                std::shared_ptr<material> mat;
                if (mat_rng < 0.7) {
                    auto albedo = color::random();
                    world.make<sphere>(center, 0.2, std::make_shared<lambertian>(albedo));
                } else if (mat_rng < 0.8) {
                    auto c1 = color::random();
                    auto c2 = color::random();
                    auto scale = random_double(2, 4);
                    auto tex = std::make_shared<noise>(c1, c2, scale);
                    world.make<sphere>(center, 0.2, std::make_shared<lambertian>(tex));
                } else if (mat_rng < 0.95) {
                    auto albedo = color::random(0.5, 1);
                    world.make<sphere>(center, 0.2, std::make_shared<metal>(albedo, random_double(0, 0.2)));
                } else {
                    world.make<sphere>(center, 0.2, std::make_shared<dielectric>(1.5));
                }
            }
        }
    }

    world.make<sphere>(point(0, 1, 0), 1, std::make_shared<dielectric>(1.5));
    world.make<sphere>(point(-4, 1, 0), 1, std::make_shared<lambertian>(color(0.7, 0.3, 0.3)));
    world.make<sphere>(point(4, 1, 0), 1, std::make_shared<metal>(color(0.7, 0.6, 0.5)));

    // camera
    point from(13, 2, 3);
    point to(0, 0, 0);
    vec3 up(0, 1, 0);
    auto focus_dist = 10.0;
    auto aperture = 0.1;
    camera cam(from, to, up, 20, 16.0 / 9.0, aperture, focus_dist, 0, 0);

    return {world, cam, nullptr};
}

scene random_scene_og() {
    hittable_list world;
    auto ground_mat = std::make_shared<lambertian>(color(0.8, 0.8, 0));
    world.make<sphere>(point(0, -1000, 0), 1000, ground_mat);

    for (int a = -11; a < 11; ++a) {
        for (int b = -11; b < 11; ++b) {
            auto mat_rng = random_double();
            point center(a + 0.9 * random_double(), 0.2, b + 0.9 * random_double());

            if ((center - point(4, 0.2, 0)).length() > 0.9) {
                std::shared_ptr<material> mat;
                if (mat_rng < 0.8) {
                    auto albedo = color::random();
                    world.make<sphere>(center, 0.2, std::make_shared<lambertian>(albedo));
                } else if (mat_rng < 0.95) {
                    auto albedo = color::random(0.5, 1);
                    world.make<sphere>(center, 0.2, std::make_shared<metal>(albedo, random_double(0, 0.2)));
                } else {
                    world.make<sphere>(center, 0.2, std::make_shared<dielectric>(1.5));
                }
            }
        }
    }

    world.make<sphere>(point(0, 1, 0), 1, std::make_shared<dielectric>(1.5));
    world.make<sphere>(point(-4, 1, 0), 1, std::make_shared<lambertian>(color(0.7, 0.3, 0.3)));
    world.make<sphere>(point(4, 1, 0), 1, std::make_shared<metal>(color(0.7, 0.6, 0.5)));

    // camera
    point from(13, 2, 3);
    point to(0, 0, 0);
    vec3 up(0, 1, 0);
    auto focus_dist = 10.0;
    auto aperture = 0.1;
    camera cam(from, to, up, 20, 16.0 / 9.0, aperture, focus_dist, 0, 0);

    return {world, cam, nullptr};
}

scene simple_light() {
    hittable_list world;
    auto perlin_tex = std::make_shared<noise>(color(0, 0, 0), color(1, 1, 1), 4);
    auto perlin_mat = std::make_shared<lambertian>(perlin_tex);
    auto ground_mat = std::make_shared<lambertian>(std::make_shared<noise>(color(0.9, 0.9, 0.1), color(0.9, 0.1, 0.9), 2));
    world.make<sphere>(point(0, -1000, 0), 1000, perlin_mat);
    world.make<sphere>(point(0, 2, 0), 2, perlin_mat);

    //auto diff_light = std::make_shared<diffuse_light>(color(0, 4, 4));
    //world.make<xy_rect>(3, 5, 1, 3, -2, diff_light);
    auto light_tex = std::make_shared<noise>(color(1, 1, 0.6), color(0.9, 0.3, 0.2), 4);
    auto diff_light = std::make_shared<diffuse_light>(light_tex);
    world.make<sphere>(point(-4, 5, -3), 2, diff_light);

    point from(26, 3, 6);
    point to(0, 2, 0);
    vec3 up(0, 1, 0);
    //auto focus_dist = 10.0;
    auto focus_dist = (from - to).length();
    auto aperture = 0.1;
    camera cam(from, to, up, 20, 16.0 / 9.0, aperture, focus_dist, 0, 0);

    return {world, cam, nullptr};
}

scene cornell_box() {
    hittable_list world;

    auto red   = std::make_shared<lambertian>(color(.65, .05, .05));
    auto white = std::make_shared<lambertian>(color(.73, .73, .73));
    auto green = std::make_shared<lambertian>(color(.12, .45, .15));
    auto light = std::make_shared<diffuse_light>(color(15, 15, 15));

    auto cyan = std::make_shared<lambertian>(color(0, 1, 1));
    auto magenta = std::make_shared<lambertian>(color(1, 0, 1));
    auto yellow = std::make_shared<lambertian>(color(1, 1, 0));

    world.make<yz_rect>(0, 555, 0, 555, 555, cyan);
    world.make<yz_rect>(0, 555, 0, 555, 0, magenta);
    world.make<xz_rect>(0, 555, 0, 555, 0, white);
    world.make<xz_rect>(0, 555, 0, 555, 555, white);
    world.make<xy_rect>(0, 555, 0, 555, 555, yellow);
    
    auto lights = std::make_shared<hittable_list>();

    auto aluminium = std::make_shared<metal>(color(0.8, 0.85, 0.88), 0.0);
    auto b1 = std::make_shared<box>(point(0, 0, 0), point(165, 330, 165), white);
    world.make<translate>(std::make_shared<rotate_y>(b1, 15), vec3(265, 0, 295));
    auto glass = std::make_shared<dielectric>(1.5);
    world.make<sphere>(point(190, 90, 190), 90, glass);
    //auto b2 = std::make_shared<box>(point(0, 0, 0), point(165, 165, 165), white);
    //world.make<translate>(std::make_shared<rotate_y>(b2, -18), vec3(130, 0, 65));
    //auto obj1 = std::make_shared<translate>(std::make_shared<rotate_y>(b1, 15), vec3(265, 0, 295));
    //world.make<constant_medium>(obj1, 0.01, color(0, 0, 0));
    //auto obj2 = std::make_shared<translate>(std::make_shared<rotate_y>(b2, -18), vec3(130, 0, 65));
    //world.make<constant_medium>(obj2, 0.01, color(1, 1, 1));

    lights->make<xz_rect>(213, 343, 227, 332, 554, light);
    world.add(lights);

    point from(278, 278, -800);
    point to(278, 278, 0);
    vec3 up(0, 1, 0);
    auto focus_dist = (from - to).length();
    auto aperture = 0.1;
    camera cam(from, to, up, 40, 1, aperture, focus_dist, 0, 0);

    return {world, cam, lights};
}

scene cornell_box_2() {
    hittable_list world;

    auto red   = std::make_shared<lambertian>(color(.65, .05, .05));
    auto green = std::make_shared<lambertian>(color(.12, .45, .15));
    auto white = std::make_shared<lambertian>(color(.73, .73, .73));
    auto black = std::make_shared<lambertian>(color(0.1));
    auto light = std::make_shared<diffuse_light>(color(15, 15, 15));

    auto cyan = std::make_shared<lambertian>(color(0, 1, 1));
    auto magenta = std::make_shared<lambertian>(color(1, 0, 1));
    auto yellow = std::make_shared<lambertian>(color(1, 1, 0));

    //world.make<yz_rect>(0, 555, 0, 555, 555, cyan);
    //world.make<yz_rect>(0, 555, 0, 555, 0, magenta);
    world.make<yz_rect>(0, 555, 0, 555, 555, white);
    world.make<yz_rect>(0, 555, 0, 555, 0, white);
    world.make<xz_rect>(0, 555, 0, 555, 0, white);
    world.make<xz_rect>(0, 555, 0, 555, 555, white);
    world.make<xy_rect>(0, 555, 0, 555, 555, black);
    //world.make<xy_rect>(0, 555, 0, 555, 555, yellow);
    
    auto lights = std::make_shared<hittable_list>();

    auto aluminium = std::make_shared<metal>(color(0.8, 0.85, 0.88), 0.0);
    auto b1 = std::make_shared<box>(point(0, 0, 0), point(165, 330, 165), white);
    world.make<translate>(std::make_shared<rotate_y>(b1, 15), vec3(265, 0, 295));
    auto glass = std::make_shared<dielectric>(1.5);
    world.make<sphere>(point(190, 90, 150), 90, glass);
    world.make<sphere>(point(190, 90, 400), 90, aluminium);
    world.make<sphere>(point(400, 340, 100), 90, glass);

    lights->make<xz_rect>(213, 343, 227, 332, 554, light);
    world.add(lights);

    point from(278, 278, -800);
    point to(278, 278, 0);
    vec3 up(0, 1, 0);
    auto focus_dist = (from - to).length();
    auto aperture = 0.1;
    camera cam(from, to, up, 40, 1, aperture, focus_dist, 0, 0);

    return {world, cam, lights};
}

scene refraction() {
    hittable_list world;

    auto lights = std::make_shared<hittable_list>();
    auto light_mat = std::make_shared<diffuse_light>(color(30));
    world.make<sphere>(point(0, 0, 0), 1, light_mat);

    auto glass = std::make_shared<dielectric>(1.5);
    lights->make<sphere>(point(0, -11.5, 0), 2.5, glass);

    auto white = std::make_shared<lambertian>(color(0.75));
    world.make<sphere>(point(0, -115, 0), 100, white);

    point from(0, 3, -30);
    point to(0, -10, 0);
    vec3 up(0, 1, 0);
    auto focus_dist = (from - to).length();
    auto aperture = 0.1;
    camera cam(from, to, up, 40, 1, aperture, focus_dist, 0, 0);

    world.add(lights);
    return {world, cam, lights};
}

scene book2_final_scene() {
    hittable_list world;

    hittable_list boxes1;
    auto ground_mat = std::make_shared<lambertian>(color(0.48, 0.83, 0.54));
    static constexpr int boxes_per_side = 20;
    for (int i = 0; i < boxes_per_side; ++i) {
        for (int j = 0; j < boxes_per_side; ++j) {
            auto w = 100;
            auto x0 = -1000 + i * w;
            auto z0 = -1000 + j * w;
            auto y0 = 0;
            auto x1 = x0 + w;
            auto y1 = random_double(1, 101);
            auto z1 = z0 + w;

            boxes1.make<box>(point(x0, y0, z0), point(x1, y1, z1), ground_mat);
        }
    }
    world.make<bvh_node>(boxes1, 0, 1);

    auto light_mat = std::make_shared<diffuse_light>(color(7, 7, 7));
    auto lights = std::make_shared<xz_rect>(123, 423, 147, 412, 554, light_mat);
    world.add(lights);

    auto c1 = point(400, 400, 200);
    auto c2 = c1 + vec3(30, 0, 0);
    auto moving_sphere_mat = std::make_shared<lambertian>(color(0.7, 0.3, 0.1));
    world.make<moving_sphere>(c1, c2, 0, 1, 50, moving_sphere_mat);

    world.make<sphere>(point(260, 150, 45), 50, std::make_shared<dielectric>(1.5));
    world.make<sphere>(point(0, 150, 145), 50, std::make_shared<metal>(color(0.8, 0.8, 0.9), 1));

    auto boundary = std::make_shared<sphere>(point(360, 150, 145), 70, std::make_shared<dielectric>(1.5));
    world.add(boundary);
    world.make<constant_medium>(boundary, 0.2, color(0.2, 0.4, 0.9));
    boundary = std::make_shared<sphere>(point(0, 0, 0), 5000, std::make_shared<dielectric>(1.5));
    world.make<constant_medium>(boundary, 0.0001, color(1, 1, 1));

    auto earth_mat = std::make_shared<lambertian>(std::make_shared<image>("../res/earthmap.jpg"));
    world.make<sphere>(point(400, 200, 400), 100, earth_mat);
    auto perlin_mat = std::make_shared<lambertian>(std::make_shared<noise>(0.1));
    world.make<sphere>(point(220, 280, 300), 80, perlin_mat);

    hittable_list boxes2;
    auto white = std::make_shared<lambertian>(color(0.73, 0.73, 0.73));
    int ns = 1000;
    for (int j = 0; j < ns; ++j) {
        boxes2.make<sphere>(vec3_random(0, 165), 10, white);
    }

    world.make<translate>(std::make_shared<rotate_y>(std::make_shared<bvh_node>(boxes2, 0, 1), 15), vec3(-100, 270, 395));

    point from(478, 278, -600);
    point to(278, 278, 0);
    vec3 up(0, 1, 0);
    //auto focus_dist = 10.0;
    auto focus_dist = (from - to).length();
    auto aperture = 0.1;
    camera cam(from, to, up, 40, 1, aperture, focus_dist, 0, 1);

    return {world, cam, lights};
}

scene book2_final_scene_random() {
    hittable_list world;

    hittable_list boxes1;
    static constexpr int boxes_per_side = 20;
    for (int i = 0; i < boxes_per_side; ++i) {
        for (int j = 0; j < boxes_per_side; ++j) {
            auto w = 100;
            auto x0 = -1000 + i * w;
            auto z0 = -1000 + j * w;
            auto y0 = 0;
            auto x1 = x0 + w;
            auto y1 = random_double(1, 101);
            auto z1 = z0 + w;

            boxes1.make<box>(point(x0, y0, z0), point(x1, y1, z1), std::make_shared<lambertian>(color::random()));
        }
    }
    world.make<bvh_node>(boxes1, 0, 1);

    auto lights = std::make_shared<hittable_list>();
    lights->make<xz_rect>(123, 423, 147, 412, 554, std::make_shared<diffuse_light>(color::random() * 4 + 3));

    auto c1 = point(400, 400, 200);
    auto c2 = c1 + vec3(30, 0, 0);
    world.make<moving_sphere>(c1, c2, 0, 1, 50, std::make_shared<lambertian>(color::random()));

    lights->make<sphere>(point(260, 150, 45), 50, std::make_shared<dielectric>(1.5));
    lights->make<sphere>(point(0, 150, 145), 50, std::make_shared<metal>(color::random(), 0.1));

    auto boundary = std::make_shared<sphere>(point(360, 150, 145), 70, std::make_shared<dielectric>(1.5));
    lights->add(boundary);
    world.make<constant_medium>(boundary, 0.2, color::random());
    boundary = std::make_shared<sphere>(point(0, 0, 0), 5000, std::make_shared<dielectric>(1.5));
    world.make<constant_medium>(boundary, 0.0001, color::random());

    auto earth_mat = std::make_shared<lambertian>(std::make_shared<image>("../res/earthmap.jpg"));
    world.make<sphere>(point(400, 200, 400), 100, earth_mat);
    auto perlin_mat = std::make_shared<lambertian>(std::make_shared<noise>(color::random(), color::random(), 0.1));
    world.make<sphere>(point(220, 280, 300), 80, perlin_mat);

    hittable_list boxes2;
    int ns = 1000;
    for (int j = 0; j < ns; ++j) {
        boxes2.make<sphere>(vec3_random(0, 165), 10, std::make_shared<lambertian>(color::random()));
        //auto bound = std::make_shared<sphere>(vec3_random(0, 165), 10, std::make_shared<dielectric>(1.5));
        //boxes2.add(bound);
        //boxes2.make<constant_medium>(bound, 0.01, color::random());
    }

    world.make<translate>(std::make_shared<rotate_y>(std::make_shared<bvh_node>(boxes2, 0, 1), 15), vec3(-100, 270, 395));

    world.add(lights);

    point from(478, 278, -600);
    point to(278, 278, 0);
    vec3 up(0, 1, 0);
    auto focus_dist = (from - to).length();
    auto aperture = 0.1;
    camera cam(from, to, up, 40, 1, aperture, focus_dist, 0, 1);

    return {world, cam, lights};
}

scene endwalker_title() {
    hittable_list world;
    auto lights = std::make_shared<hittable_list>();

    auto earth_mat = std::make_shared<lambertian>(std::make_shared<image>("../res/earthmap.jpg"));
    world.make<sphere>(point(-141, -342, -158), 225, earth_mat);
    auto moon_mat = std::make_shared<lambertian>(std::make_shared<image>("../res/moonmap.png"));
    world.make<sphere>(point(48, 450, 87), 182, moon_mat);

    //auto box_mat = std::make_shared<lambertian>(color(0, 0.002, 0.05));
    auto box_mat = std::make_shared<lambertian>(color(0.3, 0.3, 0.3));
    world.make<box>(point(-42, -431, 83), point(20, 451, 98), box_mat);

    auto boundary = std::make_shared<sphere>(point(0, 0, 0), 5000, std::make_shared<dielectric>(1.5));
    world.make<constant_medium>(boundary, 0.0001, color(1, 1, 1));

    auto light_mat = std::make_shared<diffuse_light>(color(11, 6, 1));
    lights->make<sphere>(point(204, 669, -51), 100, light_mat);

    auto back_light_mat = std::make_shared<diffuse_light>(color(60, 60, 60));
    lights->make<sphere>(point(300, -1500, 300), 50, back_light_mat);

    world.add(lights);

    point from(198, -1093, 184);
    point to(0, 0, 0);
    vec3 up(-0.2, 1, 0);
    auto focus_dist = (from - to).length();
    auto aperture = 0.1;
    camera cam(from, to, up, 25, 16.0 / 9.0, aperture, focus_dist, 0, 1);

    return {world, cam, lights};
}

#endif