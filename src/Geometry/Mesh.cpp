#include "Geometry/Mesh.h"
#include <iostream>

#include "Renderer.h"

UMesh::UMesh() {
    pgeometry_ = nullptr;
    pgeometry_ = nullptr;
    VAO = 0;
    shadingMode_ = Shading_BaseColor;
    bDrawable = true;
}

UMesh::UMesh(std::shared_ptr<Geometry> pgeometry, std::shared_ptr<Material> pmaterial): UMesh() {
    pgeometry_ = std::move(pgeometry);
    pmaterial_ = std::move(pmaterial);
}

std::shared_ptr<Geometry> UMesh::getpGeometry() {
    return pgeometry_;
}

std::shared_ptr<Material> UMesh::getpMaterial() {
    return pmaterial_;
}

unsigned UMesh::getVAO() const {
    return VAO;
}


void UMesh::setVAO(unsigned id) {
    VAO = id;
}


void UMesh::setPipelineReady(bool ready) {
    bPipelineSetup = ready;
}

bool UMesh::isPipelineReady() const {
    return bPipelineSetup;
}
