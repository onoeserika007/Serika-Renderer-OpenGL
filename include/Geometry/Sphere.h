#pragma once

#include "BoundingBox.h"
#include "Geometry/Primitives.h"

struct MaterialInfo;

class Sphere final : public Intersectable {
    glm::vec3 center_ = {};
    float radius = 0.f;
    float radius2 = 0.f;
    MaterialInfo material_info_;
    float area = 0.f;

    Sphere(const glm::vec3 &center, float radius, const MaterialInfo& mt = {});

    virtual ~Sphere() override;

    Intersection getIntersection(const Ray &ray) override;

    NO_DISCARD BoundingBox getBounds() const override;
    NO_DISCARD float getArea() const override;
    NO_DISCARD bool hasEmit() const override;
    NO_DISCARD bool intersect(const Ray& ray) override;

    void Sample(Intersection &intersection, float &pdf) override;

    virtual void transform(const glm::mat4 &trans) override;
};
