#include <sstream>
#include <gtest/gtest.h>
#include "color.h"

TEST(Color, WritePixel) {
    auto c = color(1, 0.5, 0);
    std::stringstream ss;
    write_color(ss, c, 1);
    EXPECT_EQ(ss.str(), "255 181 0\n");
}

TEST(Color, Multisample) {
    auto c = color(4, 3, 2);
    std::stringstream ss;
    write_color(ss, c, 4);
    EXPECT_EQ(ss.str(), "255 221 181\n");
}