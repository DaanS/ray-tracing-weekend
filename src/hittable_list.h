#ifndef HITTABLE_LIST_H
#define HITTABLE_LIST_H

#include <memory>
#include <vector>
#include "hittable.h"
#include "sphere.h"

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

    virtual std::tuple<bool, aabb> bounding_box(double t0, double t1) const override {
        if (objects.empty()) return {false, aabb()};

        auto [res, bb] = objects[0]->bounding_box(t0, t1);
        for (int i = 1; i < objects.size(); ++i) {
            auto [res2, bb2] = objects[i]->bounding_box(t0, t1);
            res = res && res2;
            bb = surrounding_box(bb, bb2);
        }

        return {res, bb};
    }
};

// XXX support other hittables
void to_json(json& j, hittable_list const& list) {
    j = json::array();
    for (auto ptr : list.objects) {
        j.push_back(*std::dynamic_pointer_cast<sphere>(ptr));
    }
}

// XXX support other hittables
void from_json(json const& j, hittable_list& list) {
    for (auto& sphere_json : j) {
        sphere s;
        sphere_json.get_to(s);
        list.objects.push_back(std::make_shared<sphere>(s));
    }
}

#endif