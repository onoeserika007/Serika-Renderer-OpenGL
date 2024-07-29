#include "Geometry/UMesh.h"

#include "Renderer.h"
#include "Geometry/Geometry.h"
#include "Geometry/Triangle.h"

UMesh::UMesh() {
    pgeometry_ = nullptr;
    pgeometry_ = nullptr;
    VAO = 0;
    shadingMode_ = Shading_BaseColor;
}

UMesh::UMesh(const UMesh &other): enable_shared_from_this() {
    pgeometry_ = other.pgeometry_;
    pmaterial_ = other.pmaterial_;
    VAO = other.VAO;
    bUseCull = other.bUseCull;
}

UMesh::UMesh(UMesh &&other) noexcept : enable_shared_from_this() {
    pgeometry_ = std::move(other.pgeometry_);
    pmaterial_ = std::move(other.pmaterial_);
    VAO = other.VAO;
    bUseCull = other.bUseCull;
}


UMesh::UMesh(std::shared_ptr<FGeometry> pgeometry, std::shared_ptr<FMaterial> pmaterial): UMesh() {
    pgeometry_ = std::move(pgeometry);
    pmaterial_ = std::move(pmaterial);

}

std::shared_ptr<FGeometry> UMesh::getGeometry() const {
    return pgeometry_;
}

std::shared_ptr<FMaterial> UMesh::getMaterial() const {
    return pmaterial_;
}

std::shared_ptr<UObject> UMesh::getParentObject() const {
    return parentObject_.lock();
}

glm::mat4 UMesh::getWorldMatrix() const {
    if (auto&& parent = parentObject_.lock()) {
        return parent->getWorldMatrix();
    }
    return worldMatrix_;
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

/** Note that meshes now still has hierarchy!! So recursive */
void UMesh::setParentObject(const std::shared_ptr<UObject> &obj) {
    parentObject_ = obj;
    for (auto&& submesh: submeshes_) {
        submesh->setParentObject(obj);
    }
}

void UMesh::setEmission(const glm::vec3 &EmissiveColor) const {
    if (pmaterial_) {
        pmaterial_->setEmission(EmissiveColor);
    }
    for (auto&& submesh: submeshes_) {
        submesh->setEmission(EmissiveColor);
    }
}

void UMesh::setDiffuse(const glm::vec3 &DiffuseColor) const {
    if (pmaterial_) {
        pmaterial_->setDiffuse(DiffuseColor);
    }
    for (auto&& submesh: submeshes_) {
        submesh->setDiffuse(DiffuseColor);
    }
}

bool UMesh::isPipelineReady() const {
    return (!pmaterial_ || pmaterial_->isPipelineReady()) && (!pgeometry_ || pgeometry_->isPipelineReady()) && VAO;
}

