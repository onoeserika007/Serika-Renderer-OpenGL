#include "Geometry/UMesh.h"
#include <iostream>

UMesh::UMesh(): UObject() {
    pgeometry_ = nullptr;
    pgeometry_ = nullptr;
    VAO = 0;
    shadingMode_ = Shading_BaseColor;
}

UMesh::UMesh(std::shared_ptr<Geometry> pgeometry, std::shared_ptr<Material> pmaterial): UMesh() {
    pgeometry_ = std::move(pgeometry);
    pmaterial_ = std::move(pmaterial);

    if (!pgeometry_) {
        std::cout << "Object: init - Objrct has no geometry or has a pure point!" << std::endl;
    }
    else {
        init();
    }
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

ShadingMode UMesh::getShadingMode() const {
    return shadingMode_;
}

void UMesh::setVAO(unsigned id) {
    VAO = id;
}

void UMesh::setShadingMode(ShadingMode shadingMode) {
    shadingMode_ = shadingMode;
}
