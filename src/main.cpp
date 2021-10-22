#include <iostream>
#include <fstream>

#include "color.h"
#include "ray.h"
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

bool hit_sphere(point const& center, double radius, ray const& r) {
    vec3 oc = r.origin - center;
    auto a = dot(r.direction, r.direction);
    auto b = 2 * dot(oc, r.direction);
    auto c = dot(oc, oc) - radius * radius;
    auto discriminant = b * b - 4 * a * c;
    return discriminant > 0;
}

color ray_color(ray const& r) {
    if (hit_sphere(point(0, 0, -1), 0.5, r)) return color(1, 0, 0);
    vec3 dir_n = normalize(r.direction);
    auto t = 0.5 * dir_n.y + 1;
    return t * color(0.5, 0.7, 1.0) + (1 - t) * color(1, 1, 1);
}

int main() {
    // image
    static constexpr double aspect_ratio = 16.0 / 9.0;
    static constexpr int w = 400;
    static constexpr int h = w / aspect_ratio;

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
            write_color(ofs, ray_color(r));
        }
        print_progress((h - y) / h);
    }
    std::cout << std::endl;
}