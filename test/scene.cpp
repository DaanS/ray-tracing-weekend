#include <gtest/gtest.h>
#include "scene.h"

TEST(Scene, Json) {
    auto s4_scene = four_sphere_scene();
    json s4_json = s4_scene;
    scene s4_res;
    s4_json.get_to(s4_res);
    EXPECT_EQ(s4_scene, s4_res);
}

TEST(Scene, SaveLoad) {
    auto s4_scene = four_sphere_scene();
    s4_scene.save("tmp.json");
    auto s4_res = scene::load("tmp.json");
    EXPECT_EQ(s4_scene, s4_res);
}