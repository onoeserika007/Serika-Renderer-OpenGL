#pragma once
#include <iostream>
#include <memory>
#include <vector>
#include "Base/Globals.h"

template <typename T>
class Buffer
{
    // std::unique_ptr<T> data;
    std::vector<T> data_;
    int width_{};
    int height_{};

    // Buffer() = default;
    Buffer(int w, int h): data_(w * h), width_(w), height_(h) {}
    Buffer(int _width, int _height, const T& value)
        : data_(_width * _height, value), width_(_width), height_(_height) {}

    Buffer(const Buffer<T>& other) {
        width_ = other.width_;
        height_ = other.height_;
        data_ = other.data_;
    }

    Buffer() = default;
public:

    template<typename... Args>
    static std::shared_ptr<Buffer<T>> makeBuffer(Args&& ...args);

    static int calcIndex(int x, int y, int width) {
        return x + y * width;
    }

    int getIndex(int x, int y) const
    {
        return calcIndex(x, y, width_);
    }

    T& getPixelRef(int x, int y)
    {
        x = glm::clamp(x, 0, width_ - 1);
        y = glm::clamp(y, 0, height_ - 1);
        return data_[getIndex(x, y)];
    }

    T getPixel(int x, int y) const {
        if (x < 0 || x >= width_ || y < 0 || y >= height_) return {};
        return data_[getIndex(x, y)];
    }

    T sample2D(float u, float v, FilterMode filter_mode = Filter_NEAREST) {
        float x = u * width_, y = v * height_;
        if (filter_mode == Filter_NEAREST) {
            return getPixel(x, y);
        }

        if (filter_mode == Filter_LINEAR) {
            int x1 = static_cast<int>(x);
            int y1 = static_cast<int>(y);
            int x2 = x1 + 1;
            int y2 = y1 + 1;

            T Q11 = getPixel(x1, y1);
            T Q21 = getPixel(x2, y1);
            T Q12 = getPixel(x1, y2);
            T Q22 = getPixel(x2, y2);

            // glm::uvec4不支持乘法，转换到glm::vec4进行计算
            auto lerp = [](const T& a, const T& b, const float t)-> T {
                return glm::vec4(a) + glm::vec4(b) * t;
            };

            T R1 = lerp(Q11, Q21, x - x1);
            T R2 = lerp(Q12, Q22, x - x1);

            T P = lerp(R1, R2, y - y1);

            return P;
        }
        return {};
    }

    void set(int x, int y, const T& value)
    {
        data_[getIndex(x, y)] = value;
    }

    void clear() {
        memset(data_.data(), 0, data_.size());
    }

    int width() const {
        return width_;
    }

    int height() const {
        return height_;
    }

    const T* rawData() {
        return data_.data();
    }

    [[nodiscard]] size_t size() const {
        return data_.size();
    }

    template<typename Y>
    Buffer<T>& copyFrom(const Buffer<Y>& other) {
        width_ = other.width();
        height_ = other.height();
        data_.resize(width_ * height_);
        for (int i = 0; i < width_; i++) {
            for (int j = 0; j < height_; j++) {
                set(i, j, T(other.getPixel(i, j)));
            }
        }
        return *this;
    }

    int getComponentsNum() const {
        return T::length();
    }

    void write_to_file() {

    }
};

template<typename T>
template<typename ... Args>
std::shared_ptr<Buffer<T>> Buffer<T>::makeBuffer(Args &&...args) {
    return std::shared_ptr<Buffer<T>>(new Buffer<T>(std::forward<Args>(args)...));
}


