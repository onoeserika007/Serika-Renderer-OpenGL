#pragma once
#include "BoundingBox.h"
#include "BoundingBox.h"
#include "Base/Globals.h"

struct Ray;

class BoundingBox {
public:
    BoundingBox() = default;
    BoundingBox(const glm::vec3 &a, const glm::vec3 &b) : min_(a), max_(b) {}

    void getCorners(glm::vec3 *dst) const;
    NO_DISCARD BoundingBox transform(const glm::mat4 &matrix) const;
    NO_DISCARD bool overlaps(const BoundingBox &box) const;
    NO_DISCARD bool inside(const glm::vec3& point) const;
    NO_DISCARD bool intersectRay(const Ray& ray) const;

    NO_DISCARD glm::vec3 offset(const glm::vec3& p) const;
    NO_DISCARD glm::vec3 Centroid() const;
    NO_DISCARD glm::vec3 Diagonal() const;
    NO_DISCARD int maxExtentAxis() const;
    NO_DISCARD float SurfaceArea() const;


    BoundingBox &merge(const BoundingBox &box);
    BoundingBox &merge(const glm::vec3& point);
    static BoundingBox merge(const BoundingBox &box1, const BoundingBox &box2);

protected:

public:
    glm::vec3 min_{0.f, 0.f, 0.f};
    glm::vec3 max_{0.f, 0.f, 0.f};
};
