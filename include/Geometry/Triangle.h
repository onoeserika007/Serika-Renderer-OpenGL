#pragma once
#include "BoundingBox.h"
#include "Primitives.h"
#include "Base/Globals.h"
#include "Material/FMaterial.h"

class Triangle : public Intersectable
{
public:
    glm::vec3 v0_{}, v1_{}, v2_{}; // vertices A, B ,C , counter-clockwise order
    glm::vec3 e1_{}, e2_{};     // 2 edges v1-v0, v2-v0;
    glm::vec2 t0_{}, t1_{}, t2_{}; // texture coords
    glm::vec3 normal_{};
    float area_{};
    MaterialInfo material_info_{};
    std::weak_ptr<FMaterial> material_;

    Triangle();
    virtual ~Triangle() override;

    Triangle(const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2, const MaterialInfo& m = {});

    bool intersect(const Ray& ray) override;

    virtual Intersection getIntersection(const Ray &ray) override;

    BoundingBox getBounds() const override;

    // 三角形内的均匀采样
    void Sample(Intersection &pos, float &pdf);

    float getArea() const;

    bool hasEmit() const;
};
