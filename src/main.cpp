//#define _GNU_SOURCE
//#include <fenv.h>

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
#include "pdf.h"
#include "ray.h"
#include "scene.h"
#include "spectrum.h"
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

using background_func = color(ray const&);

color background_overcast(ray const& r) {
    auto dir_n = normalize(r.direction);
    auto t = 0.5 * (dir_n.y + 1);
    return t * color(0.6, 0.7, 1.0) + (1 - t) * color(1, 0.8, 0.6);
}

color ray_color(ray const& r, background_func bg, hittable const& w, std::shared_ptr<hittable> lights, size_t depth) {
    hit_record h;
    
    // no light is gathered at the bottom of the abyss
    if (depth <= 0) return color(0);

    // if the ray escaped, return background
    if (!w.hit(r, 0.001, inf, h)) return bg(r);

    auto [res, s] = h.mat_ptr->scatter(r, h, 550);
    auto emitted = h.mat_ptr->emitted(r, h, h.u, h.v, h.p);

    // if the hit didn't scatter, return only emitted light
    if (!res) return emitted;

    if (s.is_specular) return s.attenuation * ray_color(s.specular, bg, w, lights, depth - 1);

    auto light_pdf = std::make_shared<hittable_pdf>(lights, h.p);
    mixture_pdf mixed_pdf(s.pdf_ptr, light_pdf);

    // XXX fix this better, maybe by making mixture_pdf take into account the odds of hitting the child pdfs?
    if (!lights) mixed_pdf = mixture_pdf(s.pdf_ptr, s.pdf_ptr);
    auto scattered = ray(h.p, mixed_pdf.generate(), r.time);
    auto pdf_val = mixed_pdf.value(scattered.direction);
    
    // return emitted + scattered
    return emitted + s.attenuation * h.mat_ptr->scattering_pdf(r, h, scattered) * ray_color(scattered, bg, w, lights, depth - 1) / pdf_val;
}

double ray_value(ray const& r, background_func bg, hittable const& w, std::shared_ptr<hittable> lights, size_t depth, double lambda) {
    hit_record h;
    
    // no light is gathered at the bottom of the abyss
    if (depth <= 0) return 0;

    // if the ray escaped, return background
    if (!w.hit(r, 0.001, inf, h)) return bg(r).value_at(lambda);

    auto [res, s] = h.mat_ptr->scatter(r, h, lambda);
    auto emitted = h.mat_ptr->emitted(r, h, h.u, h.v, h.p).value_at(lambda);
    auto attenuation = s.attenuation.value_at(lambda);

    // if the hit didn't scatter, return only emitted light
    if (!res) return emitted;

    if (s.is_specular) return attenuation * ray_value(s.specular, bg, w, lights, depth - 1, lambda);

    auto light_pdf = std::make_shared<hittable_pdf>(lights, h.p);
    mixture_pdf mixed_pdf(s.pdf_ptr, light_pdf);

    // XXX fix this better, maybe by making mixture_pdf take into account the odds of hitting the child pdfs?
    if (!lights) mixed_pdf = mixture_pdf(s.pdf_ptr, s.pdf_ptr);
    auto scattered = ray(h.p, mixed_pdf.generate(), r.time);
    auto pdf_val = mixed_pdf.value(scattered.direction);
    
    // return emitted + scattered
    return emitted + attenuation * h.mat_ptr->scattering_pdf(r, h, scattered) * ray_value(scattered, bg, w, lights, depth - 1, lambda) / pdf_val;
}

volatile std::atomic<size_t> count{0};

static constexpr int worker_count = 8;
static constexpr int tile_size = 40;

template<int Side>
struct tile {
    color data[Side][Side];

    void write_pixel(int x, int y, color c) {
        data[y][x] = c;
    }

    double channel_diff(double a, double b) {
        if (a > b) return a / b;
        else return b / a;
    }

    double error(color_rgb mrg, color_rgb cur) {
        color_rgb res;
        for (int i = 0; i < 3; ++i) {
            res[i] = std::abs(mrg[i] - cur[i]);
        } 
        return (res.r + res.g + res.b) / std::sqrt(mrg.r + mrg.g + mrg.b + epsilon);
    }

    double merge(canvas& img, int start_x, int start_y, int iteration) {
        auto e_sum = 0.0;
        for (int y = start_y; y < start_y + Side; ++y) {
            for (int x = start_x; x < start_x + Side; ++x) {
                auto tile_y = y - start_y;
                auto tile_x = x - start_x;
                auto img_y = img.height - y - 1;
                auto cur_col = img.pixel_at(x, img_y);
                auto new_col = data[tile_y][tile_x].to_rgb();
                new_col.ensure_positive();
                auto mrg_col = cur_col + new_col;
                //if (x == start_x || y == start_y) mrg_col = color_rgb(1, 0, 0);
                img.write_pixel(x, img_y, mrg_col);
                //auto dif_col = (mrg_col / (iteration + 1)) - (cur_col / iteration);
                //auto dif_col = color(channel_diff(mrg_col.x, cur_col.x), channel_diff(mrg_col.y, cur_col.y), channel_diff(mrg_col.z, cur_col.z));
                //e_sum += dif_col.length_squared() / 3;
                //auto dif = channel_diff(mrg_col.x + mrg_col.y + mrg_col.z, cur_col.x + cur_col.y + cur_col.z);
                //e_sum += dif / 3;
                e_sum += error(mrg_col / (iteration + 1), (iteration > 0 ? cur_col / iteration : 0));
            }
        }
        return e_sum / (Side * Side);
    }
};

void render_tile(camera const& cam, hittable const& world, background_func bg, std::shared_ptr<hittable> lights, canvas& img, int depth, int tile_x, int tile_y, std::ofstream& ofs) {
    static constexpr int color_sample_count = 960;
    auto start_x = tile_x * tile_size;
    auto start_y = tile_y * tile_size;
    tile<tile_size> t;
    auto max_iterations = 8;
    assert(img.samples % max_iterations == 0);
    auto samples_per_iteration = img.samples / max_iterations;
    std::array<spectrum_sample, color_sample_count> samples;
    for (int i = 0; i < max_iterations; ++i) {
        for (int y = start_y; y < start_y + tile_size; ++y) {
            for (int x = start_x; x < start_x + tile_size; ++x) {
                color pixel(0);
                for (int s = 0; s < samples_per_iteration; ++s) {
                    auto u = (x + random_double()) / (img.width - 1);
                    auto v = (y + random_double()) / (img.height - 1);
                    auto r = cam.get_ray(u, v);

                    for (int l = 0; l < color_sample_count; ++l) {
                        auto lambda = random_double(400, 700);
                        auto val = ray_value(r, bg, world, lights, depth, lambda);
                        samples[l] = {lambda, val}; 
                    }
                    pixel += color_spectrum::from_samples(samples);

                    //pixel += ray_color(r, bg, world, lights, depth);
                }
                auto tile_y = y - start_y;
                auto tile_x = x - start_x;
                t.write_pixel(tile_x, tile_y, pixel);
            }
        }
        auto mse = t.merge(img, start_x, start_y, i);
        ofs << "tile (" << tile_x << ", " << tile_y << ") iteration " << i << " mse = " << mse << std::endl;
        if (std::isfinite(mse) && mse < 0.25) {
            ofs << "finishing tile (" << tile_x << ", " << tile_y << ") after iteration " << i << std::endl;
            for (int j = i + 1; j < max_iterations; ++j) t.merge(img, start_x, start_y, j);
            break;
        }
    }
    count++;
}

struct job_queue {
    using Job = std::packaged_task<void(std::ofstream&)>;
    std::deque<Job> jobs;
    std::mutex mut;

    void add_tile(camera const& cam, hittable const& world, background_func bg, std::shared_ptr<hittable> lights, canvas& img, int depth, int tile_x, int tile_y) {
        Job j = Job(std::bind(render_tile, std::ref(cam), std::ref(world), bg, lights, std::ref(img), depth, tile_x, tile_y, std::placeholders::_1));
        std::scoped_lock<std::mutex> lock(mut);
        jobs.push_back(std::move(j));
    }

    std::tuple<bool, Job> get_job(int worker_id) {
        std::scoped_lock<std::mutex> lock(mut);
        if (jobs.empty()) {
            return {false, Job()};
        }
        Job j = std::move(jobs.back());
        jobs.pop_back();
        return {true, std::move(j)};
    }
};

void render_tiled(camera const& cam, hittable const& world, background_func bg, std::shared_ptr<hittable> lights, canvas& img, int depth) {
    assert(img.width % tile_size == 0);
    assert(img.height % tile_size == 0);

    const auto tiles_hor = img.width / tile_size;
    const auto tiles_ver = img.height / tile_size;
    job_queue jq;
    for (int y = 0; y < tiles_ver; ++y) {
        for (int x = 0; x < tiles_hor; ++x) {
    //for (int y = 0; y < tiles_ver / 2 - 8; ++y) {
    //    for (int x = tiles_hor / 2 - 1; x < tiles_hor - 13; ++x) {
    //for (int y = tiles_ver / 2 - 5; y < tiles_ver - 20; ++y) {
    //    for (int x = 12; x < tiles_hor / 2 - 2; ++x) {
            jq.add_tile(cam, world, bg, lights, img, depth, x, y);
        }
    }

    std::array<std::thread, worker_count> workers;
    for (int i = 0; i < worker_count; ++i) {
        workers[i] = std::thread([&, i](){
            std::ofstream ofs("worker" + std::to_string(i) + ".out");
            auto [has_job, job] = jq.get_job(i);
            while (has_job) {
                job(ofs);
                std::tie(has_job, job) = jq.get_job(i);
            }
        });
    }

    static volatile std::atomic<bool> prog_stop(false);
    std::thread prog([=](){
        size_t old_count = 0;
        const auto tile_count = tiles_hor * tiles_ver;
        while (count < tile_count && !prog_stop) {
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
    prog_stop = true;
    prog.join();
    std::cout << std::endl;
}

int main() {
    //feenableexcept(FE_ALL_EXCEPT & ~FE_INEXACT);

    // image
    //static constexpr double aspect_ratio = 16.0 / 9.0;
    //static constexpr double aspect_ratio = 1.0; // XXX link with camera aspect ratio in scene

    //auto s = four_sphere_scene();
    //auto s = macbeth_spd();
    auto s = cornell_box_2();
    //auto s = refraction();
    //auto src = random_scene_noise();
    //src.save("../random_scene_noise_with_lights.json");
    //auto s = scene::load("../random_scene_noise_with_lights.json");

    //auto bg = [](auto){ return color(0); };
    //auto bg = background_overcast;
    //auto bg = [](auto){ return color(0.2); };
    auto bg = [](auto){ return color(1); };
    //auto bg = [](auto) { return color(0, 0.01, 0.04); };

    static constexpr int h = 2048;
    int w = h * s.cam.aspect_ratio;
    static constexpr int samples = 64;
    static constexpr int max_depth = 64;
    canvas can(w, h, samples);

    // render
    auto bvh = bvh_node(s.world, 0, 0);
    //render_mt(s.cam, s.world, bg, can, max_depth, 20);
    render_tiled(s.cam, s.world, bg, s.lights, can, max_depth);
    std::ofstream ofs("out.ppm");
    can.to_ppm(ofs);
}