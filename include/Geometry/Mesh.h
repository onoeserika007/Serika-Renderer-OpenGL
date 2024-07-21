//
// Created by Inory on 2024/7/20.
//
#pragma once
#include "Geometry/Object.h"


class UMesh: public UObject{
public:

    template<typename ...Args>
    static std::shared_ptr<UMesh> makeMesh(Args&&... args);

    // getters
    std::shared_ptr<Geometry> getpGeometry();
    std::shared_ptr<Material> getpMaterial();;
    unsigned getVAO() const;
    EShadingMode getShadingMode() const;

    // setters
    void setVAO(unsigned id);
    void setShadingMode(EShadingMode shadingMode);

    ///
    /// pipeline
    ///
    void setPipelineReady(bool ready) {
        bPipelineSetup = ready;
    }

    bool isPipelineReady() const {
        return bPipelineSetup;
    }

protected:
    UMesh();
    UMesh(std::shared_ptr<Geometry> pgeometry, std::shared_ptr<Material> pmaterial);

private:
    // mesh
    std::shared_ptr<Geometry> pgeometry_;
    std::shared_ptr<Material> pmaterial_;
    EShadingMode shadingMode_ = Shading_BaseColor;
    unsigned VAO;
    bool bPipelineSetup = false;
};

template<typename ...Args>
std::shared_ptr<UMesh> UMesh::makeMesh(Args &&...args) {
    return std::shared_ptr<UMesh>(new UMesh(std::forward<Args>(args)...));
}



