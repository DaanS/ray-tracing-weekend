#ifndef PDF_H
#define PDF_H

struct pdf {
    virtual double value(vec3 const& dir) const = 0;
    virtual vec3 generate() const = 0;
};

struct uniform_pdf : public pdf {
    virtual double value(vec3 const& dir) const override {
        return 1.0 / (4 * pi);
    }

    virtual vec3 generate() const override {
        return vec3_random_unit();
    }
};

struct cosine_pdf : public pdf {
    onb base;

    cosine_pdf(vec3 const& v) : base(v) { }

    virtual double value(vec3 const& dir) const override {
        auto cos = dot(normalize(dir), base.w);
        return cos <= 0 ? 0 : cos / pi;
    }

    virtual vec3 generate() const override {
        return base.local(vec3_random_cosine());
    }
};

struct hittable_pdf : public pdf {
    point o;
    std::shared_ptr<hittable> childe;

    hittable_pdf(std::shared_ptr<hittable> childe, point const& o) : childe(childe), o(o) { }

    virtual double value(vec3 const& dir) const override { return childe->pdf_value(o, dir); }
    virtual vec3 generate() const override { return childe->random_ray(o); }
};

struct mixture_pdf : public pdf {
    std::shared_ptr<pdf> p0, p1;

    mixture_pdf(std::shared_ptr<pdf> p0, std::shared_ptr<pdf> p1) : p0(p0), p1(p1) { }

    virtual double value(vec3 const& dir) const override {
        return 0.5 * p0->value(dir) + 0.5 * p1->value(dir);
    }

    virtual vec3 generate() const override {
        if (random_double() < 0.5) return p0->generate();
        else return p1->generate();
    }
};

#endif