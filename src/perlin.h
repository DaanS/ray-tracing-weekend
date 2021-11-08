#ifndef PERLIN_H
#define PERLIN_H

#include <algorithm>
#include <array>
#include "util.h"
#include "vec3.h"

struct perlin {
    static constexpr size_t count = 256;
    std::array<vec3, count> random_jar;
    std::array<int, count> perm_x;
    std::array<int, count> perm_y;
    std::array<int, count> perm_z;

    perlin() {
        std::generate(random_jar.begin(), random_jar.end(), [](){ return normalize(vec3_random(-1, 1)); });

        std::iota(perm_x.begin(), perm_x.end(), 0);
        std::iota(perm_y.begin(), perm_y.end(), 0);
        std::iota(perm_z.begin(), perm_z.end(), 0);

        static std::minstd_rand gen;
        std::shuffle(perm_x.begin(), perm_x.end(), gen);
        std::shuffle(perm_y.begin(), perm_y.end(), gen);
        std::shuffle(perm_z.begin(), perm_z.end(), gen);
    }

    double noise(point const& p) const {
        auto u = p.x - floor(p.x);
        auto v = p.y - floor(p.y);
        auto w = p.z - floor(p.z);

        auto i = static_cast<int>(floor(p.x));
        auto j = static_cast<int>(floor(p.y));
        auto k = static_cast<int>(floor(p.z));

        vec3 c[2][2][2];
        for (int di = 0; di < 2; ++di) {
            for (int dj = 0; dj < 2; ++dj) {
                for (int dk = 0; dk < 2; ++dk) {
                    c[di][dj][dk] = random_jar[
                        perm_x[(i + di) & 255] ^
                        perm_y[(j + dj) & 255] ^
                        perm_z[(k + dk) & 255]
                    ];
                }
            }
        }

        return trilerp(c, u, v, w);
    }

    double turbulence(point const& p, int depth = 7) const {
        auto acc = 0.0;
        auto temp_p = p;
        auto weight = 1.0;

        for (int i = 0; i < depth; ++i) {
            acc += weight * noise(temp_p);
            weight *= 0.5;
            temp_p *= 2;
        }

        return std::abs(acc);
    }

    static double trilerp(vec3 c[2][2][2], double u, double v, double w) {
        auto uu = u * u * (3 - 2 * u);
        auto vv = v * v * (3 - 2 * v);
        auto ww = w * w * (3 - 2 * w);
        auto acc = 0.0;
        for (int i = 0; i < 2; ++i) {
            for (int j = 0; j < 2; ++j) {
                for (int k = 0; k < 2; ++k) {
                    vec3 weight_v(u - i, v - j, w - k);
                    acc += (i * uu + (1 - i) * (1 - uu)) *
                            (j * vv + (1 - j) * (1 - vv)) *
                            (k * ww + (1 - k) * (1 - ww)) * dot(c[i][j][k], weight_v);
                }
            }
        }

        return acc;
    }
};

#endif