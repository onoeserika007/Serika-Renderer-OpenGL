#include "Geometry/UMesh.h"
#include <iostream>

#include "Renderer.h"
#include "Geometry/BVHAccel.h"
#include "Geometry/Geometry.h"
#include "Geometry/Triangle.h"

UMesh::UMesh() {
    pgeometry_ = nullptr;
    pgeometry_ = nullptr;
    VAO = 0;
    shadingMode_ = Shading_BaseColor;
    bDrawable = true;

    // intersectable
}

UMesh::UMesh(const UMesh &other): UObject(other), area_(), bbox_(), triangles_(), bvh_accel_() {
    pgeometry_ = other.pgeometry_;
    pmaterial_ = other.pmaterial_;
    VAO = other.VAO;
    bPipelineSetup = other.bPipelineSetup;
    bUseCull = other.bUseCull;

    // buildBVH
    buildBVH();
}

UMesh::UMesh(std::shared_ptr<FGeometry> pgeometry, std::shared_ptr<FMaterial> pmaterial): UMesh() {
    pgeometry_ = std::move(pgeometry);
    pmaterial_ = std::move(pmaterial);

    // buildBVH
    buildBVH();
}

void UMesh::buildBVH() {
    // triangles
    if (pgeometry_) {
        for(int i = 0; i < pgeometry_->getTriangleNum(); i++) {
            auto&& tri = fetchTriangle(i);
            bbox_.merge(tri->getBounds());
            area_ += tri->getArea();
            triangles_.emplace_back(std::move(tri));
        }
        bvh_accel_ = std::make_unique<BVHAccel>(triangles_);
    }
}

void UMesh::init() {
    UObject::init(); // super
    
}

std::shared_ptr<FGeometry> UMesh::getGeometry() const {
    return pgeometry_;
}

std::shared_ptr<FMaterial> UMesh::getMaterial() const {
    return pmaterial_;
}

std::shared_ptr<UniformSampler> UMesh::tryGetSkyboxSampler(const Renderer &renderer) {
    // we know the cast definitely successful
    renderer.setupMesh(std::static_pointer_cast<UMesh>(shared_from_this()), ShaderPass::Shader_ForwardShading_Pass); // we assume skybox only work in forwarding pass.
    if (pmaterial_) {
        return pmaterial_->getUniformSampler(Texture::samplerName(TEXTURE_TYPE_CUBE));
    }
    return {};
}


std::unique_ptr<Triangle> UMesh::fetchTriangle(unsigned k) const {
    if (pgeometry_) {
        auto&& tri = pgeometry_->fetchTriangle(k);
        tri->material_ = pmaterial_;
        return tri;
    }
    return {};
}

void UMesh::setPipelineReady(bool ready) {
    bPipelineSetup = ready;
}

bool UMesh::isPipelineReady() const {
    return (!pmaterial_ || pmaterial_->isPipelineReady()) && (!pgeometry_ || pgeometry_->isPipelineReady()) && VAO;
}

Intersection UMesh::getIntersection(const Ray &ray) {
    if (bvh_accel_) {
        return bvh_accel_->Intersect(ray);
    }
    return {};
}

void UMesh::Sample(Intersection &pos, float &pdf) {
    if (bvh_accel_) {
        bvh_accel_->Sample(pos, pdf);
    }
}

