#include "../include/Geometry/Object.h"

#include "BufferAttribute.h"
#include "BufferAttribute.h"
#include "Shader.h"
#include "Texture.h"
#include "Camera.h"
#include "Material.h"
#include "Base/GLMInc.h"


UObject::UObject()
: rotation_(1.0f), normalToWorld_(1.0f), position_(1.0f), scale_(1.0f),
worldMatrix_(1.0f), parentWorldMatrix_(1.f), localMatrix_(1.f) {
	//glGenVertexArrays(1, &VAO);
 	bDrawable = false;
}

void UObject::updateFrame(Renderer& renderer) {
	 // 这里不应该关心渲染的信息吧
}

// set model matrix

void UObject::setWorldMatrix(const glm::mat4& transform) {

 	worldMatrix_ = transform;

 	if (parent_.lock()) {
 		localMatrix_ = glm::inverse(parentWorldMatrix_) * worldMatrix_;
 	}
 	else {
 		parent_.reset();
 	}

	// 注意：glm的默认构造是列主序
	position_ = glm::vec3(transform[3]);

 	scale_.x = glm::length(glm::vec3(transform[0]));
 	scale_.y = glm::length(glm::vec3(transform[1]));
 	scale_.z = glm::length(glm::vec3(transform[2]));

 	rotation_[0] = glm::vec4(glm::vec3(transform[0]) / scale_.x, 0.f);
 	rotation_[1] = glm::vec4(glm::vec3(transform[1]) / scale_.y, 0.f);
 	rotation_[2] = glm::vec4(glm::vec3(transform[2]) / scale_.z, 0.f);
 	rotation_[3] = glm::vec4(0.f, 0.f, 0.f, 1.f);

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

void UObject::setShadingMode(EShadingMode shadingMode) {
	shadingMode_ = shadingMode;
	for(auto&& child: children_) {
		child->setShadingMode(shadingMode);
	}
}

void UObject::updateWorldMatrix() {
	glm::mat4 model(1.0f);
 	// 操作顺序上先scale、再rotate、最后translate
 	// 所以放在最右边是最先的，操作顺序是从右到左
 	model = glm::translate(model, position_);
	model = rotation_ * model;
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
