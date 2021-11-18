#include <atomic>
#include <iostream>
#include <fstream>
#include <thread>
#include <future>
#include <functional>
#include <deque>
#include <mutex>

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

static constexpr int worker_count = 8;
static constexpr int tile_size = 40;

void render_tile(camera const& cam, hittable const& world, background_func bg, canvas& img, int depth, int tile_x, int tile_y) {
    auto start_x = tile_x * tile_size;
    auto start_y = tile_y * tile_size;
    for (int y = start_y; y < start_y + tile_size; ++y) {
        for (int x = start_x; x < start_x + tile_size; ++x) {
            color pixel(0, 0, 0);
            for (int s = 0; s < img.samples; ++s) {
                auto u = (x + random_double()) / (img.width - 1);
                auto v = (y + random_double()) / (img.height - 1);
                auto r = cam.get_ray(u, v);
                pixel += ray_color(r, bg, world, depth);
            }
            img.write_pixel(x, img.height - y - 1, pixel);
        }
    }
    count++;
}

struct job_queue {
    using Job = std::packaged_task<void()>;
    std::deque<Job> jobs;
    std::mutex mut;

    void add_tile(camera const& cam, hittable const& world, background_func bg, canvas& img, int depth, int tile_x, int tile_y) {
        Job j = Job(std::bind(render_tile, std::ref(cam), std::ref(world), bg, std::ref(img), depth, tile_x, tile_y));
        std::scoped_lock<std::mutex> lock(mut);
        jobs.push_back(std::move(j));
    }

    std::tuple<bool, Job> get_job(int worker_id) {
        static int job_idx = 0;
        std::scoped_lock<std::mutex> lock(mut);
        if (jobs.empty()) {
            return {false, Job()};
        }
        Job j = std::move(jobs.back());
        jobs.pop_back();
        return {true, std::move(j)};
    }
};

void render_tiled(camera const& cam, hittable const& world, background_func bg, canvas& img, int depth) {
    assert(img.width % tile_size == 0);
    assert(img.height % tile_size == 0);

    const auto tiles_hor = img.width / tile_size;
    const auto tiles_ver = img.height / tile_size;
    job_queue jq;
    for (int y = 0; y < tiles_ver; ++y) {
        for (int x = 0; x < tiles_hor; ++x) {
            jq.add_tile(cam, world, bg, img, depth, x, y);
        }
    }

    std::array<std::thread, worker_count> workers;
    //for (auto& worker : workers) {
    for (int i = 0; i < worker_count; ++i) {
        workers[i] = std::thread([&, i](){
            auto [has_job, job] = jq.get_job(i);
            while (has_job) {
                job();
                std::tie(has_job, job) = jq.get_job(i);
            }
        });
    }

    std::thread prog([=](){
        size_t old_count;
        const auto tile_count = tiles_hor * tiles_ver;
        while (count < tile_count) {
            if (count != old_count) {
                print_progress(double(count) / tile_count);
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
    static constexpr int h = 600;
    static constexpr int w = h * aspect_ratio;
    static constexpr int samples = 256;
    static constexpr int max_depth = 64;
    canvas can(w, h, samples);

    //random_scene_noise().save("random_scene_noise.json");
    //random_scene_og().save("random_scene_og.json");

    //auto s = book2_final_scene_random();
    //s.save("book2_final_scene_random.json");
    auto s = scene::load("book2_final_scene_random.json");

    auto bg = [](auto){ return color(0, 0, 0); };

    // render
    std::ofstream ofs("out.ppm");
    auto bvh = bvh_node(s.world, 0, 0);
    //render_mt(s.cam, s.world, bg, can, max_depth, 20);
    render_tiled(s.cam, s.world, bg, can, max_depth);
    can.to_ppm(ofs);
}