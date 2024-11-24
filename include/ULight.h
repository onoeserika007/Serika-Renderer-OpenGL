#pragma once
#include <memory>
#include <string>
#include "Base/Globals.h"
#include "Geometry/UMesh.h"
#include "Geometry/Object.h"


class RenderPass;
// 派生类需要基类的定义，因此基类就要避免包括派生类
// 用前向声明替换直接include，在cpp文件中再include
class UObject;
class Shader;
class FMaterial;
class FGeometry;
class Renderer;

enum LightType: glm::uint {
    LightType_NoLight,
    LightType_PointLight,
    LightType_DirectionalLight,
    LightType_SpotLight
};

// 只能说你还没有理解std140
// #pragma pack(4)
struct LightDataUniformBlock { // uint好像和预想的数据不太一样
    alignas(4) glm::int32 uLightType = LightType_PointLight;

    alignas(16) glm::vec3 uLightPosition;
    alignas(16) glm::vec3 uLightDirection;
    alignas(16) glm::vec3 uLightAmbient;
    alignas(16) glm::vec3 uLightDiffuse;
    alignas(16) glm::vec3 uLightSpecular;

    alignas(4) glm::float32 uLightCutoff;
    alignas(4) glm::float32 uLightOuterCutoff;
    alignas(4) glm::float32 uLightConstant;
    alignas(4) glm::float32 uLightLinear;
    alignas(4) glm::float32 uLightQuadratic;

    alignas(4) glm::float32 uLightWidth;
};

class ULight: public UObject {
public:
    template<typename ...Args>
    static std::shared_ptr<ULight> makeLight(Args&&... args);

    void setAsPointLight(glm::vec3 pos, glm::vec3 ambient, glm::vec3 diff, glm::vec3 spec, float constant, float linear, float quadratic);
    void setAsDirectionalLight(glm::vec3 dir, glm::vec3 ambient, glm::vec3 diff, glm::vec3 spec);
    void setAsSpotLight(glm::vec3 pos, glm::vec3 dir, float cutoff, float outerCutoff);
    void setToShader(std::shared_ptr<Shader> pshader, int index = -1) const;
    void setColor(const glm::vec3& color);
    void setLightDiffuse(const glm::vec3& diff);
    void setPosition(const glm::vec3& position);
    void setLightRadius(float radius) { lightData_.uLightWidth = radius; }

    LightDataUniformBlock serialize() const;
    ULight& deserialize(const LightDataUniformBlock &block);

    LightType getType() const;
    bool isPointLight() const { return lightData_.uLightType == LightType_PointLight; }
    bool isDirectionalLight() const { return lightData_.uLightType == LightType_DirectionalLight; }
    bool isSpotLight () const { return lightData_.uLightType == LightType_SpotLight; }

    virtual void updateFrame() override;

    static std::shared_ptr<ULight> generateDefaultPointLight();
    static std::shared_ptr<ULight> generateDefaultDirectionalLight();
    static std::shared_ptr<ULight> generateDefaultSpotLight();

    // shadow map
    std::shared_ptr<FCamera> getLightCamera() const;
    std::shared_ptr<Texture> getShadowMap(const Renderer &renderer) const;

private:

    ULight();
    // explicit ULight(const std::shared_ptr<UMesh>& mesh);

    mutable std::shared_ptr<FCamera> camera_;
    mutable std::shared_ptr<Texture> shadowMap_;

    LightDataUniformBlock lightData_;
};

template<typename ... Args>
std::shared_ptr<ULight> ULight::makeLight(Args&&... args) {
    return std::shared_ptr<ULight>(new ULight(std::forward<Args>(args)...));
}
