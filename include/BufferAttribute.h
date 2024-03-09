#pragma once
#include <vector>
#include <memory>
#include <glad/glad.h>

template <typename T>
class BufferAttribute {
	std::shared_ptr<std::vector<T>> data_;
	size_t item_size_;
	unsigned VBO;
public:
	const T* data() const {
		return data_->data();
	}
	size_t size() const {
		return data_->size();
	}
	size_t byte_size() const {
		return sizeof(T) * data_->size();
	}
	size_t item_size() const {
		return item_size_;
	}

	unsigned bindToGPU() {
		glGenBuffers(1, &VBO);
		// OpenGL允许我们同时绑定多个缓冲，只要它们是不同的缓冲类型
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		// 把之前定义的顶点数据复制到缓冲的内存中
		// 我们希望显卡如何管理给定的数据
		// GL_STATIC_DRAW ：数据不会或几乎不会改变。
		// GL_DYNAMIC_DRAW：数据会被改变很多。
		// GL_STREAM_DRAW ：数据每次绘制时都会改变。
		// 比如说一个缓冲中的数据将频繁被改变，那么使用的类型就是GL_DYNAMIC_DRAW或GL_STREAM_DRAW，这样就能确保显卡把数据放在能够高速写入的内存部分。
		glBufferData(GL_ARRAY_BUFFER, this->byte_size(), this->data(), GL_STATIC_DRAW);
		return VBO;
	}

	unsigned getVBO() const {
		return VBO;
	}

	BufferAttribute(const std::vector<T>& data, size_t item_size): item_size_(item_size), VBO(0) {
		data_ = std::make_shared<std::vector<T>>(data);
	}

	BufferAttribute() :item_size_(0){}
};