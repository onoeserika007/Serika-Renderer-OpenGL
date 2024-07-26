//
// Created by Inory on 2024/7/20.
//
#pragma once
#include "BoundingBox.h"
#include "Primitives.h"
#include "Geometry/Object.h"
#include "Geometry/BVHAccel.h"

class BVHAccel;
class Triangle;

// multi inheritance? Think twice
class UMesh: public UObject, public Intersectable{
public:

    template<typename ...Args>
    static std::shared_ptr<UMesh> makeMesh(Args&&... args);

    // 只保证clone出来的对象是子类，返回指针还是基类
    virtual std::unique_ptr<UObject> Clone() const override { return std::unique_ptr<UMesh>(new UMesh(*this)); }
    virtual void init() override;

    ~UMesh() override = default;

    // getters
    std::shared_ptr<FGeometry> getGeometry() const;
    std::shared_ptr<FMaterial> getMaterial() const;;
    std::shared_ptr<UniformSampler> tryGetSkyboxSampler(const Renderer &renderer);
    bool useCull() const  { return bUseCull; }

    std::unique_ptr<Triangle> fetchTriangle(unsigned k) const;

    // setters
    void setGeometry(const std::shared_ptr<FGeometry>& geo) { pgeometry_ = geo; }
    void setMaterial(const std::shared_ptr<FMaterial>& mat) { pmaterial_ = mat; }
    void enableFaceCull(const bool bEnabled) { bUseCull = bEnabled; }

    ///
    /// pipeline
    ///
    void setPipelineReady(bool ready);
    bool isPipelineReady() const;

    UMesh& operator=(const UMesh& other) = delete;

    ///
    /// Inherited from Primitive
    ///
    virtual bool intersect(const Ray& ray) override { return true; }
    // virtual bool intersect(const Ray& ray, float &, uint32_t &) const = 0;
    virtual Intersection getIntersection(const Ray &ray) override;

    virtual BoundingBox getBounds() const override{ return bbox_; }
    virtual float getArea() const override { return area_; }
    virtual void Sample(Intersection &pos, float &pdf) override;
    virtual bool hasEmit() const override { return true; /* never to be called since mesh not directly intersect */ }

public:
    mutable unsigned VAO;

protected:
    UMesh();
    UMesh(const UMesh& other);
    UMesh(std::shared_ptr<FGeometry> pgeometry, std::shared_ptr<FMaterial> pmaterial);

private:
    void buildBVH();

    // mesh
    std::shared_ptr<FGeometry> pgeometry_;
    std::shared_ptr<FMaterial> pmaterial_;
    bool bPipelineSetup = false;
    bool bUseCull = true;

    // intersectable
    float area_ = 0.f;
    BoundingBox bbox_;
    std::vector<std::shared_ptr<Intersectable>> triangles_;
    std::unique_ptr<BVHAccel> bvh_accel_;
};

template<typename ...Args>
std::shared_ptr<UMesh> UMesh::makeMesh(Args &&...args) {
    return std::shared_ptr<UMesh>(new UMesh(std::forward<Args>(args)...));
}



