#pragma once
#include "BoundingBox.h"
#include "Primitives.h"
#include "Base/Globals.h"
#include "Material/FMaterial.h"

struct Triangle_Encoded {
    glm::vec3 v0_, v1_, v2_;    // 顶点坐标
    glm::vec3 n0_, n1_, n2_;    // 顶点法线
    glm::vec2 t0_, t1_, t2_;
    glm::vec3 emissive_;      // 自发光参数
    glm::vec3 baseColor_;     // 颜色
    glm::vec3 param1_;        // (subsurface, metallic, specular)
    glm::vec3 param2_;        // (specularTint, roughness, anisotropic)
    glm::vec3 param3_;        // (sheen, sheenTint, clearcoat)
    glm::vec3 param4_;        // (clearcoatGloss, IOR, transmission)
};

class Triangle : public Intersectable, public std::enable_shared_from_this<Triangle>, public PipelineLoadable
{
public:
    glm::vec3 v0_{}, v1_{}, v2_{}; // vertices A, B ,C , counter-clockwise order
    glm::vec3 e1_{}, e2_{};     // 2 edges v1-v0, v2-v0;
    glm::vec2 t0_{}, t1_{}, t2_{}; // texture coords
    glm::vec3 n0_{}, n1_{}, n2_{};
    glm::vec3 normal_{};
    float area_{};
    std::weak_ptr<FMaterial> material_;

    // debugs
    Serika::UUID<Triangle> uuid_;
    float persistTime_ = 0.f;
    unsigned VAO_ = 0;

    int getUUID() const { return uuid_.get(); }

    Triangle();
    virtual ~Triangle() override;
    Triangle(const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2, const std::shared_ptr<FMaterial>& m = {});

    Triangle_Encoded encode() const;
    Triangle& deconde(const Triangle_Encoded& triangle_encoded);
    bool intersect(const Ray& ray) override;
    virtual Intersection getIntersection(const Ray &ray) override;
    BoundingBox getBounds() const override;

    // 三角形内的均匀采样
    void Sample(Intersection &pos, float &pdf) override;
    float getArea() const override;
    bool hasEmit() const override;
    virtual void transform(const glm::mat4 &modelMatrix) override;
};
