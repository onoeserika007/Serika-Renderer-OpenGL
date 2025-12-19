#ifndef SERIKA_RENDERER_INCLUDE_GEOMETRY_GEOMETRY_H
#define SERIKA_RENDERER_INCLUDE_GEOMETRY_GEOMETRY_H
#include "BufferAttribute.h"
#include <string>
#include <unordered_map>

#include "BoundingBox.h"

class Triangle;
class BoundingBox;
class Renderer;

class FGeometry: public PipelineLoadable {
    enum MeshType {
        Mesh,
        Mesh_Indexed
    };

    std::unordered_map<EBufferAttribute, BufferAttribute> data_map_;
    std::vector<unsigned> indices_;
    bool bReady_ = false;
    MeshType mesh_type_ = Mesh;

    Serika::UUID<FGeometry> uuid_;

public:
    FGeometry();

    mutable unsigned EBO = 0;
    mutable unsigned VAO = 0;

    static std::string getAttributeName(EBufferAttribute type);

    void setAttribute(EBufferAttribute attr, const BufferAttribute &data, bool isVertex = false);
    void setIndex(const std::vector<unsigned> &indices);
    void setupPipeline(const Renderer &renderer) { renderer.setupGeometry(*this); }
    void setPipelineReady(bool ready) { bReady_ = ready; }
    void setEBO(unsigned id) { EBO = id; }

    const BufferAttribute &getBufferAttribute(EBufferAttribute attr);

    std::unordered_map<EBufferAttribute, BufferAttribute> &getBufferData() { return data_map_; }
    bool isMesh() const { return mesh_type_ == Mesh; }
    bool isMeshIndexed() const { return mesh_type_ == Mesh_Indexed; }
    bool isPipelineReady() const { return bReady_; }
    unsigned getVeticesNum() const;
    unsigned getIndicesNum() const { return indices_.size(); }
    unsigned getTriangleNum() const;

    std::unique_ptr<Triangle> fetchTriangle(unsigned k) const;
    const unsigned *getIndicesRawData() const;
    unsigned getEBO() const { return EBO; }
};

class Plane {
public:
    enum PlaneIntersects {
        Intersects_Cross = 0,
        Intersects_Tangent = 1,
        Intersects_Front = 2,
        Intersects_Back = 3
    };

    void set(const glm::vec3 &n, const glm::vec3 &pt) {
        normal_ = glm::normalize(n);
        d_ = -(glm::dot(normal_, pt));
    }

    float distance(const glm::vec3 &pt) const {
        return glm::dot(normal_, pt) + d_;
    }

    const glm::vec3 &getNormal() const {
        return normal_;
    }

    Plane::PlaneIntersects intersects(const BoundingBox &box) const;

    // check intersect with point (world space)
    Plane::PlaneIntersects intersects(const glm::vec3 &p0) const;

    // check intersect with line segment (world space)
    Plane::PlaneIntersects intersects(const glm::vec3 &p0, const glm::vec3 &p1) const;

    // check intersect with triangle (world space)
    Plane::PlaneIntersects intersects(const glm::vec3 &p0, const glm::vec3 &p1, const glm::vec3 &p2) const;

private:
    // Ax + By + Cz + D = 0
    glm::vec3 normal_;
    float d_ = 0;
};

struct Frustum {
public:
    bool intersects(const BoundingBox &box) const;

    // check intersect with point (world space)
    bool intersects(const glm::vec3 &p0) const;

    // check intersect with line segment (world space)
    bool intersects(const glm::vec3 &p0, const glm::vec3 &p1) const;

    // check intersect with triangle (world space)
    bool intersects(const glm::vec3 &p0, const glm::vec3 &p1, const glm::vec3 &p2) const;

public:
    /**
     * planes[0]: near;
     * planes[1]: far;
     * planes[2]: top;
     * planes[3]: bottom;
     * planes[4]: left;
     * planes[5]: right;
     */
    Plane planes[6];

    /**
     * corners[0]: nearTopLeft;
     * corners[1]: nearTopRight;
     * corners[2]: nearBottomLeft;
     * corners[3]: nearBottomRight;
     * corners[4]: farTopLeft;
     * corners[5]: farTopRight;
     * corners[6]: farBottomLeft;
     * corners[7]: farBottomRight;
     */
    glm::vec3 corners[8];

    BoundingBox bbox;
};

enum FrustumClipMask {
    POSITIVE_X = 1 << 0,
    NEGATIVE_X = 1 << 1,
    POSITIVE_Y = 1 << 2,
    NEGATIVE_Y = 1 << 3,
    POSITIVE_Z = 1 << 4,
    NEGATIVE_Z = 1 << 5,
};

const int FrustumClipMaskArray[6] = {
    FrustumClipMask::POSITIVE_X,
    FrustumClipMask::NEGATIVE_X,
    FrustumClipMask::POSITIVE_Y,
    FrustumClipMask::NEGATIVE_Y,
    FrustumClipMask::POSITIVE_Z,
    FrustumClipMask::NEGATIVE_Z,
};

const glm::vec4 FrustumClipPlane[6] = {
    {-1, 0, 0, 1},
    {1, 0, 0, 1},
    {0, -1, 0, 1},
    {0, 1, 0, 1},
    {0, 0, -1, 1},
    {0, 0, 1, 1}
};

#endif // SERIKA_RENDERER_INCLUDE_GEOMETRY_GEOMETRY_H
