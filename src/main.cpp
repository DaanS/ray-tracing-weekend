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
    if (depth <= 0) return color(0, 0, 0);

    // if the ray escaped, return background
    if (!w.hit(r, 0.001, inf, h)) return bg(r);

    auto [res, s] = h.mat_ptr->scatter(r, h);
    auto emitted = h.mat_ptr->emitted(r, h, h.u, h.v, h.p);

    // if the hit didn't scatter, return only emitted light
    if (!res) return emitted;

    if (s.is_specular) return s.attenuation * ray_color(s.specular, bg, w, lights, depth - 1);

    auto light_pdf = std::make_shared<hittable_pdf>(lights, h.p);
    mixture_pdf mixed_pdf(s.pdf_ptr, light_pdf);

    auto scattered = ray(h.p, mixed_pdf.generate(), r.time);
    auto pdf_val = mixed_pdf.value(scattered.direction);
    //scattered = ray(h.p, cos_pdf->generate(), r.time);
    //pdf_val = cos_pdf->value(scattered.direction);
    
    // return emitted + scattered
    return emitted + s.attenuation * h.mat_ptr->scattering_pdf(r, h, scattered) * ray_color(scattered, bg, w, lights, depth - 1) / pdf_val;
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

    double error(color cur, color mrg) {
        color res;
        for (int i = 0; i < 3; ++i) {
            res[i] = std::abs(mrg[i] - cur[i]);
        } 
        return (res.x + res.y + res.z) / std::sqrt(mrg.x + mrg.y + mrg.z);
    }

    double merge(canvas& img, int start_x, int start_y, int iteration) {
        auto e_sum = 0.0;
        for (int y = start_y; y < start_y + Side; ++y) {
            for (int x = start_x; x < start_x + Side; ++x) {
                auto tile_y = y - start_y;
                auto tile_x = x - start_x;
                auto img_y = img.height - y - 1;
                auto cur_col = img.pixel_at(x, img_y);
                auto new_col = data[tile_y][tile_x];
                auto mrg_col = cur_col + new_col;
                img.write_pixel(x, img_y, mrg_col);
                //auto dif_col = (mrg_col / (iteration + 1)) - (cur_col / iteration);
                //auto dif_col = color(channel_diff(mrg_col.x, cur_col.x), channel_diff(mrg_col.y, cur_col.y), channel_diff(mrg_col.z, cur_col.z));
                //e_sum += dif_col.length_squared() / 3;
                //auto dif = channel_diff(mrg_col.x + mrg_col.y + mrg_col.z, cur_col.x + cur_col.y + cur_col.z);
                //e_sum += dif / 3;
                e_sum += error(mrg_col / (iteration + 1), cur_col / iteration);
            }
        }
        return e_sum / (Side * Side);
    }
};

void render_tile(camera const& cam, hittable const& world, background_func bg, std::shared_ptr<hittable> lights, canvas& img, int depth, int tile_x, int tile_y, std::ofstream& ofs) {
    auto start_x = tile_x * tile_size;
    auto start_y = tile_y * tile_size;
    tile<tile_size> t;
    auto max_iterations = 8;
    assert(img.samples % max_iterations == 0);
    auto samples_per_iteration = img.samples / max_iterations;
    for (int i = 0; i < max_iterations; ++i) {
        for (int y = start_y; y < start_y + tile_size; ++y) {
            for (int x = start_x; x < start_x + tile_size; ++x) {
                color pixel(0, 0, 0);
                for (int s = 0; s < samples_per_iteration; ++s) {
                    auto u = (x + random_double()) / (img.width - 1);
                    auto v = (y + random_double()) / (img.height - 1);
                    auto r = cam.get_ray(u, v);
                    pixel += ray_color(r, bg, world, lights, depth);
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

void render_tiled(camera const& cam, hittable const& world, background_func bg, std::shared_ptr<hittable> lights, canvas& img, int depth) {
    assert(img.width % tile_size == 0);
    assert(img.height % tile_size == 0);

    const auto tiles_hor = img.width / tile_size;
    const auto tiles_ver = img.height / tile_size;
    job_queue jq;
    for (int y = 0; y < tiles_ver; ++y) {
        for (int x = 0; x < tiles_hor; ++x) {
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
    static constexpr int h = 400;
    static constexpr int w = h * aspect_ratio;
    static constexpr int samples = 256;
    static constexpr int max_depth = 64;
    canvas can(w, h, samples);

    auto s = book2_final_scene_random();

    auto bg = [](auto){ return color(0, 0, 0); };
    //auto bg = background_overcast;
    //auto bg = [](auto) { return color(0, 0.01, 0.04); };

    // render
    std::ofstream ofs("out.ppm");
    auto bvh = bvh_node(s.world, 0, 0);
    //render_mt(s.cam, s.world, bg, can, max_depth, 20);
    render_tiled(s.cam, s.world, bg, s.lights, can, max_depth);
    can.to_ppm(ofs);
}