#include <sstream>
#include <gtest/gtest.h>
#include "color.h"

TEST(Color, WritePixel) {
    auto c = color(1, 0.5, 0);
    std::stringstream ss;
    write_color(ss, c);
    EXPECT_EQ(ss.str(), "255 127 0\n");
}