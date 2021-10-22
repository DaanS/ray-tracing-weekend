#ifndef HITTABLE_LIST_H
#define HITTABLE_LIST_H

#include <memory>
#include <vector>
#include "hittable.h"

struct hittable_list : public hittable {
    std::vector<std::shared_ptr<hittable>> objects;

    hittable_list() {}
    hittable_list(std::shared_ptr<hittable> h) { add(h); }

    void clear() { objects.clear(); }
    void add(std::shared_ptr<hittable> object) { objects.push_back(object); }
    template<typename T, typename... Args>
    void make(Args... args) { objects.push_back(std::make_shared<T>(std::forward<Args>(args)...)); }

    virtual bool hit(ray const& r, double t_min, double t_max, hit_record& res) const override {
        hit_record h;
        bool any_hit = false;
        auto closest = t_max;

        for (auto const& object : objects) {
            if (object->hit(r, t_min, closest, h)) {
                any_hit = true;
                closest = h.t;
                res = h;
            }
        }

        return any_hit;
    }
};

#endif