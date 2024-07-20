#pragma once
#include "BufferAttribute.h"
#include <string>
#include "Utils//utils.h"
#include <unordered_map>

class Renderer;
class Geometry {

	enum Type {
		Mesh,
		Mesh_Indexed
	};

	size_t size_ = 0;
	std::unordered_map<std::string, BufferAttribute> data_map_;
	std::vector<unsigned> indices_;
	unsigned EBO = 0;
	bool bReady_ = false;
	Type geometry_type_ = Mesh;


public:
	Geometry();

	std::vector<std::string> getAttributeNameList();

	BufferAttribute& getBufferData(const std::string& attr);

	void setAttribute(const std::string& attr, BufferAttribute& data,  bool isVertex = false);

	void setIndex(const std::vector<unsigned>& indices);

	void setupPipeline(Renderer& renderer);

	bool isMesh();

	bool isMeshIndexed();

	bool isPipelineReady();

	void setPipelineReady(bool ready);

	unsigned getVeticesNum();

	unsigned getIndicesNum();

	const unsigned* getIndicesRawData();

	unsigned getEBO();

	void setEBO(unsigned id);
};

class BoundingBox {
public:
	BoundingBox() = default;
	BoundingBox(const glm::vec3 &a, const glm::vec3 &b) : min_(a), max_(b) {}

	void getCorners(glm::vec3 *dst) const;
	BoundingBox transform(const glm::mat4 &matrix) const;
	bool intersects(const BoundingBox &box) const;
	void merge(const BoundingBox &box);

protected:
	static void updateMinMax(glm::vec3 *point, glm::vec3 *min, glm::vec3 *max);

public:
	glm::vec3 min_{0.f, 0.f, 0.f};
	glm::vec3 max_{0.f, 0.f, 0.f};
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