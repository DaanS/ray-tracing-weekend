#ifndef BVH_H
#define BVH_H

#include <cassert>
#include <functional>
#include "aabb.h"
#include "hittable.h"
#include "hittable_list.h"

#include "sphere.h"

bool box_compare(std::shared_ptr<hittable> const h1, std::shared_ptr<hittable> const h2, size_t axis) {
    auto [res1, bb1] = h1->bounding_box(0, 0); // XXX shouldn't this consider the time stuff?
    auto [res2, bb2] = h2->bounding_box(0, 0);
    assert(res1 && res2);

    return bb1.min[axis] < bb2.min[axis];
}

struct bvh_node : public hittable {
    std::shared_ptr<hittable> left;
    std::shared_ptr<hittable> right;
    aabb bb;

    bvh_node(hittable_list const& list, double t0, double t1) : bvh_node(list.objects, 0, list.objects.size(), t0, t1) { }
    bvh_node(std::vector<std::shared_ptr<hittable>> const& src_objects, size_t start, size_t end, double t0, double t1) {
        auto axis = random_int(0, 2);
        size_t span = end - start;
        auto comp = std::bind(box_compare, std::placeholders::_1, std::placeholders::_2, axis);

        if (span == 1) {
            left = src_objects[start];
            right = src_objects[start];
        } else if (span == 2) {
            if (comp(src_objects[start], src_objects[start + 1])) {
                left = src_objects[start];
                right = src_objects[start + 1];
            } else {
                left = src_objects[start + 1];
                right = src_objects[start];
            }
        } else {
            auto objects = src_objects;
            std::sort(objects.begin() + start, objects.begin() + end, comp);
            auto mid = start + span / 2;
            left = std::make_shared<bvh_node>(objects, start, mid, t0, t1);
            right = std::make_shared<bvh_node>(objects, mid, end, t0, t1); 
        }

        auto [res1, bb1] = left->bounding_box(t0, t1);
        auto [res2, bb2] = right->bounding_box(t0, t1);
        assert(res1 && res2);

        bb = surrounding_box(bb1, bb2);
    }

    virtual bool hit(ray const& r, double t_min, double t_max, hit_record& h) const override {
        if (!bb.hit(r, t_min, t_max)) return false;

        auto hit_left = left->hit(r, t_min, t_max, h);
        auto hit_right = right->hit(r, t_min, (hit_left ? h.t : t_max), h);

        return hit_left || hit_right;
    }

    virtual std::tuple<bool, aabb> bounding_box(double t0, double t1) const override {
        return {true, bb};
    }
};

#endif