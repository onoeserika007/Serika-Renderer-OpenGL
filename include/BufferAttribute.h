#pragma once
#include <vector>
#include <memory>

template <typename T>
class BufferAttribute {
	std::shared_ptr<std::vector<T>> data_;
	size_t item_size_;
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

	BufferAttribute(const std::vector<T>& data, size_t item_size): item_size_(item_size) {
		data_ = std::make_shared<std::vector<T>>(data);
	}

	BufferAttribute() {}
};