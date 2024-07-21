#include "Geometry/Geometry.h"

Geometry::Geometry() :size_(0), geometry_type_(Mesh) {
}

 std::vector<std::string> Geometry::getAttributeNameList() {
	std::vector <std::string> ret;
	for (auto [k, v] : data_map_) {
		ret.push_back(k);
	}
	return ret;
}

 BufferAttribute& Geometry::getBufferData(const std::string& attr) {
	return data_map_[attr];
}

 void Geometry::setAttribute(const std::string& attr, BufferAttribute& data, bool isVertex) {
	//if(attr != "position" && size_ && size_ != data.size())
	if (isVertex) {
		size_ = data.size();
	};

	data_map_[attr] = data;
}

 void Geometry::setIndex(const std::vector<unsigned>& indices) {
	geometry_type_ = Mesh_Indexed;
	indices_.assign(indices.begin(), indices.end());
	//glBindVertexArray(VAO);
	//glBindVertexArray(NULL);
}

 void Geometry::setupPipeline(Renderer& renderer)
 {
	 renderer.setupGeometry(*this);
 }

 bool Geometry::isMesh() {
	return geometry_type_ == Mesh;
}

 bool Geometry::isMeshIndexed() {
	return geometry_type_ == Mesh_Indexed;
}

 bool Geometry::isPipelineReady()
 {
	 return bReady_;
 }

 void Geometry::setPipelineReady(bool ready)
 {
	 bReady_ = ready;
 }

 GLuint Geometry::getVeticesNum() {
	return size_;
}

 GLuint Geometry::getIndicesNum() {
	return indices_.size();
}

 const unsigned* Geometry::getIndicesRawData()
 {
	 return indices_.data();
 }

 unsigned Geometry::getEBO()
 {
	 return EBO;
 }

 void Geometry::setEBO(unsigned id)
 {
	 EBO = id;
 }

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

void BoundingBox::updateMinMax(glm::vec3 *point, glm::vec3 *min, glm::vec3 *max) {
  if (point->x < min->x) {
    min->x = point->x;
  }

  if (point->x > max->x) {
    max->x = point->x;
  }

  if (point->y < min->y) {
    min->y = point->y;
  }

  if (point->y > max->y) {
    max->y = point->y;
  }

  if (point->z < min->z) {
    min->z = point->z;
  }

  if (point->z > max->z) {
    max->z = point->z;
  }
}

BoundingBox BoundingBox::transform(const glm::mat4 &matrix) const {
  glm::vec3 corners[8];
  getCorners(corners);

  corners[0] = matrix * glm::vec4(corners[0], 1.f);
  glm::vec3 newMin = corners[0];
  glm::vec3 newMax = corners[0];
  for (int i = 1; i < 8; i++) {
    corners[i] = matrix * glm::vec4(corners[i], 1.f);
    updateMinMax(&corners[i], &newMin, &newMax);
  }
  return {newMin, newMax};
}

bool BoundingBox::intersects(const BoundingBox &box) const {
  return ((min_.x >= box.min_.x && min_.x <= box.max_.x) || (box.min_.x >= min_.x && box.min_.x <= max_.x)) &&
      ((min_.y >= box.min_.y && min_.y <= box.max_.y) || (box.min_.y >= min_.y && box.min_.y <= max_.y)) &&
      ((min_.z >= box.min_.z && min_.z <= box.max_.z) || (box.min_.z >= min_.z && box.min_.z <= max_.z));
}

void BoundingBox::merge(const BoundingBox &box) {
  min_.x = std::min(min_.x, box.min_.x);
  min_.y = std::min(min_.y, box.min_.y);
  min_.z = std::min(min_.z, box.min_.z);

  max_.x = std::max(max_.x, box.max_.x);
  max_.y = std::max(max_.y, box.max_.y);
  max_.z = std::max(max_.z, box.max_.z);
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
  for (auto &plane : planes) {
    if (plane.intersects(box) == Plane::Intersects_Back) {
      return false;
    }
  }

  // check box intersects
  if (!bbox.intersects(box)) {
    return false;
  }

  return true;
}

bool Frustum::intersects(const glm::vec3 &p0) const {
  for (auto &plane : planes) {
    if (plane.intersects(p0) == Plane::Intersects_Back) {
      return false;
    }
  }

  return true;
}

bool Frustum::intersects(const glm::vec3 &p0, const glm::vec3 &p1) const {
  for (auto &plane : planes) {
    if (plane.intersects(p0, p1) == Plane::Intersects_Back) {
      return false;
    }
  }

  return true;
}

bool Frustum::intersects(const glm::vec3 &p0, const glm::vec3 &p1, const glm::vec3 &p2) const {
  for (auto &plane : planes) {
    if (plane.intersects(p0, p1, p2) == Plane::Intersects_Back) {
      return false;
    }
  }

  return true;
}

