#include "Geometry/Geometry.h"

#include "Geometry/BoundingBox.h"
#include "Geometry/Triangle.h"

FGeometry::FGeometry() : mesh_type_(Mesh) {
}

std::string FGeometry::getAttributeName(EBufferAttribute type) {
    switch (type) {
        case EBA_Position: return "aPos";
        case EBA_TexCoord: return "aTexCoord";
        case EBA_Normal:   return "aNormal";
        default: {
            throw std::runtime_error("[FGeometry::getAttributeName] Unmatched type");
        }
    }
}

const BufferAttribute &FGeometry::getBufferAttribute(EBufferAttribute attr) {
    static BufferAttribute empty{};
    if (data_map_.contains(attr)) return data_map_[attr];
    return empty;
}

void FGeometry::setAttribute(EBufferAttribute attr, const BufferAttribute &data, const bool isVertex) {
    //if(attr != "position" && size_ && size_ != data.size())

    data_map_[attr] = data;
}

void FGeometry::setIndex(const std::vector<unsigned> &indices) {
    mesh_type_ = Mesh_Indexed;
    indices_.assign(indices.begin(), indices.end());
};

unsigned FGeometry::getVeticesNum() const {
    if (data_map_.contains(EBA_Position)) return data_map_.at(EBA_Position).size();
    return 0;
}

unsigned FGeometry::getTriangleNum() const {
    if (isMesh()) return getVeticesNum() / 3;
    if (isMeshIndexed()) return getIndicesNum() / 3;
    return 0;
}

std::unique_ptr<Triangle> FGeometry::fetchTriangle(unsigned k) const {
    if (isMeshIndexed() && data_map_.contains(EBA_Position)) {
        unsigned idx0 = indices_[k * 3];
        unsigned idx1 = indices_[k * 3 + 1];
        unsigned idx2 = indices_[k * 3 + 2];
        auto&& pos = data_map_.at(EBA_Position);
        glm::vec3 v0 {pos[idx0 * 3], pos[idx0 * 3 + 1], pos[idx0 * 3 + 2]};
        glm::vec3 v1 {pos[idx1 * 3], pos[idx1 * 3 + 1], pos[idx1 * 3 + 2]};;
        glm::vec3 v2 {pos[idx2 * 3], pos[idx2 * 3 + 1], pos[idx2 * 3 + 2]};

        auto ret = std::make_unique<Triangle>(v0, v1, v2);
        if (data_map_.contains(EBA_TexCoord)) {
            auto&& tex = data_map_.at(EBA_TexCoord);
            ret->t0_ = glm::vec2(tex[idx0 * 2], tex[idx0 * 2 + 1]);
            ret->t1_ = glm::vec2(tex[idx1 * 2], tex[idx1 * 2 + 1]);
            ret->t2_ = glm::vec2(tex[idx2 * 2], tex[idx2 * 2 + 1]);
        };

        if (data_map_.contains(EBA_Normal)) {
            auto&& normal = data_map_.at(EBA_Normal);
            ret->n0_ = glm::vec3(normal[idx0 * 3], normal[idx0 * 3 + 1], normal[idx0 * 3 + 2]);
            ret->n1_ = glm::vec3(normal[idx1 * 3], normal[idx1 * 3 + 1], normal[idx1 * 3 + 2]);
            ret->n2_ = glm::vec3(normal[idx2 * 3], normal[idx2 * 3 + 1], normal[idx2 * 3 + 2]);
        }

        return std::move(ret);
    }
    return {};
}

const unsigned *FGeometry::getIndicesRawData() const {
    return indices_.data();
}

Plane::PlaneIntersects Plane::intersects(const BoundingBox &box) const {
    glm::vec3 center = (box.min_ + box.max_) * 0.5f;
    glm::vec3 extent = (box.max_ - box.min_) * 0.5f;
    float d = distance(center);
    // Approximately
    float r = fabsf(extent.x * normal_.x) + fabsf(extent.y * normal_.y) + fabsf(extent.z * normal_.z);
    if (d == r) {
        return Plane::Intersects_Tangent;
    } else if (std::abs(d) < r) {
        return Plane::Intersects_Cross;
    }

    return (d > 0.0f) ? Plane::Intersects_Front : Plane::Intersects_Back;
}

Plane::PlaneIntersects Plane::intersects(const glm::vec3 &p0) const {
    float d = distance(p0);
    if (d == 0) {
        return Plane::Intersects_Tangent;
    }
    return (d > 0.0f) ? Plane::Intersects_Front : Plane::Intersects_Back;
}

Plane::PlaneIntersects Plane::intersects(const glm::vec3 &p0, const glm::vec3 &p1) const {
    Plane::PlaneIntersects state0 = intersects(p0);
    Plane::PlaneIntersects state1 = intersects(p1);

    if (state0 == state1) {
        return state0;
    }

    if (state0 == Plane::Intersects_Tangent || state1 == Plane::Intersects_Tangent) {
        return Plane::Intersects_Tangent;
    }

    return Plane::Intersects_Cross;
}

Plane::PlaneIntersects Plane::intersects(const glm::vec3 &p0, const glm::vec3 &p1, const glm::vec3 &p2) const {
    Plane::PlaneIntersects state0 = intersects(p0, p1);
    Plane::PlaneIntersects state1 = intersects(p0, p2);
    Plane::PlaneIntersects state2 = intersects(p1, p2);

    if (state0 == state1 && state0 == state2) {
        return state0;
    }

    if (state0 == Plane::Intersects_Cross || state1 == Plane::Intersects_Cross || state2 == Plane::Intersects_Cross) {
        return Plane::Intersects_Cross;
    }

    return Plane::Intersects_Tangent;
}

bool Frustum::intersects(const BoundingBox &box) const {
    for (auto &plane: planes) {
        if (plane.intersects(box) == Plane::Intersects_Back) {
            return false;
        }
    }

    // check box intersects
    if (!bbox.overlaps(box)) {
        return false;
    }

    return true;;
}

bool Frustum::intersects(const glm::vec3 &p0) const {
    for (auto &plane: planes) {
        if (plane.intersects(p0) == Plane::Intersects_Back) {
            return false;
        }
    };

    return true;
}

bool Frustum::intersects(const glm::vec3 &p0, const glm::vec3 &p1) const {
    for (auto &plane: planes) {
        if (plane.intersects(p0, p1) == Plane::Intersects_Back) {
            return false;
        }
    }

    return true;
}

bool Frustum::intersects(const glm::vec3 &p0, const glm::vec3 &p1, const glm::vec3 &p2) const {
    for (auto &plane: planes) {
        if (plane.intersects(p0, p1, p2) == Plane::Intersects_Back) {
            return false;
        }
    }

    return true;
}
