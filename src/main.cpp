#include <atomic>
#include <iostream>
#include <fstream>
#include <thread>

#include "camera.h"
#include "canvas.h"
#include "color.h"
#include "hittable_list.h"
#include "ray.h"
#include "sphere.h"
#include "util.h"
#include "vec3.h"

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

color ray_color(ray const& r, hittable const& w, size_t depth) {
    hit_record h;
    
    if (depth <= 0) return color(0, 0, 0);

    if (w.hit(r, 0.001, inf, h)) {
        point target = h.p + h.n + vec3_random_unit();
        return 0.5 * ray_color(ray(h.p, target - h.p), w, depth - 1);
    }

    auto dir_n = normalize(r.direction);
    auto t = 0.5 * dir_n.y + 1;
    return t * color(0.5, 0.7, 1.0) + (1 - t) * color(1, 1, 1);
}

volatile std::atomic<size_t> count{0};

void render_segment(camera const& cam, hittable_list const& world, canvas& img, int depth, int segment, int segment_count) {
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
                pixel += ray_color(r, world, depth);
            }
            img.write_pixel(x, img.height - y - 1, pixel);
        }
        count++;
    }
}

void render_mt(camera const& cam, hittable_list const& world, canvas& img, int depth, int segment_count) {
    std::vector<std::thread> workers;
    for (int segment = 0; segment < segment_count; ++segment) {
        workers.push_back(std::thread([&, segment](){ render_segment(cam, world, img, depth, segment, segment_count); }));
    }
    std::thread prog([=](){
        size_t old_count;
        while (count < img.height) {
            if (count != old_count) {
                print_progress(double(count) / img.height);
                old_count = count;
            }
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
    static constexpr double aspect_ratio = 16.0 / 9.0;
    static constexpr int w = 800;
    static constexpr int h = w / aspect_ratio;
    static constexpr int samples = 64;
    static constexpr int max_depth = 16;
    canvas can(w, h, samples);

    // world
    hittable_list world;
    world.make<sphere>(point(0, 0, -1), 0.5);
    world.make<sphere>(point(0, -100.5, -1), 100);

    // camera
    camera cam;

    // render
    std::ofstream ofs("out.ppm");
    render_mt(cam, world, can, max_depth, 5);
    can.to_ppm(ofs);
}