//
// Created by Inory on 2024/7/20.
//
#pragma once
#include "BoundingBox.h"
#include "Primitives.h"
#include "Geometry/Object.h"

class BVHAccel;
class Triangle;

// multi inheritance? Think twice
class UMesh:  public std::enable_shared_from_this<UMesh> {
public:

    template<typename ...Args>
    static std::shared_ptr<UMesh> makeMesh(Args&&... args);

    // 只保证clone出来的对象是子类，返回指针还是基类
    // virtual std::unique_ptr<UObject> Clone() const override { return std::unique_ptr<UMesh>(new UMesh(*this)); }
    // virtual void init() override;
    std::shared_ptr<UMesh> Clone();

    // getters
    std::shared_ptr<FGeometry> getGeometry() const;
    std::shared_ptr<FMaterial> getMaterial() const;;
    std::shared_ptr<UObject> getParentObject() const;
    const std::vector<std::shared_ptr<UMesh>>& getMeshes() const { return submeshes_; }
    glm::mat4 getWorldMatrix() const;
    std::shared_ptr<UniformSampler> tryGetSkyboxSampler(Renderer &renderer);
    bool useCull() const  { return bUseCull; }
    bool drawable() const  { return pgeometry_ && pmaterial_; }
    bool castShadow() const { return bCastShadow; }
    EShadingModel getShadingMode() const { return shadingMode_; }

    std::unique_ptr<Triangle> fetchTriangle(unsigned k) const;

    // setters
    void setGeometry(const std::shared_ptr<FGeometry>& geo);

    void setMaterial(const std::shared_ptr<FMaterial>& mat);

    void setParentObject(const std::shared_ptr<UObject>& obj);
    void enableFaceCull(const bool bEnabled);

    void enableCastShadow(bool bEnabled);

    void setShadingMode(EShadingModel shadingMode);

    void setEmission(const glm::vec3& EmissiveColor) const;
    void setDiffuse(const glm::vec3& DiffuseColor) const;
    void addMesh(const std::shared_ptr<UMesh>& mesh);

    bool isPipelineReady() const;

    UMesh& operator=(const UMesh& other) = delete;


    void updateWorldMatrix(const glm::mat4& worldMatrix) const { worldMatrix_ = worldMatrix; }

public:

protected:
    UMesh();
    UMesh(const UMesh& other);
    UMesh(UMesh&& other) noexcept;
    UMesh(std::shared_ptr<FGeometry> pgeometry, std::shared_ptr<FMaterial> pmaterial);

private:

    // mesh
    std::shared_ptr<FGeometry> pgeometry_;
    std::shared_ptr<FMaterial> pmaterial_;
    bool bUseCull = true;
    bool bCastShadow = false;
    EShadingModel shadingMode_ = Shading_BaseColor;

    // subMeshes
    std::vector<std::shared_ptr<UMesh>> submeshes_;

    mutable glm::mat4 worldMatrix_ {1.f};
    std::weak_ptr<UObject> parentObject_;
};

template<typename ...Args>
std::shared_ptr<UMesh> UMesh::makeMesh(Args &&...args) {
    auto&& newMesh = std::shared_ptr<UMesh>(new UMesh(std::forward<Args>(args)...));
    return newMesh;
}



