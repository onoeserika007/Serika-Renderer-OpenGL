#ifndef SERIKA_RENDERER_INCLUDE_GEOMETRY_SPHERE_H
#define SERIKA_RENDERER_INCLUDE_GEOMETRY_SPHERE_H

#include <glm/glm.hpp>
#include "Geometry/Primitives.h"
#include "Geometry/BoundingBox.h"

class FMaterial;

class Sphere final : public Intersectable {
    glm::vec3 center_ = {};
    float radius = 0.f;
    float radius2 = 0.f;
    std::weak_ptr<FMaterial> material_;
    float area = 0.f;

    Sphere(const glm::vec3 &center, float radius, const std::shared_ptr<FMaterial>& mat = {});

    virtual ~Sphere() override;

    Intersection getIntersection(const Ray &ray) override;

    BoundingBox getBounds() const override;
    float getArea() const override;
    bool hasEmit() const override;
    bool intersect(const Ray& ray) override;

    void Sample(Intersection &intersection, float &pdf) override;

    virtual void transform(const glm::mat4 &trans) override;
};

#endif // SERIKA_RENDERER_INCLUDE_GEOMETRY_SPHERE_H
