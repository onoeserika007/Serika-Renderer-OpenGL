#include "Object.h"
#include "Geometry.h"
#include "Shader.h"
#include "Texture.h"
#include "Camera.h"
#include "Material.h"
#include "Base/GLMInc.h"

 Object::Object() : pgeometry_(nullptr), VAO(0), modelMatrix_(1.0f), position_(1.0f), normalToWorld_(1.0f), scale_(1.0f), rotation_(1.0f), pmaterial_(nullptr) {
	//glGenVertexArrays(1, &VAO);
}

 Object::Object(std::shared_ptr<Geometry> pgeometry, std::shared_ptr<Material> pmaterial) : Object() {
	pgeometry_ = pgeometry, pmaterial_ = pmaterial;

	if (!pgeometry_) {
		std::cout << "Object: init - Objrct has no geometry or has a pure point!" << std::endl;
	}
	else {
		init();
	}
}

 void Object::init() {
}

 void Object::updateFrame(Renderer& renderer) {
	 // 这里不应该关心渲染的信息吧
}

 void Object::draw(Renderer& renderer) {
	 //renderer.drawObject(*this);
}

 std::shared_ptr<Shader> Object::getpShader() {
	if (pmaterial_) {
		return pmaterial_->getpshader();
	}
	return nullptr;
}

 std::shared_ptr<Geometry> Object::getpGeometry()
 {
	 return pgeometry_;
 }

 std::shared_ptr<Material> Object::getpMaterial()
 {
	 return pmaterial_;
 }

 unsigned Object::getVAO()
 {
	 return VAO;
 }

 ShadingMode Object::getShadingMode()
 {
	 return shadingMode_;
 }

 void Object::setVAO(unsigned id)
 {
	 VAO = id;
 }

 void Object::setShadingMode(ShadingMode shadingMode)
 {
	 shadingMode_ = shadingMode;
 }

// set model matrix

 void Object::setWorldMatrix(glm::mat4 model) {
	modelMatrix_ = model;
	// 注意：glm的默认构造是列主序
	position_ = glm::vec3(model[3][0], model[3][1], model[3][2]);
	// TODO: 怎么更新rotation和scale呢？
	normalToWorld_ = glm::transpose(glm::inverse(modelMatrix_));
}

 void Object::updateWorldMatrix() {
	glm::mat4 model(1.0f);
	model = glm::scale(model, scale_);
	model = rotation_ * model;
	model = glm::translate(model, position_);
	setWorldMatrix(model);
}

 void Object::setPosition(const glm::vec3& position) {
	position_ = position;
	updateWorldMatrix();
}

 void Object::setScale(const glm::vec3& scale)
 {
	 scale_ = scale;
	 updateWorldMatrix();
 }

 void Object::setScale(float x, float y, float z)
 {
	 setScale(glm::vec3(x, y, z));
 }

 void Object::setLight(std::shared_ptr<Light> plight) const {
	pmaterial_->setLight(plight);
}

 void Object::setLightArray(const std::vector<std::shared_ptr<Light>>& plightArray) const
 {
	 pmaterial_->setLightArray(plightArray);
 }

 void Object::setpshader(std::shared_ptr<Shader> pshader)
 {
	 // 更改shader后需要重新绑定layout location
	 pmaterial_->setpShader(pshader);
 }

 glm::vec3 Object::getPosition() const {
	return position_;
}

 glm::mat4 Object::getModelMatrix() const
 {
	 return modelMatrix_;
 }

 glm::mat4 Object::getNormalToWorld() const
 {
	 return normalToWorld_;
 }
