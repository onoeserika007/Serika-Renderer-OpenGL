#pragma once
#include "Geometry.h"
#include "Shader.h"
#include "Texture.h"
#include <memory>

class Object {
	std::shared_ptr<Geometry> pgeometry_;
	std::shared_ptr<Shader> pshader_;
	std::unordered_map<std::string, unsigned> buffer_map_;
	unsigned VAO;
	void BindObjectBuffer() {
		if (!VAO) {
			glGenVertexArrays(1, &VAO);
		}
	}
public:
	Object(std::shared_ptr<Geometry> pgeometry, std::shared_ptr<Shader> pshader): pgeometry_(pgeometry), VAO(0), pshader_(pshader) {
		glGenVertexArrays(1, &VAO);
		glBindVertexArray(VAO);
		const auto& attrList = pgeometry->getAttributeNameList();
		for (auto attr : attrList) {
			auto loc = pshader_->getAttributeLocation(attr);
			if (loc != -1) {
				const auto& VBO = pgeometry_->getAttributeBuffer(attr);
				const auto& bufferData = pgeometry_->getBufferData(attr);
				glBindBuffer(GL_ARRAY_BUFFER, VBO);
				glVertexAttribPointer(loc, bufferData.item_size(), GL_FLOAT, GL_FALSE, bufferData.item_size() * sizeof(float), (void*)0);
				glEnableVertexAttribArray(loc);
			}
		}
		glBindVertexArray(NULL);
	}

	void draw() {
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
};