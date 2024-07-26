#pragma once
#include <memory>

#include "Ray.h"

class BoundingBox;
class UMesh;
class FGeometry;
struct Ray;
/* 在 C++ 中，使用 const 关键字修饰的变量默认是被视为内部链接（internal linkage）。
这意味着，即使多个源文件中都包含了相同的 const 常量定义，这些常量在链接时会被视为是相互独立的，不会引起多次定义的问题 */

class Intersectable {
public:
    Intersectable() = default;
    virtual ~Intersectable() = default;

    virtual bool intersect(const Ray& ray) = 0;
    // virtual bool intersect(const Ray& ray, float &, uint32_t &) const = 0;
    virtual Intersection getIntersection(const Ray &ray) = 0;

    virtual BoundingBox getBounds() const = 0;
    virtual float getArea() const = 0;
    // for lights' importance sampling
    // this kind of sampling wont't depends on uniform hemisphere sampling, so won't use intersction
    virtual void Sample(Intersection &pos, float &pdf) = 0; // for lights' importance sampling
    virtual bool hasEmit() const = 0;
};

namespace Cube {
    std::shared_ptr<UMesh> loadCubeMesh(bool bReverseFace = false);

    constexpr float CubeVertices[] = {
        // 前面
        -1.0f, -1.0f, 1.0f, // 左下角
        1.0f, -1.0f, 1.0f, // 右下角
        1.0f, 1.0f, 1.0f, // 右上角
        -1.0f, 1.0f, 1.0f, // 左上角

        // 后面
        -1.0f, -1.0f, -1.0f, // 左下角
        1.0f, -1.0f, -1.0f, // 右下角
        1.0f, 1.0f, -1.0f, // 右上角
        -1.0f, 1.0f, -1.0f, // 左上角

        // 左面
        -1.0f, -1.0f, -1.0f, // 左下角
        -1.0f, -1.0f, 1.0f, // 右下角
        -1.0f, 1.0f, 1.0f, // 右上角
        -1.0f, 1.0f, -1.0f, // 左上角

        // 右面
        1.0f, -1.0f, -1.0f, // 左下角
        1.0f, -1.0f, 1.0f, // 右下角
        1.0f, 1.0f, 1.0f, // 右上角
        1.0f, 1.0f, -1.0f, // 左上角

        // 顶面
        -1.0f, 1.0f, -1.0f, // 左下角
        1.0f, 1.0f, -1.0f, // 右下角
        1.0f, 1.0f, 1.0f, // 右上角
        -1.0f, 1.0f, 1.0f, // 左上角

        // 底面
        -1.0f, -1.0f, -1.0f, // 左下角
        1.0f, -1.0f, -1.0f, // 右下角
        1.0f, -1.0f, 1.0f, // 右上角
        -1.0f, -1.0f, 1.0f, // 左上角
    };

    constexpr float CubeUVs[] = {
        // 前面
        0.0f, 0.0f,
        1.0f, 0.0f,
        1.0f, 1.0f,
        0.0f, 1.0f,

        // 后面
        1.0f, 0.0f,
        0.0f, 0.0f,
        0.0f, 1.0f,
        1.0f, 1.0f,

        // 左面
        0.0f, 0.0f,
        1.0f, 0.0f,
        1.0f, 1.0f,
        0.0f, 1.0f,

        // 右面
        0.0f, 0.0f,
        1.0f, 0.0f,
        1.0f, 1.0f,
        0.0f, 1.0f,

        // 顶面
        0.0f, 0.0f,
        1.0f, 0.0f,
        1.0f, 1.0f,
        0.0f, 1.0f,

        // 底面
        0.0f, 0.0f,
        1.0f, 0.0f,
        1.0f, 1.0f,
        0.0f, 1.0f,
    };

    constexpr float CubeNormals[] = {
        // 前面
        0.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 1.0f,
        0.0f, 0.0f, 1.0f,

        // 后面
        0.0f, 0.0f, -1.0f,
        0.0f, 0.0f, -1.0f,
        0.0f, 0.0f, -1.0f,
        0.0f, 0.0f, -1.0f,

        // 左面
        -1.0f, 0.0f, 0.0f,
        -1.0f, 0.0f, 0.0f,
        -1.0f, 0.0f, 0.0f,
        -1.0f, 0.0f, 0.0f,

        // 右面
        1.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 0.0f,

        // 顶面
        0.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 0.0f,

        // 底面
        0.0f, -1.0f, 0.0f,
        0.0f, -1.0f, 0.0f,
        0.0f, -1.0f, 0.0f,
        0.0f, -1.0f, 0.0f,
    };;

    constexpr unsigned int CubeIndices[] = {
        // 前面
        0, 1, 2,
        2, 3, 0,

        // 后面
        4, 6, 5,
        6, 4, 7,

        // 左面
        8, 9, 10,
        10, 11, 8,

        // 右面
        12, 14, 13,
        14, 12, 15,

        // 顶面
        16, 18, 17,
        18, 16, 19,

        // 底面
        20, 21, 22,
        22, 23, 20,
    };

    constexpr unsigned int CubeReverseFaceIndices[] = {
        // 前面
        0, 2, 1,
        2, 0, 3,

        // 后面
        4, 5, 6,
        6, 7, 4,

        // 左面
        8, 10, 9,
        10, 8, 11,

        // 右面
        12, 13, 14,
        14, 15, 12,

        // 顶面
        16, 17, 18,
        18, 19, 16,

        // 底面
        20, 22, 21,
        22, 20, 23,
    };
}
