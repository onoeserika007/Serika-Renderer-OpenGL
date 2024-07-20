#include "ULight.h"
#include "Shader.h"
#include "Material.h"
#include "Geometry/Geometry.h"
#include "Base/GLMInc.h"
#include "Camera.h"
#include "Renderer.h"
#include "Base/Config.h"

 ULight::ULight() : name_in_shader_("light") {
     lightData_.uLightType = LightType_PointLight;
 }

 ULight::ULight(std::string name, glm::vec3 ambient, glm::vec3 diff, glm::vec3 spec)
    : name_in_shader_(std::move(name)) {
     lightData_.uLightAmbient = ambient;
     lightData_.uLightDiffuse = diff;
     lightData_.uLightSpecular = spec;
     lightData_.uLightType = LightType_PointLight;
}

 ULight::ULight(std::shared_ptr<Geometry> pgeometry, std::shared_ptr<Material> pmaterial)
     : UMesh(std::move(pgeometry), std::move(pmaterial))
 {
     lightData_.uLightType = LightType_PointLight;
 }

 ULight::ULight(std::string name, std::shared_ptr<Geometry> pgeometry, std::shared_ptr<Material> pmaterial)
     : UMesh(std::move(pgeometry), std::move(pmaterial)), name_in_shader_(std::move(name))
 {
     lightData_.uLightType = LightType_PointLight;
 }

 ULight::ULight(std::string name, glm::vec3 ambient, glm::vec3 diff, glm::vec3 spec, std::shared_ptr<Geometry> pgeometry, std::shared_ptr<Material> pmaterial)
    : UMesh(std::move(pgeometry), std::move(pmaterial)), name_in_shader_(std::move(name)) {
     lightData_.uLightAmbient = ambient;
     lightData_.uLightDiffuse = diff;
     lightData_.uLightSpecular = spec;
     lightData_.uLightType = LightType_PointLight;
}

 void ULight::setName(const std::string& name) {
    name_in_shader_ = name;
}

 void ULight::setAsPointLight(glm::vec3 pos, glm::vec3 ambient, glm::vec3 diff, glm::vec3 spec, float constant, float linear, float quadratic)
 {
     setPosition(pos);
     lightData_.uLightAmbient = ambient;
     lightData_.uLightDiffuse = diff;
     lightData_.uLightSpecular = spec;
     lightData_.uLightConstant = constant;
     lightData_.uLightLinear = linear;
     lightData_.uLightQuadratic = quadratic;
     lightData_.uLightType = LightType_PointLight;
 }

 void ULight::setAsDirectionalLight(glm::vec3 dir, glm::vec3 ambient, glm::vec3 diff, glm::vec3 spec)
 {
     lightData_.uLightDirection = dir;
     lightData_.uLightAmbient = ambient;
     lightData_.uLightDiffuse = diff;
     lightData_.uLightSpecular = spec;
     lightData_.uLightType = LightType_DirectionalLight;
 }

 void ULight::setAsSpotLight(glm::vec3 pos, glm::vec3 dir, float cutoff, float outerCutoff)
 {
     setPosition(pos);
     lightData_.uLightDirection = dir;
     lightData_.uLightCutoff = cutoff;
     lightData_.uLightOuterCutoff = outerCutoff;
     lightData_.uLightType = LightType_SpotLight;
 }

 void ULight::setToShader(std::shared_ptr<Shader> pshader, int index) const {

}

 void ULight::setColor(const glm::vec3& color)
 {
     lightData_.uLightAmbient = color;
     lightData_.uLightDiffuse = color;
     lightData_.uLightSpecular = color;
 }

void ULight::setPosition(const glm::vec3 &position) {
     UObject::setPosition(position);
     lightData_.uLightPosition = position;
}

LightDataUniformBlock ULight::serialize() const {
     return lightData_;
}

ULight& ULight::deserialize(const LightDataUniformBlock &block) {
     lightData_ = block;
     return *this;
}

LightType ULight::getType() const
 {
     return static_cast<LightType>(lightData_.uLightType);
 }

 std::string ULight::getName() const
 {
     return name_in_shader_;
 }

 void ULight::updateFrame(Renderer& renderer) {
    UObject::updateFrame(renderer);

    if (lightData_.uLightType == LightType_SpotLight) {
        lightData_.uLightDirection = renderer.getCamera().forward();
        setPosition(renderer.getCamera().position());
    }
}

std::shared_ptr<ULight> ULight::generateDefaultPointLight() {
     return {};
}

std::shared_ptr<ULight> ULight::generateDefaultDirectionalLight() {
     return {};
}

std::shared_ptr<ULight> ULight::generateDefaultSpotLight() {
     return {};
}

std::shared_ptr<Camera> ULight::getLightCamera() {
     if (!camera_) {
         auto&& config = Config::getInstance();
         if (lightData_.uLightType == LightType_DirectionalLight) {
             float radius = config.CaptureRadius_ShadowMap;
             camera_ = std::make_shared<OrthographicCamera>(-radius, radius, -radius, radius, config.CameraNear, config.CameraFar);
             camera_->setPosition(getPosition());
             camera_->lookAt(getPosition() + lightData_.uLightDirection);
         }
         else {
             camera_ = std::make_shared<PerspectiveCamera>(config.CameraFOV, config.CameraAspect, config.CameraNear, config.CameraFar);
             camera_->setPosition(getPosition());
             // TODO: PointLight | SpotLight ShadowCamera
         }
     }
     return camera_;
}

std::shared_ptr<Texture> ULight::getShadowMap(RenderPass &renderPass) {
     if (!shadowMap_) {
         const Config& config = Config::getInstance();
         renderPass.setupShadowMapBuffer(shadowMap_, config.Resolution_ShadowMap, config.Resolution_ShadowMap, false);
     }
     return shadowMap_;
}


void ULight::markLightVPDirty() {
     lightVP_ = nullptr;
}
