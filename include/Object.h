#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "Geometry.h"
#include "Shader.h"
#include "Texture.h"
#include "Camera.h"
#include "Material.h"
#include <memory>

template <typename T>
class Object {
	std::shared_ptr<Geometry<T>> pgeometry_;
	//std::shared_ptr<Shader> pshader_;
	std::shared_ptr<Material> pmaterial_;

	//std::unordered_map<std::string, unsigned> buffer_map_;

	glm::mat4 modelMatrix_;
	glm::mat4 normalToWorld_;
	glm::vec3 position_;
	unsigned VAO;
public:
	Object() : pgeometry_(nullptr), VAO(0), modelMatrix_(1.0f), position_(1.0f), normalToWorld_(1.0f), pmaterial_(nullptr){
		glGenVertexArrays(1, &VAO);
	}

	Object(std::shared_ptr<Geometry<T>> pgeometry, std::shared_ptr<Material> pmaterial): Object(){
		pgeometry_ = pgeometry, pmaterial_ = pmaterial;

		if (!pgeometry_) {
			std::cout << "Object: init - Objrct has no geometry or has a pure point!" << std::endl;
		}
		else {
			init();
		}
	}  

	void init() {
		if (!pgeometry_ || !pmaterial_) {
			return;
		}

		auto pshader = pmaterial_->getpshader();
		glBindVertexArray(VAO);
		const auto& attrList = pgeometry_->getAttributeNameList();
		for (auto attr : attrList) {
			auto loc = pshader->getAttributeLocation(attr);
			if (loc != -1) {
				//const auto& VBO = pgeometry_->getAttributeBuffer(attr);
				const auto& bufferData = pgeometry_->getBufferData(attr);
				const auto& VBO = bufferData.getVBO();
				glBindBuffer(GL_ARRAY_BUFFER, VBO);
				// 指定顶点属性的解释方式（如何解释VBO中的数据）
				// 1. glVertexAttribPointer
				// attri的Location(layout location = 0) | item_size | 数据类型 | 是否Normalize to 0-1 | stride | 从Buffer起始位置开始的偏移
				glVertexAttribPointer(loc, bufferData.item_size(), GL_FLOAT, GL_FALSE, bufferData.item_size() * sizeof(float), (void*)0);
				// 以顶点属性位置值作为参数，启用顶点属性；顶点属性默认是禁用的
				glEnableVertexAttribArray(loc);
			}
		}
		glBindVertexArray(NULL);
	}

	void updateFrame() {
		// 不要忘记在设置uniform变量之前激活着色器程序！
		pmaterial_->use();
		// 里面的modelMatrix会积累改变，还是把状态管理做在外面吧// 暂时
		//modelMatrix *= transform;
	}

	void draw(const Camera& camera) {
		updateFrame();

		if (!pgeometry_) {
			return;
		}

		auto pshader_ = pmaterial_->getpshader();
		pshader_->setMat4("uNormalToWorld", normalToWorld_);
		pshader_->setMat4("uModel",  modelMatrix_);
		//std::cout <<  << std::endl;
		//printMat4("model", model);
		pshader_->setMat4("uView", camera.GetViewMatrix());
		pshader_->setMat4("uProjection", camera.GetProjectionMatrix());
		glBindVertexArray(VAO);
		if (pgeometry_->isMesh()) {
			// primitive | 顶点数组起始索引 | 绘制indices数量
			glDrawArrays(GL_TRIANGLES, 0, pgeometry_->getVeticesNum());
			//for (auto num : data_map_["position"]) {
			//	std::cout << num << ", ";
			//}
			//std::cout <<" -------------------------------------------------- " << std::endl;
		}
		else {
			// primitive | nums | 索引类型 | 最后一个参数里我们可以指定EBO中的偏移量（或者传递一个索引数组，但是这是当你不在使用EBO的时候），但是我们会在这里填写0。
			glDrawElements(GL_TRIANGLES, pgeometry_->getIndicesNum(), GL_UNSIGNED_INT, 0);
		}
		glBindVertexArray(NULL);
	}

	std::shared_ptr<Shader> getpShader() const {
		if (pmaterial_) {
			return pmaterial_->getpshader();
		}
		return nullptr;
	}

	//void setShader(std::shared_ptr<Shader> pshader) {
	//	pshader_ = pshader;
	//	init();
	//}

	// set model matrix
	void setWorldMatrix(glm::mat4 model) {
		modelMatrix_ = model;
		position_ = glm::vec3(model[0][3], model[1][3], model[2][3]);
		normalToWorld_ = glm::transpose(glm::inverse(modelMatrix_));
	}
};