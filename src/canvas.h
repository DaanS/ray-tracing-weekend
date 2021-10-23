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

    void write_pixel(int x, int y, color col) {
        assert_pos(x, y);
        size_t start = pix_start(x, y);
        pix[start] = col.x;
        pix[start + 1] = col.y;
        pix[start + 2] = col.z;
    }

    color pixel_at(int x, int y) {
        assert_pos(x, y);
        size_t start = pix_start(x, y);
        return color(pix[start], pix[start + 1], pix[start + 2]);
    }

    int cast_color(double c) {
        return static_cast<int>(256 * clamp(c, 0, 0.99999));
    }

    // XXX find a cleaner way to pass samples I guess?
    void to_ppm(std::ostream& os) {
        os << "P3\n" << width << ' ' << height << "\n255\n";
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                color c = scale_color(pixel_at(x, y), samples);
                os << cast_color(c.x) << ' ' 
                   << cast_color(c.y) << ' ' 
                   << cast_color(c.z) << '\n';
            }
        }
    }
};

#endif