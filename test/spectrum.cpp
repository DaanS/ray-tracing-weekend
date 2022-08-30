#include <gtest/gtest.h>
#include "spectrum.h"

TEST(Spectrum, FromSamples) {
    auto left = color_spectrum<60>::from_samples({900}, {1});
    for (auto s : left.samples) EXPECT_EQ(s, 1);

    auto right = color_spectrum<60>::from_samples({300}, {2});
    for (auto s : right.samples) EXPECT_EQ(s, 2);

    auto mid = color_spectrum<60>::from_samples({500}, {3});
    for (auto s : mid.samples) EXPECT_EQ(s, 3);

    std::vector<double> lambdas;
    std::vector<double> values;
    for (int i = 0; i < 61; ++i) {
        lambdas.push_back(400 + i * 5);
        values.push_back(i);
    }
    auto full = color_spectrum<60>::from_samples(lambdas, values);
    for (int i = 0; i < 60; ++i) {
        EXPECT_EQ(full.samples[i], (values[i] + values[i + 1]) / 2);
    }
}