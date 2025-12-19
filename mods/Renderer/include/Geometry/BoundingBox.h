#ifndef SERIKA_RENDERER_INCLUDE_GEOMETRY_BOUNDINGBOX_H
#define SERIKA_RENDERER_INCLUDE_GEOMETRY_BOUNDINGBOX_H
#include "Base/Globals.h"
#include "Utils/UUID.h"

struct Ray;

class BoundingBox {
public:
    BoundingBox(): min_(FLOAT_MAX), max_(FLOAT_MIN) {

    }

    // 错误的bbox写法，不能直接用a b来初始化min max，因为不知都谁大谁小
    BoundingBox(const glm::vec3 &a, const glm::vec3 &b): min_(FLOAT_MAX), max_(FLOAT_MIN) {
        merge(a);
        merge(b);
    }

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
    NO_DISCARD int getUUID() const;


    BoundingBox &merge(const BoundingBox &box);
    BoundingBox &merge(const glm::vec3& point);
    static BoundingBox merge(const BoundingBox &box1, const BoundingBox &box2);

public:
    glm::vec3 min_;
    glm::vec3 max_;

protected:

private:
    Serika::UUID<BoundingBox> uuid_;
};

#endif // SERIKA_RENDERER_INCLUDE_GEOMETRY_BOUNDINGBOX_H
