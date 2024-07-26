#include "Geometry/Object.h"
#include "Material/Texture.h"
#include "FCamera.h"
#include "Material/FMaterial.h"
#include "Base/Globals.h"


UObject::UObject()
: quaternion_(1.f, 0.f, 0.f, 0.f), normalToWorld_(1.0f), position_(1.0f), scale_(1.0f),
worldMatrix_(1.0f), parentWorldMatrix_(1.f), localMatrix_(1.f) {
	//glGenVertexArrays(1, &VAO);
 	bDrawable = false;
}

UObject::UObject(const UObject &other): enable_shared_from_this(other), parent_(), children_(), uuid_()  {
	bDrawable = other.bDrawable;
	bCastShadow = other.bDrawable;
	shadingMode_ = other.shadingMode_;
	quaternion_ = other.quaternion_;
	normalToWorld_ = other.normalToWorld_;
	position_ = other.position_;
	scale_ = other.scale_;

	worldMatrix_ = other.worldMatrix_;
	parentWorldMatrix_ = glm::mat4(1.f);
	localMatrix_ = glm::mat4(1.f);

	for (auto&& child: other.children_) {
		// 为了保证复制的正确性，带有子类信息，调用Clone虚函数
		children_.emplace_back(child->Clone());
	}
}

void UObject::updateFrame(Renderer& renderer) {
	 // 这里不应该关心渲染的信息吧
}

void UObject::init() {
	for (auto&& child: children_) {
		// 延迟初始化，因为在构造函数里调用shared_from_this 会直接abort
		child->setParent(shared_from_this());
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
 		localMatrix_ = worldMatrix_;
 	}

	// 注意：glm的默认构造是列主序
	position_ = glm::vec3(transform[3]);

 	scale_.x = glm::length(glm::vec3(transform[0]));
 	scale_.y = glm::length(glm::vec3(transform[1]));
 	scale_.z = glm::length(glm::vec3(transform[2]));

	glm::mat4 rotation;
 	rotation[0] = glm::vec4(glm::vec3(transform[0]) / scale_.x, 0.f);
 	rotation[1] = glm::vec4(glm::vec3(transform[1]) / scale_.y, 0.f);
 	rotation[2] = glm::vec4(glm::vec3(transform[2]) / scale_.z, 0.f);
 	rotation[3] = glm::vec4(0.f, 0.f, 0.f, 1.f);
	quaternion_ = glm::quat_cast(rotation);

	normalToWorld_ = glm::transpose(glm::inverse(worldMatrix_));

 	for(auto&& child: children_) {
 		child->setParentWorldMatrix(worldMatrix_);
 	}
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

void UObject::enableCastShadow(const bool enabled) {
	bCastShadow = enabled;
	for(auto&& child: children_) {
		child->enableCastShadow(enabled);
	}
}

void UObject::setShadingMode(EShadingModel shadingMode) {
	shadingMode_ = shadingMode;
	for(auto&& child: children_) {
		child->setShadingMode(shadingMode);
	}
}

void UObject::rotate(float angle, const glm::vec3 &axis) {
	quaternion_ = glm::angleAxis(angle, glm::normalize(axis)) * quaternion_;
	updateWorldMatrix();
}

void UObject::lookAt(const glm::vec3 &focus, bool bIgnorePitch) {
	glm::vec3 dir = focus - getPosition();
	glm::vec3 pos = getPosition();
	if (bIgnorePitch) {
		dir.y = 0.f;
		pos.y = 0.f;
	}
	glm::mat4 rot = glm::lookAt(pos, glm::normalize(dir), getUpVector());
	quaternion_ = glm::quat_cast(rot);
	updateWorldMatrix();
}

void UObject::translate(float x, float y, float z) {
	position_ += glm::vec3(x, y, z);
	updateWorldMatrix();
}

void UObject::scale(float x, float y, float z) {
	scale_ *= glm::vec3(x, y, z);
	updateWorldMatrix();
}

void UObject::updateWorldMatrix() {
	glm::mat4 model(1.0f);
 	// 操作顺序上先scale、再rotate、最后translate
 	// 所以放在最右边是最先的，操作顺序是从右到左
 	model = glm::translate(model, position_);
	model = glm::mat4_cast(quaternion_) * model;
 	model = glm::scale(model, scale_);
	setWorldMatrix(model);
}

 void UObject::setPosition(const glm::vec3& position) {
	position_ = position;
	updateWorldMatrix();
}

 void UObject::setScale(const glm::vec3& scale)
 {
	 scale_ = scale;
	 updateWorldMatrix();
 }

 void UObject::setScale(float x, float y, float z)
 {
	 setScale(glm::vec3(x, y, z));
 }

 glm::vec3 UObject::getPosition() const {
	return position_;
}

 glm::mat4 UObject::getWorldMatrix() const
 {
	 return worldMatrix_;
 }

 glm::mat4 UObject::getNormalToWorld() const
 {
	 return normalToWorld_;
 }
