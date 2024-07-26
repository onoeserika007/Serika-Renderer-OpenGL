#pragma once
#include <vector>
#include <memory>
#include "Renderer.h"

enum EBufferAttribute {
	EBA_Position,
	EBA_TexCoord,
	EBA_Normal
};

class BufferAttribute {

public:
	BufferAttribute(){}

	void setupPipeline(const Renderer &renderer) const { renderer.setupVertexAttribute(*this); }
	const float* data() const {
		if (data_) return data_->data();
		return nullptr;
	}
	size_t size() const {
		if (data_) return data_->size();
		return 0;
	}
	size_t byte_size() const { return sizeof(float) * size(); }
	size_t elem_size() const { return elem_size_; }

	float operator[](const unsigned index) const { return (*data_)[index]; }
	float& operator[](const unsigned index) { return (*data_)[index]; }

	BufferAttribute(const std::vector<float>& data, size_t item_size): elem_size_(item_size) {
		data_ = std::make_shared<std::vector<float>>(data);
	}

public:
	mutable unsigned VBO = 0;
private:
	std::shared_ptr<std::vector<float>> data_;
	size_t elem_size_ = 0;
};