#include "../include/Geometry/UObject.h"
#include "Shader.h"
#include "Texture.h"
#include "Camera.h"
#include "Material.h"
#include "Base/GLMInc.h"

 UObject::UObject() : modelMatrix_(1.0f), position_(1.0f), normalToWorld_(1.0f), scale_(1.0f), rotation_(1.0f){
	//glGenVertexArrays(1, &VAO);
}

 void UObject::init() {
}

 void UObject::updateFrame(Renderer& renderer) {
	 // 这里不应该关心渲染的信息吧
}

// set model matrix

 void UObject::setWorldMatrix(glm::mat4 model) {
	modelMatrix_ = model;
	// 注意：glm的默认构造是列主序
	position_ = glm::vec3(model[3][0], model[3][1], model[3][2]);
	// TODO: 怎么更新rotation和scale呢？
	normalToWorld_ = glm::transpose(glm::inverse(modelMatrix_));
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

 glm::mat4 UObject::getModelMatrix() const
 {
	 return modelMatrix_;
 }

 glm::mat4 UObject::getNormalToWorld() const
 {
	 return normalToWorld_;
 }
