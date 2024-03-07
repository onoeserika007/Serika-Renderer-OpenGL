#pragma once
#include "BufferAttribute.h"
#include <string>
#include "glad/glad.h"
#include <unordered_map>
#include <iostream>

class Geometry {

	enum Type {
		Mesh,
		Mesh_Indexed
	};

	size_t size_;
	std::unordered_map<std::string, unsigned> layout_map_;
	std::unordered_map<std::string, BufferAttribute<float>> data_map_;
	//std::unordered_map<std::string, std::vector<float>> data_map_old_;
	std::unordered_map<std::string, unsigned> buffer_map_;
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

	unsigned getAttributeBuffer(const std::string& attr) {
		return buffer_map_[attr];
	}

	std::vector<std::string> getAttributeNameList() {
		std::vector <std::string> ret;
		for (auto [k, v] : buffer_map_) {
			ret.push_back(k);
		}
		return ret;
	}

	BufferAttribute<float> getBufferData(const std::string& attr) {
		return data_map_[attr];
	}

	void setAttribute(const std::string& attr, BufferAttribute<float> data, unsigned layout_position, bool isVertex = false) {
		//if(attr != "position" && size_ && size_ != data.size())
		if (isVertex) {
			size_ = data.size();
		}

		//glBindVertexArray(VAO);
		layout_map_[attr] = layout_position;
		auto& data_ = data_map_[attr];
		data_ = data;
		auto& VBO = buffer_map_[attr];
		glGenBuffers(1, &VBO);
		// OpenGL允许我们同时绑定多个缓冲，只要它们是不同的缓冲类型
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		// 把之前定义的顶点数据复制到缓冲的内存中
		// 我们希望显卡如何管理给定的数据
		// GL_STATIC_DRAW ：数据不会或几乎不会改变。
		// GL_DYNAMIC_DRAW：数据会被改变很多。
		// GL_STREAM_DRAW ：数据每次绘制时都会改变。
		// 比如说一个缓冲中的数据将频繁被改变，那么使用的类型就是GL_DYNAMIC_DRAW或GL_STREAM_DRAW，这样就能确保显卡把数据放在能够高速写入的内存部分。
		glBufferData(GL_ARRAY_BUFFER, data_.byte_size() , data_.data(), GL_STATIC_DRAW);
		// 指定顶点属性的解释方式（如何解释VBO中的数据）
		// 1. glVertexAttribPointer
		// attri的Location(layout location = 0) | item_size | 数据类型 | 是否Normalize to 0-1 | stride | 从Buffer起始位置开始的偏移
		glVertexAttribPointer(layout_position, data_.item_size(), GL_FLOAT, GL_FALSE, data_.item_size() * sizeof(float), (void*)0);
		// 以顶点属性位置值作为参数，启用顶点属性；顶点属性默认是禁用的
		glEnableVertexAttribArray(layout_position);
		glBindVertexArray(NULL);
	}

	//void setAttribute(const std::string& attr, const std::vector<float>& data, size_t item_size, unsigned layout_position) {
	//	//if(attr != "position" && size_ && size_ != data.size())
	//	if (attr == "position") {
	//		size_ = data.size();
	//	}

	//	glBindVertexArray(VAO);
	//	layout_map_[attr] = layout_position;
	//	auto& data_ = data_map_old_[attr];
	//	data_.assign(data.begin(), data.end());
	//	auto& VBO = buffer_map_[attr];
	//	glGenBuffers(1, &VBO);
	//	// OpenGL允许我们同时绑定多个缓冲，只要它们是不同的缓冲类型
	//	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	//	// 把之前定义的顶点数据复制到缓冲的内存中
	//	// 我们希望显卡如何管理给定的数据
	//	// GL_STATIC_DRAW ：数据不会或几乎不会改变。
	//	// GL_DYNAMIC_DRAW：数据会被改变很多。
	//	// GL_STREAM_DRAW ：数据每次绘制时都会改变。
	//	// 比如说一个缓冲中的数据将频繁被改变，那么使用的类型就是GL_DYNAMIC_DRAW或GL_STREAM_DRAW，这样就能确保显卡把数据放在能够高速写入的内存部分。
	//	// Buffer类型 | 总byte长度
	//	glBufferData(GL_ARRAY_BUFFER, data_.size() * sizeof(float), data_.data(), GL_STATIC_DRAW);
	//	// 指定顶点属性的解释方式（如何解释VBO中的数据）
	//	// 1. glVertexAttribPointer
	//	// attri的Location(layout location = 0) | item_size | 数据类型 | 是否Normalize to 0-1 | stride | 从Buffer起始位置开始的偏移
	//	glVertexAttribPointer(layout_position, item_size, GL_FLOAT, GL_FALSE, item_size * sizeof(float), (void*)0);
	//	// 以顶点属性位置值作为参数，启用顶点属性；顶点属性默认是禁用的
	//	glEnableVertexAttribArray(layout_position);
	//	glBindVertexArray(NULL);
	//}

	void setIndex(const std::vector<unsigned>& indices) {
		geometry_type_ = Mesh_Indexed;
		indices_.assign(indices.begin(), indices.end());
		//glBindVertexArray(VAO);
		glGenBuffers(1, &EBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices_.size() * sizeof(unsigned), indices_.data(), GL_STATIC_DRAW);
		//glBindVertexArray(NULL);
	}

	//void draw() {
	//	//glBindVertexArray(VAO);
	//	if (geometry_type_ == Mesh) {
	//		// primitive | 顶点数组起始索引 | 绘制indices数量
	//		glDrawArrays(GL_TRIANGLES, 0, size_);
	//		//for (auto num : data_map_["position"]) {
	//		//	std::cout << num << ", ";
	//		//}
	//		//std::cout <<" -------------------------------------------------- " << std::endl;
	//	}
	//	else {
	//		// primitive | nums | 索引类型 | 最后一个参数里我们可以指定EBO中的偏移量（或者传递一个索引数组，但是这是当你不在使用EBO的时候），但是我们会在这里填写0。
	//		glDrawElements(GL_TRIANGLES, indices_.size(), GL_UNSIGNED_INT, 0);
	//	}
	//	glBindVertexArray(NULL);
	//}

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