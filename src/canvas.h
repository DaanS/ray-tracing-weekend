#ifndef CANVAS_H
#define CANVAS_H

#include <cassert>
#include <iostream>
#include "color.h"
#include "vec3.h"

struct canvas {
    int width;
    int height;
    int depth = 3;
    int samples;
    std::vector<double> pix;

    canvas(int width, int height, int samples) : width(width), height(height), samples(samples) {
        pix.reserve(width * height * depth);
    }

    void assert_pos(int x, int y) {
        assert(x >= 0 && x < width);
        assert(y >= 0 && y < height);
    }

    size_t pix_start(int x, int y) { return depth * (x + y * width); }

    void write_pixel(int x, int y, color_rgb col) {
        assert_pos(x, y);
        size_t start = pix_start(x, y);
        auto rgb = col.to_rgb();
        pix[start] = rgb.r;
        pix[start + 1] = rgb.g;
        pix[start + 2] = rgb.b;
    }

    color_rgb pixel_at(int x, int y) {
        assert_pos(x, y);
        size_t start = pix_start(x, y);
        return color_rgb(pix[start], pix[start + 1], pix[start + 2]);
    }

    int cast_color(double c) {
        return static_cast<int>(256 * clamp(c, 0, 0.99999));
    }

    // XXX find a cleaner way to pass samples I guess?
    void to_ppm(std::ostream& os) {
        os << "P3\n" << width << ' ' << height << "\n255\n";
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                color_rgb c = scale_color(pixel_at(x, y), samples);
                os << cast_color(c.r) << ' ' 
                   << cast_color(c.g) << ' ' 
                   << cast_color(c.b) << '\n';
            }
        }
    }
};

#endif