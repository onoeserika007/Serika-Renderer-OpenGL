//
// Created by Inory on 2024/7/20.
//
#pragma once
#include "Geometry/UObject.h"


class UMesh: public UObject{
public:
    UMesh();
    UMesh(std::shared_ptr<Geometry> pgeometry, std::shared_ptr<Material> pmaterial);

    // getters
    std::shared_ptr<Geometry> getpGeometry();
    std::shared_ptr<Material> getpMaterial();
    unsigned getVAO() const;
    ShadingMode getShadingMode() const;

    // setters
    void setVAO(unsigned id);
    void setShadingMode(ShadingMode shadingMode);

private:
    // mesh
    std::shared_ptr<Geometry> pgeometry_;
    std::shared_ptr<Material> pmaterial_;
    ShadingMode shadingMode_ = Shading_BaseColor;
    unsigned VAO;
};



