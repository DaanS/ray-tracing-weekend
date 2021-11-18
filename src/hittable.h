#ifndef HITTABLE_H
#define HITTABLE_H

#include "aabb.h"
#include "ray.h"
#include "vec3.h"

struct material;

struct hit_record {
    point p;
    vec3 n;
    std::shared_ptr<material> mat_ptr;
    double t;
    double u;
    double v;
    bool front_face;

    void set_face_normal(ray const& r, vec3 const& n_out) {
        front_face = dot(r.direction, n_out) < 0;
        n = front_face ? n_out : -n_out;
    }
};

using nlohmann::json;

struct sphere;
struct moving_sphere;
struct translate;
struct rotate_y;
struct bvh_node;
struct xz_rect;
struct box;
struct constant_medium;
struct hittable {
    virtual bool hit(ray const& r, double t_min, double t_max, hit_record& rec) const = 0;
    virtual std::tuple<bool, aabb> bounding_box(double t0, double t1) const = 0;

    virtual std::string name() const {
        return "";
    }

    virtual json to_json() const { throw std::logic_error("to_json not implemented for this hittable type"); }
    virtual bool equals(hittable const& rhs) const { throw std::logic_error("equals not implemented for this hittable type"); }
    bool operator==(hittable const& rhs) const { return this->equals(rhs); }
    virtual void print(std::ostream& os) const { throw std::logic_error("print not implemented for this hittable type"); }
    friend std::ostream& operator<<(std::ostream& os, hittable const& h) { h.print(os); return os; }

    static std::shared_ptr<hittable> make_from_json(json const& j) {
        auto type = j.at("type");
        if (type == "sphere") return std::dynamic_pointer_cast<hittable>(std::make_shared<sphere>(j));
        else if (type == "moving_sphere") return std::dynamic_pointer_cast<hittable>(std::make_shared<moving_sphere>(j));
        else if (type == "translate") return std::dynamic_pointer_cast<hittable>(std::make_shared<translate>(j));
        else if (type == "rotate_y") return std::dynamic_pointer_cast<hittable>(std::make_shared<rotate_y>(j));
        else if (type == "bvh_node") return std::dynamic_pointer_cast<hittable>(std::make_shared<bvh_node>(j));
        else if (type == "xz_rect") return std::dynamic_pointer_cast<hittable>(std::make_shared<xz_rect>(j));
        else if (type == "box") return std::dynamic_pointer_cast<hittable>(std::make_shared<box>(j));
        else if (type == "constant_medium") return std::dynamic_pointer_cast<hittable>(std::make_shared<constant_medium>(j));
        std::cerr << "unknown hittable: " << type << std::endl;
        return nullptr;
    }
};

void to_json(json& j, hittable const& h) {
    j = h.to_json();
}

struct translate : public hittable {
    std::shared_ptr<hittable> childe;
    vec3 offset;

    translate(std::shared_ptr<hittable> childe, vec3 const& offset) : childe(childe), offset(offset) { }
    translate() : translate(nullptr, vec3(0, 0, 0)) { }
    translate(json const& j) {
        childe = hittable::make_from_json(j.at("child"));
        j.at("offset").get_to(offset);
    }

    bool hit(ray const& r, double t_min, double t_max, hit_record& h) const override {
        ray r_moved(r.origin - offset, r.direction, r.time);
        if (!childe->hit(r_moved, t_min, t_max, h)) return false;

        h.p += offset;
        h.set_face_normal(r_moved, h.n);
        return true;
    }

    std::tuple<bool, aabb> bounding_box(double t0, double t1) const override {
        auto [res, bb] = childe->bounding_box(t0, t1);
        return {res, aabb(bb.min + offset, bb.max + offset)};
    }

    json to_json() const override {
        return json{
            {"type", "translate"},
            {"child", *childe},
            {"offset", offset}
        };
    }

    bool equals(hittable const& rhs) const override {
        if (typeid(*this) != typeid(rhs)) return false;
        auto that = static_cast<translate const&>(rhs);
        return *childe == *that.childe && offset == that.offset;
    }

    void print(std::ostream& os) const override {
        os << "translate: {child: " << *childe << ", offset: " << offset << "}";
    }
};

struct rotate_y : public hittable {
    std::shared_ptr<hittable> childe;
    double sin_theta;
    double cos_theta;
    bool hasbox;
    aabb bb;
    double angle;

    void construct() {
        auto rad = radians(angle);
        sin_theta = sin(rad);
        cos_theta = cos(rad);
        std::tie(hasbox, bb) = childe->bounding_box(0, 1);

        point min(inf, inf, inf);
        point max(-inf, -inf, -inf);

        for (int i = 0; i < 2; ++i) {
            for (int j = 0; j < 2; ++j) {
                for (int k = 0; k < 2; ++k) {
                    auto x = i * bb.max.x + (1 - i) * bb.min.x;
                    auto y = j * bb.max.y + (1 - j) * bb.min.y;
                    auto z = k * bb.max.z + (1 - k) * bb.min.z;

                    auto x_new = cos_theta * x + sin_theta * z;
                    auto z_new = -sin_theta * x + cos_theta * z;

                    vec3 tester(x_new, y, z_new);

                    for (int c = 0; c < 3; ++c) {
                        min[c] = std::min(min[c], tester[c]);
                        max[c] = std::max(max[c], tester[c]);
                    }
                }
            }
        }

        bb = aabb(min, max);
    }

    rotate_y(std::shared_ptr<hittable> childe, double angle) : childe(childe), angle(angle) { construct(); }
    rotate_y() : rotate_y(nullptr, 0) { }
    rotate_y(json const& j) {
        childe = hittable::make_from_json(j.at("child"));
        j.at("angle").get_to(angle);
        construct();
    }

    virtual bool hit(ray const& r, double t_min, double t_max, hit_record& h) const override {
        auto origin = r.origin;
        auto direction = r.direction;

        origin[0] = cos_theta * r.origin[0] - sin_theta * r.origin[2]; 
        origin[2] = sin_theta * r.origin[0] + cos_theta * r.origin[2]; 

        direction[0] = cos_theta * r.direction[0] - sin_theta * r.direction[2]; 
        direction[2] = sin_theta * r.direction[0] + cos_theta * r.direction[2]; 

        ray r_rot(origin, direction, r.time);

        if (!childe->hit(r_rot, t_min, t_max, h)) return false;

        auto p = h.p;
        auto n = h.n;

        p[0] = cos_theta * h.p[0] + sin_theta * h.p[2];
        p[2] = -sin_theta * h.p[0] + cos_theta * h.p[2];

        n[0] = cos_theta * h.n[0] + sin_theta * h.n[2];
        n[2] = -sin_theta * h.n[0] + cos_theta * h.n[2];

        h.p = p;
        h.set_face_normal(r_rot, n);

        return true;
    }

    virtual std::tuple<bool, aabb> bounding_box(double t0, double t1) const override {
        return {hasbox, bb};
    }

    virtual json to_json() const override {
        return json{
            {"type", "rotate_y"},
            {"child", *childe},
            {"angle", angle}
        };
    }

    virtual bool equals(hittable const& rhs) const override {
        if (typeid(*this) != typeid(rhs)) return false;
        auto that = static_cast<rotate_y const&>(rhs);
        return *childe == *that.childe && angle == that.angle;
    }

    virtual void print(std::ostream& os) const override {
        os << "rotate_y: {child: " << *childe << ", angle: " << angle << "}";
    }
};

#endif