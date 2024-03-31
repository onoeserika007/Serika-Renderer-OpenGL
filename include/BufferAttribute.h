#pragma once
#include <vector>
#include <memory>
#include <glad/glad.h>
#include "Renderer.h"

class BufferAttribute {
	unsigned VBO = 0;
	std::shared_ptr<std::vector<float>> data_;
	size_t item_size_ = 0;
public:
	const float* data() const {
		return data_->data();
	}
	size_t size() const {
		return data_->size();
	}
	size_t byte_size() const {
		return sizeof(float) * data_->size();
	}
	size_t item_size() const {
		return item_size_;
	}

	void setupPipeline(Renderer& renderer) {
		renderer.setupVertexAttribute(*this);
	}

	unsigned getVBO() const {
		return VBO;
	}

	void setVBO(unsigned id) {
		VBO = id;
	}

	BufferAttribute(const std::vector<float>& data, size_t item_size): item_size_(item_size) {
		data_ = std::make_shared<std::vector<float>>(data);
	}

	BufferAttribute(){}
};