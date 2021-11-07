#include <gtest/gtest.h>
#include "texture.h"
#include "vec3.h"

TEST(Texture, SolidColor) {
    auto tex = solid_color(1, 0.5, 0);
    EXPECT_EQ(tex.value(0.2, 0.8, point(0, 0, 0)), color(1, 0.5, 0));

    EXPECT_EQ(tex, tex);
    auto tex2 = solid_color(0, 0, 0);
    EXPECT_NE(tex, tex2);
    auto tex3 = checker(color(1, 1, 1), color(0, 0, 0));
    EXPECT_NE(tex, tex3);
}