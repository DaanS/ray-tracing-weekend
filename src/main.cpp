#include <iostream>
#include <fstream>

#include "util.h"
#include "color.h"
#include "ray.h"
#include "vec3.h"
#include "hittable_list.h"
#include "sphere.h"

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

color ray_color(ray const& r, hittable const& w) {
    hit_record h;
    if (w.hit(r, 0, inf, h)) {
        return 0.5 * (h.n + color(1, 1, 1));
    }
    auto dir_n = normalize(r.direction);
    auto t = 0.5 * dir_n.y + 1;
    return t * color(0.5, 0.7, 1.0) + (1 - t) * color(1, 1, 1);
}

int main() {
    // image
    static constexpr double aspect_ratio = 16.0 / 9.0;
    static constexpr int w = 400;
    static constexpr int h = w / aspect_ratio;

    // world
    hittable_list world;
    world.make<sphere>(point(0, 0, -1), 0.5);
    world.make<sphere>(point(0, -100.5, -1), 100);

    // camera
    static constexpr double view_h = 2;
    static constexpr double view_w = view_h * aspect_ratio;
    static constexpr double focal_length = 1;

    static constexpr auto origin = point(0, 0, 0);
    static constexpr auto hor = vec3(view_w, 0, 0);
    static constexpr auto ver = vec3(0, view_h, 0);
    static constexpr auto lower_left = origin - hor / 2 - ver / 2 - vec3(0, 0, focal_length);

    // render
    std::ofstream ofs("out.ppm");
    ofs << "P3\n" << w << ' ' << h << "\n255\n";
    for (int y = h - 1; y >= 0; --y) {
        for (int x = 0; x < w; ++x) {
            auto u = double(x) / (w - 1);
            auto v = double(y) / (h - 1);
            auto r = ray(origin, lower_left + u * hor + v * ver - origin);
            write_color(ofs, ray_color(r, world));
        }
        print_progress((h - y) / h);
    }
    std::cout << std::endl;
}