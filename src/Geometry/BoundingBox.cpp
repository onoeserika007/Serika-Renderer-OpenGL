#include "Geometry/BoundingBox.h"

#include "Geometry/Ray.h"

void BoundingBox::getCorners(glm::vec3 *dst) const {
    dst[0] = glm::vec3(min_.x, max_.y, max_.z);
    dst[1] = glm::vec3(min_.x, min_.y, max_.z);
    dst[2] = glm::vec3(max_.x, min_.y, max_.z);
    dst[3] = glm::vec3(max_.x, max_.y, max_.z);

    dst[4] = glm::vec3(max_.x, max_.y, min_.z);
    dst[5] = glm::vec3(max_.x, min_.y, min_.z);
    dst[6] = glm::vec3(min_.x, min_.y, min_.z);
    dst[7] = glm::vec3(min_.x, max_.y, min_.z);
}

BoundingBox BoundingBox::transform(const glm::mat4 &matrix) const {
    BoundingBox newBBox;

    glm::vec3 corners[8];
    getCorners(corners);

    corners[0] = matrix * glm::vec4(corners[0], 1.f);
    for (auto & corner : corners) {
        corner = matrix * glm::vec4(corner, 1.f);
        newBBox.merge(corner);
    }
    return newBBox;
}

bool BoundingBox::overlaps(const BoundingBox &box) const {
    return  ((min_.x >= box.min_.x && min_.x <= box.max_.x) || (box.min_.x >= min_.x && box.min_.x <= max_.x)) &&
            ((min_.y >= box.min_.y && min_.y <= box.max_.y) || (box.min_.y >= min_.y && box.min_.y <= max_.y)) &&
            ((min_.z >= box.min_.z && min_.z <= box.max_.z) || (box.min_.z >= min_.z && box.min_.z <= max_.z));
}

bool BoundingBox::inside(const glm::vec3 &point) const {
    auto&& res = glm::greaterThan(point, min_) * glm::lessThan(point, max_);
    return res.x && res.y && res.z;
}

glm::vec3 BoundingBox::offset(const glm::vec3 &p) const {
    glm::vec3 offset = p - min_;
    if (max_.x > min_.x) offset.x /= max_.x - min_.x;
    if (max_.y > min_.y) offset.y /= max_.y - min_.y;
    if (max_.z > min_.z) offset.z /= max_.z - min_.z;
    return offset;
}

bool BoundingBox::intersectRay(const Ray &ray) const {
    // invDir: ray direction(x,y,z), invDir=(1.0/x,1.0/y,1.0/z), use this because Multiply is faster that Division
    // dirIsNeg: ray direction(x,y,z), dirIsNeg=[int(x>0),int(y>0),int(z>0)], use this to simplify your logic

    // t = (p - o) / dir
    glm::vec3 intesection_1 = (min_ - ray.origin) * ray.direction_inv; // in case
    glm::vec3 intesection_2 = (max_ - ray.origin) * ray.direction_inv;
    glm::vec3 tMin = glm::min(intesection_1, intesection_2);
    glm::vec3 tMax = glm::max(intesection_1, intesection_2);

    float tEnter = std::max(tMin.x, std::max(tMin.y, tMin.z)); // Enter Time
    float tExit = tExit = std::min(tMax.x, std::min(tMax.y, tMax.z)); // Exit Time
    return tExit >= 0.f && tEnter <= tExit;
}

glm::vec3 BoundingBox::Centroid() const {
    return (min_ + max_) * 0.5f;
}

glm::vec3 BoundingBox::Diagonal() const {
    return max_ - min_;
}

int BoundingBox::maxExtentAxis() const {
    glm::vec3 d = Diagonal();
    if (d.x > d.y && d.x > d.z)
        return 0;
    else if (d.y > d.z)
        return 1;
    else
        return 2;
}

float BoundingBox::SurfaceArea() const {
    glm::vec3 d = Diagonal();
    return 2 * (d.x * d.y + d.x * d.z + d.y * d.z);
}

BoundingBox &BoundingBox::merge(const glm::vec3 &point) {
    min_ = glm::min(min_, point);
    max_ = glm::max(max_, point);
    return *this;
}

BoundingBox &BoundingBox::merge(const BoundingBox &box) {
    min_ = glm::min(min_, box.min_);
    max_ = glm::max(max_, box.max_);
    return *this;
}

BoundingBox BoundingBox::merge(const BoundingBox &box1, const BoundingBox &box2) {
    return BoundingBox().merge(box1).merge(box2);
}
