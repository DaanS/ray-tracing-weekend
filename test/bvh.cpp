#include "test.h"
#include "bvh.h"
#include "sphere.h"

TEST(Bvh, Basic) {
    hittable_list list;

    auto s1 = std::make_shared<sphere>(point(0, 0, 0), 1);
    list.add(s1);
    auto bvh = bvh_node(list, 0, 1);
    auto [res1, bb1] = s1->bounding_box(0, 1);
    EXPECT_EQ(bb1, bvh.bb);

    auto s2 = std::make_shared<sphere>(point(3, 2, 1), 5);
    list.add(s2);
    bvh = bvh_node(list, 0, 5);
    auto [res2, bb2] = s2->bounding_box(0, 1);
    EXPECT_EQ(surrounding_box(bb1, bb2), bvh.bb);
}

int count(std::shared_ptr<bvh_node> bvh, std::shared_ptr<sphere> obj) {
    int res = 0;
    auto left_sphere = std::dynamic_pointer_cast<sphere>(bvh->left);
    if (left_sphere) {
        if (*left_sphere == *obj) ++res;
    } else res += count(std::dynamic_pointer_cast<bvh_node>(bvh->left), obj);
    auto right_sphere = std::dynamic_pointer_cast<sphere>(bvh->right);
    if (right_sphere) {
        if (*right_sphere == *obj && (!left_sphere || *left_sphere != *right_sphere)) ++res;
    } else res += count(std::dynamic_pointer_cast<bvh_node>(bvh->right), obj);
    return res;
}

TEST(Bvh, Order) {
    hittable_list list;
    for (int y = 0; y < 10; ++y) {
        for (int x = 0; x < 10; ++x) {
            list.make<sphere>(point(x, y, 0), 0.5);
        }
    }
    auto bvh = std::make_shared<bvh_node>(list, 0, 1);

    for (auto obj_ptr : list.objects) {
        EXPECT_EQ(count(bvh, std::dynamic_pointer_cast<sphere>(obj_ptr)), 1);
    }
}