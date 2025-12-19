#ifndef SERIKA_RENDERER_INCLUDE_GEOMETRY_BUFFERATTRIBUTE_H
#define SERIKA_RENDERER_INCLUDE_GEOMETRY_BUFFERATTRIBUTE_H
#include <vector>
#include <memory>
#include "Renderer.h"

enum EBufferAttribute {
	EBA_Position,
	EBA_TexCoord,
	EBA_Normal
};

class BufferAttribute: public PipelineLoadable {

public:
	BufferAttribute()= default;

	void setPipelineReady(bool bReady) const { bIsPipelineSetup_ = bReady; }
	bool isPipelineSetup() const { return bIsPipelineSetup_; }
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
	bool empty() const { return data_->empty(); }

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
	mutable bool bIsPipelineSetup_ = false;
};

#endif // SERIKA_RENDERER_INCLUDE_GEOMETRY_BUFFERATTRIBUTE_H
