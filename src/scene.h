#ifndef SCENE_H
#define SCENE_H

#include <fstream>
#include <nlohmann/json.hpp>
#include "hittable_list.h"
#include "camera.h"

struct scene;
void to_json(json& j, scene const& s);
void from_json(json const& j, scene& s);
struct scene {
    hittable_list world;
    camera cam;

    bool operator==(scene const& rhs) const {
        return world == rhs.world && cam == rhs.cam;
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
        {"camera", s.cam}
    };
}

void from_json(json const& j, scene& s) {
    j.at("world").get_to(s.world);
    j.at("camera").get_to(s.cam);
}

scene four_sphere_scene() {
    // world
    hittable_list world;
    //auto mat_ground = std::make_shared<lambertian>(color(0.8, 0.8, 0.0));
    //auto mat_ground = std::make_shared<lambertian>(std::make_shared<noise>(color(0, 1, 1), color(1, 0, 1), 2));
    auto mat_ground = std::make_shared<lambertian>(std::make_shared<image>("../res/earthmap.jpg"));
    auto mat_center = std::make_shared<lambertian>(color(0.7, 0.3, 0.3));
    auto mat_left = std::make_shared<dielectric>(1.5);
    auto mat_right = std::make_shared<metal>(color(0.8, 0.6, 0.2), 0.0);
    world.make<sphere>(point(0, -100.5, -1), 100, mat_ground);
    world.make<sphere>(point(0, 0, -1), 0.5, mat_ground);
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

    return {world, cam};
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

    return {world, cam};
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

    return {world, cam};
}

#endif