#include <atomic>
#include <iostream>
#include <fstream>
#include <thread>

#include "bvh.h"
#include "camera.h"
#include "canvas.h"
#include "color.h"
#include "hittable_list.h"
#include "material.h"
#include "ray.h"
#include "sphere.h"
#include "util.h"
#include "vec3.h"
#include "scene.h"

void print_progress(double prog) {
    static constexpr int barWidth = 70;

    std::cout << "[";
    int pos = barWidth * prog;
    for (int i = 0; i < barWidth; ++i) {
        if (i < pos) std::cout << "=";
        else if (i == pos) std::cout << ">";
        else std::cout << " ";
    }
    std::cout << "] " << int(prog * 100.0) << " %\r";
    std::cout.flush();
}

using background_func = color(ray const&);

color background_overcast(ray const& r) {
    auto dir_n = normalize(r.direction);
    auto t = 0.5 * (dir_n.y + 1);
    return t * color(0.6, 0.7, 1.0) + (1 - t) * color(1, 0.8, 0.6);
}

color ray_color(ray const& r, background_func bg, hittable const& w, size_t depth) {
    hit_record h;
    
    // no light is gathered at the bottom of the abyss
    if (depth <= 0) return color(0, 0, 0);

    // if the ray escaped, return background
    if (!w.hit(r, 0.001, inf, h)) return bg(r);

    auto [res, attenuation, scattered] = h.mat_ptr->scatter(r, h);
    auto emitted = h.mat_ptr->emitted(h.u, h.v, h.p);

    // if the hit didn't scatter, return only emitted light
    if (!res) return emitted;

    // return emitted + scattered
    return emitted + attenuation * ray_color(scattered, bg, w, depth - 1);
}

volatile std::atomic<size_t> count{0};

void render_segment(camera const& cam, hittable const& world, background_func bg, canvas& img, int depth, int segment, int segment_count) {
    assert(img.height % segment_count == 0);
    auto segment_h = img.height / segment_count;
    auto start_y = segment * segment_h;
    for (int y = start_y; y < start_y + segment_h; ++y) {
        for (int x = 0; x < img.width; ++x) {
            color pixel(0, 0, 0);
            for (int s = 0; s < img.samples; ++s) {
                auto u = (x + random_double()) / (img.width - 1);
                auto v = (y + random_double()) / (img.height - 1);
                auto r = cam.get_ray(u, v);
                pixel += ray_color(r, bg, world, depth);
            }
            img.write_pixel(x, img.height - y - 1, pixel);
        }
        count++;
    }
}

void render_mt(camera const& cam, hittable const& world, background_func bg, canvas& img, int depth, int segment_count) {
    std::vector<std::thread> workers;
    for (int segment = 0; segment < segment_count; ++segment) {
        workers.push_back(std::thread([&, segment](){ render_segment(cam, world, bg, img, depth, segment, segment_count); }));
    }
    std::thread prog([=](){
        size_t old_count;
        while (count < img.height) {
            if (count != old_count) {
                print_progress(double(count) / img.height);
                old_count = count;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        }
    });
    for (auto& worker : workers) {
        if (worker.joinable()) worker.join();
    }
    prog.join();
    std::cout << std::endl;
}

int main() {
    // image
    //static constexpr double aspect_ratio = 16.0 / 9.0;
    static constexpr double aspect_ratio = 1.0; // XXX link with camera aspect ratio in scene
    static constexpr int h = 450;
    static constexpr int w = h * aspect_ratio;
    static constexpr int samples = 2048;
    static constexpr int max_depth = 64;
    canvas can(w, h, samples);

    //random_scene_noise().save("random_scene_noise.json");
    //random_scene_og().save("random_scene_og.json");

    auto s = cornell_box();
    //auto s = scene::load("random_scene_og.json");

    auto bg = [](auto){ return color(0, 0, 0); };

    // render
    std::ofstream ofs("out.ppm");
    auto bvh = bvh_node(s.world, 0, 0);
    render_mt(s.cam, s.world, bg, can, max_depth, 10);
    can.to_ppm(ofs);
}