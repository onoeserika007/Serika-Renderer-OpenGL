#include "Geometry/Object.h"

#include <queue>
#include <glm/gtx/matrix_decompose.hpp>

#include "Geometry/Triangle.h"
#include "Geometry/UMesh.h"
#include "Geometry/Geometry.h"

// UObject::UObject(const std::shared_ptr<UMesh> &mesh) {
// 	setMesh(mesh);
// }

UObject::UObject(const UObject &other): enable_shared_from_this(other), parent_(), children_(), uuid_()  {
	std::cout << "UObject - Copy Ctor called" << std::endl;
	bVisible = other.bVisible;
	worldOrientation_ = other.worldOrientation_;
	normalToWorld_ = other.normalToWorld_;
	worldPosition_ = other.worldPosition_;
	worldScale_ = other.worldScale_;

	worldMatrix_ = other.worldMatrix_;
	parentWorldMatrix_ = glm::mat4(1.f);
	localMatrix_ = glm::mat4(1.f);

	for (auto&& child: other.children_) {
		// 为了保证复制的正确性，带有子类信息，调用Clone虚函数
		children_.emplace_back(child->Clone());;
	}
}

UObject::UObject(UObject &&other) noexcept : enable_shared_from_this(other), uuid_() {
	std::cout << "UObject - Move Ctor called" << std::endl;
	bVisible = other.bVisible;
	worldOrientation_ = other.worldOrientation_;
	normalToWorld_ = other.normalToWorld_;
	worldPosition_ = other.worldPosition_;
	worldScale_ = other.worldScale_;

	// move
	parent_ = std::move(other.parent_);
	children_ = std::move(other.children_);

	worldMatrix_ = other.worldMatrix_;
	parentWorldMatrix_ = glm::mat4(1.f);
	localMatrix_ = glm::mat4(1.f);
}

void UObject::updateFrame() {
	// 这里不应该关心渲染的信息吧
	// update mesh transform
	if (mesh_) {
		mesh_->updateWorldMatrix((getWorldMatrix()));
	}
}

void UObject::init() {
	for (auto&& child: children_) {
		// 延迟初始化，因为在构造函数里调用shared_from_this 会直接abort
		child->setParent(shared_from_this());
		child->init();
	}
}

// set model matrix

void UObject::setWorldMatrix(const glm::mat4& transform) {

 	worldMatrix_ = transform;

 	if (parent_.lock()) {
 		localMatrix_ = glm::inverse(parentWorldMatrix_) * worldMatrix_;
 	}
 	else {
 		parent_.reset();
 		// localMatrix_ = worldMatrix_; // worldMatrix is updated by local matrix, it shouldn't change localMatrix_
 	}

	// 注意：glm的默认构造是列主序
	// position_ = glm::vec3(transform[3]); // 这个translation不是我们需要的translation
	// TRS，先scale，再rotate，最后translate，但是顺序好像无所谓，主要分解得分解对
	glm::vec3 scale;
	glm::vec3 translation;
	glm::quat orientation;
	glm::vec3 skew;
	glm::vec4 perspective;
	glm::decompose(worldMatrix_, scale, orientation, translation, skew, perspective);

	worldPosition_ = translation;
	worldScale_ = scale;
	worldOrientation_ = orientation;
	normalToWorld_ = glm::transpose(glm::inverse(worldMatrix_));

 	for(auto&& child: children_) {
 		child->setParentWorldMatrix(worldMatrix_);
 	}
}

void UObject::setLocalMatrix(const glm::mat4 &local) {
	localMatrix_ = local;
	setWorldMatrix(parentWorldMatrix_ * localMatrix_);
}

void UObject::setParentWorldMatrix(const glm::mat4 &transform) {
 	parentWorldMatrix_ = transform;
 	setWorldMatrix(parentWorldMatrix_ * localMatrix_);
}

void UObject::setParent(const std::shared_ptr<UObject> &parent) {
 	parent_ = parent;
 	setParentWorldMatrix(parent->getWorldMatrix());
}

void UObject::addChild(const std::shared_ptr<UObject> &child) {
 	child->setParent(shared_from_this());
 	children_.emplace_back(child);
}

std::vector<std::shared_ptr<UObject>>& UObject::getChildren() {
 	return children_;
}

void UObject::setMesh(const std::shared_ptr<UMesh> &mesh) {
	mesh_ = mesh;
	if (mesh_) {
		mesh_->setParentObject(shared_from_this());

	}
}

void UObject::rotate(float angle, const glm::vec3 &axis) {
	// auto&& quat = glm::angleAxis(glm::radians(angle), glm::normalize(axis));
	// printMat4("New Rotate: ", glm::mat4_cast(quat));
	localOrientation_ = glm::angleAxis(glm::radians(angle), glm::normalize(axis)) * localOrientation_;
	updateLocalMatrix();
}

void UObject::lookAt(const glm::vec3 &focus, bool bIgnorePitch) {

	glm::vec3 pos = getWorldPosition();
	if (bIgnorePitch) {
		pos.y = focus.y;
	}
	glm::mat4 rot = glm::lookAt(pos, focus, getUpVector());

	// // look at will leave parent
	// parent_.reset();
	// glm::vec3 scale;
	// glm::vec3 translation;
	// glm::quat orientation;
	// glm::vec3 skew;
	// glm::vec4 perspective;
	// glm::decompose(parentWorldMatrix_, scale, orientation, translation, skew, perspective);
	localOrientation_ = glm::quat_cast(glm::inverse(rot)); // transform minus z axis by default, if the front of the obj is like this
	updateLocalMatrix(); // 调整模型转向的问题应该交给一个Mesh子组件去解决，而这是我们所没有实现的
	// glm::mat4 newParentTransform {1.f};
	// newParentTransform = glm::scale(newParentTransform, scale);
	// newParentTransform = newParentTransform * glm::mat4_cast(orientation);
	// newParentTransform = glm::translate(newParentTransform, translation);
	// setParentWorldMatrix(newParentTransform);
}

// 要特别小心运作的状态，updateWorldMatrix就是从现有的数据重新按照T、R、S的顺序重新
void UObject::translate(float x, float y, float z) {
	worldPosition_ += glm::vec3(x, y, z);
	updateLocalMatrix();
}

void UObject::scale(float x, float y, float z) {
	worldScale_ *= glm::vec3(x, y, z);
	updateLocalMatrix();
}

void UObject::applyTransform(const glm::mat4 &transform) {;
	setLocalMatrix(transform * localMatrix_);
}

void UObject::updateLocalMatrix() {
	glm::mat4 model(1.0f);
 	// 操作顺序上先scale、再rotate、最后translate
 	// 所以放在最右边是最先的，操作顺序是从右到左
	model = glm::translate(model, localPosition_);
	model = model * glm::mat4_cast(localOrientation_);
	model = glm::scale(model, localScale_);
	setLocalMatrix(model);
}

 void UObject::setPosition(const glm::vec3& position) {
	localPosition_ = position;
	updateLocalMatrix();
}

 void UObject::setScale(const glm::vec3& scale)
 {
	 localScale_ = scale;
	 updateLocalMatrix();
 }

 void UObject::setScale(float x, float y, float z)
 {
	 setScale(glm::vec3(x, y, z));
 }

 glm::vec3 UObject::getWorldPosition() const {
	return worldPosition_;
}

 glm::mat4 UObject::getWorldMatrix() const
 {
	 return worldMatrix_;
 }

 glm::mat4 UObject::getNormalToWorld() const
 {
	 return normalToWorld_;
 }

/**
 * Intersection Reated
 */
void UObject::buildBVH() {
	bShowDebugBBox_ = true;
	if (Config::getInstance().bUseBVH == false) return;

	intersectables_.clear(); // clear old bvhs

	// travserse to collect all submeshes
	std::vector<std::shared_ptr<UMesh>> subMeshes;
	std::queue<std::shared_ptr<UMesh>> q;
	q.push(mesh_);
	while(!q.empty()) {
		auto&& cur = q.front();
		q.pop();
		subMeshes.emplace_back(cur);
		if (cur) {
			for (auto&& subMesh: cur->getMeshes()) {
				q.push(subMesh);
			}
		}
	}

	// triangles
	for (auto&& submesh: subMeshes) {
		if (auto&& pgeom = submesh->getGeometry()) {
			for(int i = 0; i < pgeom->getTriangleNum(); i++) {
				auto&& tri = submesh->fetchTriangle(i);
				// apply model matrix before generate BVH!!
				tri->transform(getWorldMatrix());
				bbox_.merge(tri->getBounds());
				area_ += tri->getArea();
				intersectables_.emplace_back(std::move(tri));
			}
		}
	}

	// children
	for (auto&& child: getChildren()) {
		child->buildBVH();
		bbox_.merge(child->getBounds());
		area_ += child->getArea();
		intersectables_.emplace_back(child);
	}

	bvh_accel_ = std::make_shared<BVHAccel>(intersectables_);
}

Intersection UObject::getIntersection(const Ray &ray) {
	if (bvh_accel_) {
		return bvh_accel_->Intersect(ray);
	}
	return {};
}

void UObject::Sample(Intersection &pos, float &pdf) {
	if (bvh_accel_) {
		bvh_accel_->Sample(pos, pdf);
	}
}
