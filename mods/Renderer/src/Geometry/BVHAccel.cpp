#include "Geometry/BVHAccel.h"

#include <algorithm>

#include "Geometry/Primitives.h"
#include "Utils/UniversalUtils.h"

BVHAccel::BVHAccel(const std::vector<std::shared_ptr<Intersectable>>& primitives, const int maxPrimsInNode, SplitMethod splitMethod):
    maxPrimsInNode_(std::min(255, maxPrimsInNode)),
    splitMethod_(splitMethod),
    primitives_(primitives)
{
    time_t start, stop;
    time(&start);
    if (primitives_.empty())
        return;

    root_ = recursiveBuild(primitives_);

    time(&stop);
    double diff = difftime(stop, start);
    int hrs = (int)diff / 3600;
    int mins = ((int)diff / 60) - (hrs * 60);
    int secs = (int)diff - (hrs * 3600) - (mins * 60);

    printf(
        "\rBVH Generation complete: \nTime Taken: %i hrs, %i mins, %i secs\n\n",
        hrs, mins, secs);
    fflush(stdout);
}

Intersection BVHAccel::Intersect(const Ray &ray) const {
    if (!root_) return {};
    return IntersectImpl(root_, ray);
}

Intersection BVHAccel::IntersectImpl(const std::shared_ptr<BVHNode> &node, const Ray &ray) const {
    // TODO Traverse the BVH to find intersection

    if(!node || !node->bbox.intersectRay(ray)) return {};

    if(!node->right && !node->left && node->primitive)
        return node->primitive->getIntersection(ray);

    // can not return early here, since bounds might overlap
    // so we must initialize distance by +Inf
    Intersection isect2= IntersectImpl(node->left,ray);
    Intersection isect1= IntersectImpl(node->right,ray);

    return isect1.traceDistance < isect2.traceDistance ? isect1 : isect2;
}

BoundingBox BVHAccel::WorldBound() const {
    if (root_) return root_->bbox;;
    return {};;
}

void BVHAccel::getSample(const std::shared_ptr<BVHNode> &node, float split, Intersection &pos, float &pdf) {
    if(node->left == nullptr || node->right == nullptr){
        node->primitive->Sample(pos, pdf);
        pdf *= node->area;
        return;
    }

    if(split < node->left->area) {
        getSample(node->left, split, pos, pdf);
    }
    else {
        getSample(node->right, split - node->left->area, pos, pdf);
    }
}

void BVHAccel::Sample(Intersection &pos, float &pdf) {
    float split = std::sqrt(MathUtils::get_random_float()) * root_->area;
    getSample(root_, split, pos, pdf);
    pdf /= root_->area;
}

std::shared_ptr<BVHNode> BVHAccel::recursiveBuild(const std::vector<std::shared_ptr<Intersectable>> &primitives) {
    auto&& node = std::make_shared<BVHNode>();

    std::vector<std::shared_ptr<Intersectable>> prim_cpoy {primitives.begin(), primitives.end()};

    // Compute bounds of all primitives in BVH node
    BoundingBox bounds;
    for (int i = 0; i < prim_cpoy.size(); ++i)
        bounds.merge(prim_cpoy[i]->getBounds());

    if (prim_cpoy.size() == 1) {
        // Create leaf _BVHBuildNode_
        node->bbox = prim_cpoy[0]->getBounds();
        node->primitive = prim_cpoy[0];
        node->left = nullptr;
        node->right = nullptr;
        node->area = prim_cpoy[0]->getArea();
        return node;
    }
    else if (prim_cpoy.size() == 2) {
        node->left = recursiveBuild({prim_cpoy[0]});
        node->right = recursiveBuild({prim_cpoy[1]});
        node->bbox = BoundingBox::merge(node->left->bbox, node->right->bbox);
        node->area = node->left->area + node->right->area;
        return node;
    }
    else {
        BoundingBox centroidBounds;
        for (int i = 0; i < prim_cpoy.size(); ++i)
            centroidBounds.merge(prim_cpoy[i]->getBounds().Centroid());

        int dim = centroidBounds.maxExtentAxis();
        switch (dim) {
        case 0:
            std::sort(prim_cpoy.begin(), prim_cpoy.end(), [](const auto& f1, const auto& f2) {
                return f1->getBounds().Centroid().x < f2->getBounds().Centroid().x;
            });
            break;
        case 1:
            std::sort(prim_cpoy.begin(), prim_cpoy.end(), [](const auto& f1, const auto& f2) {
                return f1->getBounds().Centroid().y < f2->getBounds().Centroid().y;
            });;
            break;
        case 2:
            std::sort(prim_cpoy.begin(), prim_cpoy.end(), [](const auto& f1, const auto& f2) {
                return f1->getBounds().Centroid().z < f2->getBounds().Centroid().z;
            });
            break;
        default: break;
        }

        auto beginning = prim_cpoy.begin();
        auto middling = prim_cpoy.begin() + (prim_cpoy.size() / 2);
        auto ending = prim_cpoy.end();

        auto leftshapes = std::vector<std::shared_ptr<Intersectable>>(beginning, middling);
        auto rightshapes = std::vector<std::shared_ptr<Intersectable>>(middling, ending);

        assert(prim_cpoy.size() == (leftshapes.size() + rightshapes.size()));

        node->left = recursiveBuild(leftshapes);
        node->right = recursiveBuild(rightshapes);

        node->bbox = BoundingBox::merge(node->left->bbox, node->right->bbox);
        node->area = node->left->area + node->right->area;
    }

    return node;
}


