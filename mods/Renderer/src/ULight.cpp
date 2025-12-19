#include "ULight.h"
#include "Material/Shader.h"
#include "FCamera.h"
#include "Renderer.h"
#include "Base/Config.h"

ULight::ULight() {
    lightData_.uLightType = LightType_PointLight;
}

// ULight::ULight(const std::shared_ptr<UMesh> &mesh): UObject(mesh) {
//     lightData_.uLightType = LightType_PointLight;
// }

void ULight::setAsPointLight(glm::vec3 pos, glm::vec3 ambient, glm::vec3 diff, glm::vec3 spec, float constant,
                             float linear, float quadratic) {
    setPosition(pos);
    lightData_.uLightAmbient = ambient;
    lightData_.uLightDiffuse = diff;
    lightData_.uLightSpecular = spec;
    lightData_.uLightConstant = constant;
    lightData_.uLightLinear = linear;
    lightData_.uLightQuadratic = quadratic;
    lightData_.uLightType = LightType_PointLight;
}

void ULight::setAsDirectionalLight(glm::vec3 dir, glm::vec3 ambient, glm::vec3 diff, glm::vec3 spec) {
    lightData_.uLightDirection = dir;
    lightData_.uLightAmbient = ambient;
    lightData_.uLightDiffuse = diff;
    lightData_.uLightSpecular = spec;
    lightData_.uLightType = LightType_DirectionalLight;
}

void ULight::setAsSpotLight(glm::vec3 pos, glm::vec3 dir, float cutoff, float outerCutoff) {
    setPosition(pos);
    lightData_.uLightDirection = dir;
    lightData_.uLightCutoff = cutoff;
    lightData_.uLightOuterCutoff = outerCutoff;
    lightData_.uLightType = LightType_SpotLight;
}

void ULight::setToShader(std::shared_ptr<Shader> pshader, int index) const {
}

void ULight::setColor(const glm::vec3 &color) {
    lightData_.uLightAmbient = color;
    lightData_.uLightDiffuse = color;
    lightData_.uLightSpecular = color;
}

void ULight::setLightDiffuse(const glm::vec3 &diff) {
    lightData_.uLightDiffuse = diff;
}

void ULight::setPosition(const glm::vec3 &position) {
    UObject::setPosition(position);
    lightData_.uLightPosition = position;
}

LightDataUniformBlock ULight::serialize() const {
    return lightData_;
}

ULight &ULight::deserialize(const LightDataUniformBlock &block) {
    lightData_ = block;
    return *this;
}

LightType ULight::getType() const {
    return static_cast<LightType>(lightData_.uLightType);
}

void ULight::updateFrame() {
    UObject::updateFrame();
    // 感觉相聚可以放在一个全局的单例类里获取
    // if (lightData_.uLightType == LightType_SpotLight) {
    //     lightData_.uLightDirection = renderer.getCamera()->forward();
    //     setPosition(renderer.getCamera()->position());
    // }
}

std::shared_ptr<ULight> ULight::generateDefaultPointLight() {
    return {};
}

std::shared_ptr<ULight> ULight::generateDefaultDirectionalLight() {
    return {};
}

std::shared_ptr<ULight> ULight::generateDefaultSpotLight() {
    return {};;
}

std::shared_ptr<FCamera> ULight::getLightCamera() const {
    if (!camera_) {
        auto &&config = Config::getInstance();
        if (lightData_.uLightType == LightType_DirectionalLight) {
            float radius = config.CaptureRadius_ShadowMap;
            camera_ = std::make_shared<OrthographicCamera>(-radius, radius, -radius, radius, config.CameraNear,
                                                           config.CameraFar);
            camera_->setPosition(getWorldPosition());
            camera_->lookAt(getWorldPosition() + lightData_.uLightDirection);;
        } else {
            camera_ = std::make_shared<PerspectiveCamera>(90.f, 1.f, config.CameraNear, config.CameraFar);
            camera_->setPosition(getWorldPosition());
            camera_->lookAt(getWorldPosition() + lightData_.uLightDirection);
            // TODO: PointLight | SpotLight ShadowCamera
        }
    };
    return camera_;
}

std::shared_ptr<Texture> ULight::getShadowMap(const Renderer &renderer) const {
    if (!shadowMap_) {
        const Config &config = Config::getInstance();
        renderer.setupShadowMapBuffer(
            shadowMap_, config.Resolution_ShadowMap, config.Resolution_ShadowMap,
            false,
            isPointLight()); // use cube shadow map if point light
    }
    return shadowMap_;
};;
