#pragma once
#include <memory>
#include <string>
#include "Base/GLMInc.h"
#include "Geometry/UMesh.h"
#include "Geometry/UObject.h"


class RenderPass;
// 派生类需要基类的定义，因此基类就要避免包括派生类
// 用前向声明替换直接include，在cpp文件中再include
class UObject;
class Shader;
class Material;
class Geometry;
class Renderer;

const unsigned MAX_LIGHT_NUMS = 10;

// struct PointLightData: LightData {
//     glm::vec3 position;
//
//     glm::vec3 ambient;
//     glm::vec3 diffuse;
//     glm::vec3 specular;
//
//     float constant;
//     float linear;
//     float quadratic;
// };
//
// struct DirectionalLightData: LightData {
//     glm::vec3 direction;
//
//     glm::vec3 ambient;
//     glm::vec3 diffuse;
//     glm::vec3 specular;
// };
//
// struct SpotLightData : LightData {
//     glm::vec3 position;
//     glm::vec3 direction;
//     float cutoff;
//     float outerCutoff;
// };

// struct PointLightUniformBlock {
//     PointLightData PointLights[MAX_LIGHT_NUMS];
//     int numPointLight;
// };
//
// struct DirectionalLightUniformBlock {
//     DirectionalLightData DirectionalLights[MAX_LIGHT_NUMS];
//     int numDirectionalLight;
// };
//
// struct SpotLightUniformBlock {
//     SpotLightData SpotLights[MAX_LIGHT_NUMS];
//     int numSpotLight;
// };

enum LightType: glm::uint {
    LightType_NoLight,
    LightType_PointLight,
    LightType_DirectionalLight,
    LightType_SpotLight
};

// 只能说你还没有理解std140
#pragma pack(4)
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
};

class ULight: public UMesh{
public:
    ULight();
    ULight(std::string name, glm::vec3 ambient, glm::vec3 diff, glm::vec3 spec);
    ULight(std::shared_ptr<Geometry> pgeometry, std::shared_ptr<Material> pmaterial);
    ULight(std::string name, std::shared_ptr<Geometry> pgeometry, std::shared_ptr<Material> pmaterial);
    ULight(std::string name, glm::vec3 ambient, glm::vec3 diff, glm::vec3 spec, std::shared_ptr<Geometry> pgeometry, std::shared_ptr<Material> pmaterial);
    void setName(const std::string& name);
    void setAsPointLight(glm::vec3 pos, glm::vec3 ambient, glm::vec3 diff, glm::vec3 spec, float constant, float linear, float quadratic);
    void setAsDirectionalLight(glm::vec3 dir, glm::vec3 ambient, glm::vec3 diff, glm::vec3 spec);
    void setAsSpotLight(glm::vec3 pos, glm::vec3 dir, float cutoff, float outerCutoff);
    void setToShader(std::shared_ptr<Shader> pshader, int index = -1) const;
    void setColor(const glm::vec3& color);
    void setPosition(const glm::vec3& position);

    LightDataUniformBlock serialize() const;
    ULight& deserialize(const LightDataUniformBlock &block);
    LightType getType() const;
    std::string getName() const;

    virtual void updateFrame(Renderer& renderer) override;

    static std::shared_ptr<ULight> generateDefaultPointLight();
    static std::shared_ptr<ULight> generateDefaultDirectionalLight();
    static std::shared_ptr<ULight> generateDefaultSpotLight();

    std::shared_ptr<Camera> getLightCamera();
    std::shared_ptr<Texture> getShadowMap(RenderPass& renderPass);
    // glm::mat4 getLightVP();
    void markLightVPDirty();

private:
    std::string name_in_shader_;
    std::shared_ptr<Camera> camera_;
    std::shared_ptr<Texture> shadowMap_;
    std::shared_ptr<glm::mat4> lightVP_;

    LightDataUniformBlock lightData_;
};