#pragma once
#include <vector>
#include <memory>

template <typename T>
class BufferAttribute {
	std::shared_ptr<std::vector<T>> data_;
	size_t item_size_;
public:
	const T* data() {
		return data_->data();
	}
	size_t size() {
		return data_->size();
	}
	size_t byte_size() {
		return sizeof(T) * data_->size();
	}
	size_t item_size() {
		return item_size_;
	}

	BufferAttribute(const std::vector<T>& data, size_t item_size): item_size_(item_size) {
		data_ = std::make_shared<std::vector<T>>(data);
	}

	BufferAttribute() {}
};