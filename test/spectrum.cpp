#include <gtest/gtest.h>
#include "spectrum.h"

TEST(Spectrum, FromSamples) {
    auto left = color_spectrum::from_samples(std::vector<spectrum_sample>{{900, 1}});
    for (auto s : left.samples) EXPECT_EQ(s, 1);

    auto right = color_spectrum::from_samples(std::vector<spectrum_sample>{{300, 2}});
    for (auto s : right.samples) EXPECT_EQ(s, 2);

    auto mid = color_spectrum::from_samples(std::vector<spectrum_sample>{{500, 3}});
    for (auto s : mid.samples) EXPECT_EQ(s, 3);

    std::vector<spectrum_sample> samples;
    for (int i = 0; i < color_spectrum::sample_count + 1; ++i) {
        samples.push_back(spectrum_sample{400.0 + i * 5, (double) i});
    }
    auto full = color_spectrum::from_samples(samples);
    for (int i = 0; i < color_spectrum::sample_count; ++i) {
        EXPECT_EQ(full.samples[i], (samples[i].value + samples[i + 1].value) / 2);
        EXPECT_EQ(full.samples[i], full.value_at(402.5 + i * 5));
    }

    auto mixed = color_spectrum::from_samples(std::vector<spectrum_sample>{{407, 2}, {412, 3}, {423, 5}});
    EXPECT_EQ(mixed.samples[0], 2);
    EXPECT_EQ(mixed.samples[1], (2 * 2 + 3 * 0.5 * (2 + std::lerp(2, 3, 0.6))) / 5.0);
    EXPECT_EQ(mixed.samples[2], (2 * 0.5 * (std::lerp(2, 3, 0.6) + 3) + 3 * 0.5 * (3 + std::lerp(3, 5, 3.0 / 11))) / 5.0);
}

TEST(Spectrum, Json) {
    color_spectrum full;
    for (int i = 0; i < color_spectrum::sample_count; ++i) {
        full.samples[i] = i;
    }
    json j = full;
    color_spectrum res;
    j.get_to(res);
    for (int i = 0; i < color_spectrum::sample_count; ++i) {
        EXPECT_EQ(full.samples[i], res.samples[i]);
    }
}

TEST(Spectrum, Conversions) {
    vec3 xyz{0.2, 0.4, 0.6};
    EXPECT_EQ(rgb_to_xyz(xyz_to_rgb(xyz)), xyz);

    vec3 rgb{0, 0.5, 1};
    EXPECT_TRUE(compare(xyz_to_rgb(rgb_to_xyz(rgb)), rgb, 0.01));

    color_spectrum white;
    white.samples.fill(1);
    EXPECT_TRUE(compare(white.to_xyz(), vec3(1, 1, 1), 0.01));
    EXPECT_TRUE(compare(white.to_rgb_v(), vec3(1, 1, 1), 0.2)); // XXX YIKES

    color_spectrum black;
    white.samples.fill(0);
    EXPECT_EQ(white.to_xyz(), vec3(0, 0, 0));
    EXPECT_EQ(white.to_rgb(), vec3(0, 0, 0));

    EXPECT_TRUE(compare(color_spectrum::get_rgb_base(spectrum_type::illuminant, rgb_base::red).to_rgb_v(), vec3(1, 0, 0), 0.05));
    EXPECT_TRUE(compare(color_spectrum::get_rgb_base(spectrum_type::reflectance, rgb_base::cyan).to_rgb_v(), vec3(0, 1, 1), 0.05));

    EXPECT_TRUE(compare(color_spectrum::from_rgb(vec3(1, 0, 1), spectrum_type::reflectance).to_rgb_v(), vec3(1, 0, 1), 0.25)); // XXX YIKES
    EXPECT_TRUE(compare(color_spectrum::from_xyz(vec3(0, 1, 1), spectrum_type::illuminant).to_xyz(), vec3(0, 1, 1), 0.15)); // XXX YIKES
}

//TEST(Spectrum, RgbHex) {
//    EXPECT_EQ(color_rgb(0x00ff0000u), color_rgb(1, 0, 0));
//    EXPECT_EQ(color_rgb(0x00204060u), color_rgb(32.0 / 255, 64.0 / 255, 96.0 / 255));
//}