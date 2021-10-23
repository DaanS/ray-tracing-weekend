#include <gtest/gtest.h>
#include "camera.h"

TEST(Camera, GetRay) {
    camera cam;

    EXPECT_EQ(cam.get_ray(0, 0).direction, vec3(-1 * cam.aspect_ratio, -1, -1));
    EXPECT_EQ(cam.get_ray(1, 1).direction, vec3(1 * cam.aspect_ratio, 1, -1));
}