#include <sstream>
#include <gtest/gtest.h>
#include "canvas.h"

TEST(Canvas, Basic) {
    canvas c(2, 2, 1);

    c.write_pixel(0, 0, color_rgb(1, 0, 0));
    c.write_pixel(1, 0, color_rgb(0, 1, 0));
    c.write_pixel(0, 1, color_rgb(0, 0, 1));
    c.write_pixel(1, 1, color_rgb(1, 1, 1));
    EXPECT_EQ(c.pixel_at(0, 0), color_rgb(1, 0, 0));
    EXPECT_EQ(c.pixel_at(1, 0), color_rgb(0, 1, 0));
    EXPECT_EQ(c.pixel_at(0, 1), color_rgb(0, 0, 1));
    EXPECT_EQ(c.pixel_at(1, 1), color_rgb(1, 1, 1));

    std::stringstream ss;
    c.to_ppm(ss);
    EXPECT_EQ(ss.str(), "P3\n2 2\n255\n255 0 0\n0 255 0\n0 0 255\n255 255 255\n");
}