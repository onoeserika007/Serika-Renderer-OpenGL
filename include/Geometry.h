#pragma once
#include "BufferAttribute.h"
#include <string>
#include "Utils//utils.h"
#include "glad/glad.h"
#include <unordered_map>
#include <iostream>
#include <utility>

template <typename T>
class Geometry {

	enum Type {
		Mesh,
		Mesh_Indexed
	};

	size_t size_;
	//std::unordered_map<std::string, unsigned> layout_map_;
	std::unordered_map<std::string, BufferAttribute<T>> data_map_;
	//std::unordered_map<std::string, std::vector<float>> data_map_old_;
	//std::unordered_map<std::string, unsigned> buffer_map_;
	std::vector<unsigned> indices_;
	//unsigned VAO = 0;
	unsigned EBO = 0;
	//std::vector<float> pos_data_;
	//std::vector<float> color_data_;
	//std::vector<float> normal_data_;
	//std::vector<float> uv_data_;
	Type geometry_type_;


public:
	Geometry():size_(0), geometry_type_(Mesh) {
		//glGenVertexArrays(1, &VAO);
	}
	//Geometry(const std::vector<float>& vertex_data): geometry_type_(Mesh) {
	//	// Vertex Array Object
	//	// 等于说VAO也是一种上下文，在设置VBO前绑定，解绑后，需要绘制时再绑定
	//	glGenVertexArrays(1, &VAO);
	//	setAttribute("position", vertex_data, 3, 0);
	//}

	//unsigned getAttributeBuffer(const std::string& attr) {
	//	return buffer_map_[attr];
	//}

	std::vector<std::string> getAttributeNameList() {
		std::vector <std::string> ret;
		for (auto [k, v] : data_map_) {
			ret.push_back(k);
		}
		return ret;
	}

	//std::vector<BufferAttribute<T>> getAttributes() {
	//	std::vector <BufferAttribute<T>> ret;
	//	for (auto [k, v] : data_map_) {
	//		ret.push_back(k);
	//	}
	//	return ret;
	//}

	BufferAttribute<T> getBufferData(const std::string& attr) {
		return data_map_[attr];
	}

	void setAttribute(const std::string& attr, BufferAttribute<T> data,  bool isVertex = false) {
		//if(attr != "position" && size_ && size_ != data.size())
		if (isVertex) {
			size_ = data.size();
		}

		const auto& VBO = data.bindToGPU();
		if (!VBO) {
			logDebug(attr + ": bind to gpu failed!");
		}
		auto& data_ = data_map_[attr];
		data_ = data;
	}

	void setIndex(const std::vector<unsigned>& indices) {
		geometry_type_ = Mesh_Indexed;
		indices_.assign(indices.begin(), indices.end());
		//glBindVertexArray(VAO);
		glGenBuffers(1, &EBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices_.size() * sizeof(unsigned), indices_.data(), GL_STATIC_DRAW);
		//glBindVertexArray(NULL);
	}

	bool isMesh() {
		return geometry_type_ == Mesh;
	}

	bool isMeshIndexed() {
		return geometry_type_ == Mesh_Indexed;
	}

	GLuint getVeticesNum() {
		return size_;
	}

	GLuint getIndicesNum() {
		return indices_.size();
	}

	void drawWithOutTexture() {

	}
};