#pragma once
#include <vector>

#include "BoundingBox.h"
#include "Ray.h"

class Intersectable;
struct BVHBuildNode;

struct BVHPrimitiveInfo;

struct BVHBuildNode {
    BoundingBox bbox {};
    std::unique_ptr<BVHBuildNode> left = nullptr;
    std::unique_ptr<BVHBuildNode> right = nullptr;
    std::shared_ptr<Intersectable> primitive = nullptr;

    float area;
    int splitAxis = 0;
    int firstPrimOffset = 0;
    int nPrimitives = 0;

    // BVHBuildNode Public Methods
};

class BVHAccel {

public:
    // BVHAccel Public Types
    enum class SplitMethod { NAIVE, SAH };

    // BVHAccel Public Methods
    explicit BVHAccel(const std::vector<std::shared_ptr<Intersectable>>& primitives, int maxPrimsInNode = 1, SplitMethod splitMethod = SplitMethod::NAIVE);

    NO_DISCARD Intersection Intersect(const Ray &ray) const;
    NO_DISCARD Intersection IntersectImpl(const std::unique_ptr<BVHBuildNode> &node, const Ray& ray)const;
    NO_DISCARD BoundingBox WorldBound() const;
    void getSample(const std::unique_ptr<BVHBuildNode> &node, float split, Intersection &pos, float &pdf);
    void Sample(Intersection &pos, float &pdf);

    // BVHAccel Private Methods
    std::unique_ptr<BVHBuildNode> recursiveBuild(const std::vector<std::shared_ptr<Intersectable>>& primitives);


public:

private:
    // BVHAccel Private Data
    const int maxPrimsInNode_;
    const SplitMethod splitMethod_;
    std::vector<std::shared_ptr<Intersectable>> primitives_;
    std::unique_ptr<BVHBuildNode> root_;
};
