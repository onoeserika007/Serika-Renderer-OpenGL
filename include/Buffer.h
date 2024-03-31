#pragma once
#include <iostream>
#include <memory>
#include <vector>
#include <cassert>

template <typename T>
class Buffer
{
    // std::unique_ptr<T> data;
    std::vector<T> data_;
    int width_;
    int height_;

    Buffer() = default;
    Buffer(int w, int h): width_(w), height_(h), data_(w * h) {}
    Buffer(const int& _width, const int& _height, const T& value)
        : width_(_width), height_(_height), data_(_width* _height, value) {}
    struct PassKey{};
public:
    Buffer(const Buffer<T>& other) {
        width_ = other.width_;
        height_ = other.height_;
        data_ = other.data_;
    }
    Buffer(PassKey _, int w, int h): Buffer(w, h) {}
    static std::shared_ptr<Buffer<T>> makeDefault(int w, int h);
    static int calcIndex(int x, int y, int width) {
        return x + y * width;
    }

    int getIndex(int x, int y)
    {
        return calcIndex(x, y, width_);
    }

    T& get(int x, int y)
    {
        return data_[getIndex(x, y)];
    }

    void set(int x, int y, const T& value)
    {
        data_[getIndex(x, y)] = value;
    }

    void clear() {
        memset(data_.data(), 0, data_.size());
    }

    int width() {
        return width_;
    }

    int height() {
        return height_;
    }

    const T* rawData() {
        return data_.data();
    }

    void write_to_file() {

    }
};

template<typename T>
inline std::shared_ptr<Buffer<T>> Buffer<T>::makeDefault(int w, int h)
{
    return std::make_shared<Buffer<T>>(PassKey(), w, h);
}
