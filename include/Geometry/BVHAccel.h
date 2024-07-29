#pragma once
#include <vector>

#include "BoundingBox.h"
#include "Ray.h"

class Intersectable;
struct BVHNode;

struct BVHPrimitiveInfo;

struct BVHNode {
    BoundingBox bbox {};
    std::shared_ptr<BVHNode> left = nullptr;
    std::shared_ptr<BVHNode> right = nullptr;
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
    NO_DISCARD Intersection IntersectImpl(const std::shared_ptr<BVHNode> &node, const Ray& ray)const;
    NO_DISCARD BoundingBox WorldBound() const;
    void Sample(Intersection &pos, float &pdf);

    NO_DISCARD std::shared_ptr<BVHNode> getRoot() const { return root_; }

public:

private:
    // BVHAccel Private Methods
    std::shared_ptr<BVHNode> recursiveBuild(const std::vector<std::shared_ptr<Intersectable>>& primitives);

    void getSample(const std::shared_ptr<BVHNode> &node, float split, Intersection &pos, float &pdf);

    // BVHAccel Private Data
    const int maxPrimsInNode_;
    const SplitMethod splitMethod_;
    std::vector<std::shared_ptr<Intersectable>> primitives_;
    std::shared_ptr<BVHNode> root_;
};
