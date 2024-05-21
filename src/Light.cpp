#include "Light.h"
#include "Shader.h"
#include "Material.h"
#include "Geometry.h"
#include "Base/GLMInc.h"
#include "Camera.h"

 Light::Light() : name_in_shader_("light"), type_(PointLight) {}

 Light::Light(const std::string& name, glm::vec3 ambient, glm::vec3 diff, glm::vec3 spec)
    : name_in_shader_(name), ambient_(ambient), diffuse_(diff), specular_(spec), lightColor_(1.0f), type_(PointLight) {
}

 Light::Light(std::shared_ptr<Geometry> pgeometry, std::shared_ptr<Material> pmaterial)
     : Object(pgeometry, pmaterial), lightColor_(1.0f), type_(PointLight)
 {
 }

 Light::Light(const std::string& name, std::shared_ptr<Geometry> pgeometry, std::shared_ptr<Material> pmaterial)
     : name_in_shader_(name), Object(pgeometry, pmaterial), lightColor_(1.0f), type_(PointLight)
 {
 }

 Light::Light(const std::string& name, glm::vec3 ambient, glm::vec3 diff, glm::vec3 spec, std::shared_ptr<Geometry> pgeometry, std::shared_ptr<Material> pmaterial)
    : name_in_shader_(name), ambient_(ambient), diffuse_(diff), specular_(spec), Object(pgeometry, pmaterial), lightColor_(1.0f), type_(PointLight) {
}

 void Light::setName(const std::string& name) {
    name_in_shader_ = name;
}

 void Light::setAsPointLight(glm::vec3 pos, glm::vec3 ambient, glm::vec3 diff, glm::vec3 spec, float constant, float linear, float quadratic)
 {
     setPosition(pos);
     ambient_ = ambient;
     diffuse_ = diff;
     specular_ = spec;
     constant_ = constant;
     linear_ = linear;
     quadratic_ = quadratic;
     type_ = PointLight;
 }

 void Light::setAsDirectionalLight(glm::vec3 dir, glm::vec3 ambient, glm::vec3 diff, glm::vec3 spec)
 {
     direction_ = dir;
     ambient_ = ambient;
     diffuse_ = diff;
     specular_ = spec;
     type_ = DirectionalLight;
 }

 void Light::setAsSpotLight(glm::vec3 pos, glm::vec3 dir, float cutoff, float outerCutoff)
 {
     setPosition(pos);
     direction_ = dir;
     cutoff_ = cutoff;
     outerCutoff_ = outerCutoff;
     type_ = SpotLight;
 }

 void Light::setToShader(std::shared_ptr<Shader> pshader, int index) const {
     //std::string prefix = name_in_shader_;
     //if (index != -1) {
     //    prefix += "[" + std::to_string(index) + "]";
     //}
     //if (type_ == PointLight) {
     //    pshader->setVec3(prefix + ".position", getPosition());
     //    pshader->setVec3(prefix + ".ambient", ambient_);
     //    pshader->setVec3(prefix + ".diffuse", diffuse_);
     //    pshader->setVec3(prefix + ".specular", specular_);
     //    pshader->setFloat(prefix + ".constant", constant_);
     //    pshader->setFloat(prefix + ".linear", linear_);
     //    pshader->setFloat(prefix + ".quadratic", quadratic_);
     //}
     //else if (type_ == DirectionalLight) {
     //    pshader->setVec3(prefix + ".direction", direction_);
     //    pshader->setVec3(prefix + ".ambient", ambient_);
     //    pshader->setVec3(prefix + ".diffuse", diffuse_);
     //    pshader->setVec3(prefix + ".specular", specular_);
     //}
     //else if (type_ == SpotLight) {
     //    pshader->setVec3(prefix + ".position", getPosition());
     //    pshader->setVec3(prefix + ".direction", direction_);
     //    pshader->setFloat(prefix + ".cutoff", cutoff_);
     //    pshader->setFloat(prefix + ".outerCutoff", outerCutoff_);
     //}
    // 三个项其实已经定义过光源的颜色了
    //pshader->setVec3("lightColor")
}


 void Light::setColor(const glm::vec3& color)
 {
     lightColor_ = color;
 }

 Light::LightType Light::getType() const
 {
     return type_;
 }

 std::string Light::getName() const
 {
     return name_in_shader_;
 }

 void Light::updateFrame(Renderer& renderer) {
    Object::updateFrame(renderer);
    auto pmaterial = Object::getpMaterial();
    if (pmaterial) {
        //pmaterial->setUniform("")
    }

    if (type_ == SpotLight) {
        direction_ = renderer.getCamera().forward();
        setPosition(renderer.getCamera().position());
    }
}

 /*PointLight::PointLight(std::shared_ptr<Geometry> pgeometry, std::shared_ptr<Material> pmaterial)
     : Light(pgeometry, pmaterial)
 {
 }

 void PointLight::setToShader(std::shared_ptr<Shader> pshader, int index) const
 {
     std::string prefix = getName();
     if (index != -1) {
         prefix += "[" + std::to_string(getIndex()) + "]";
     }
     pshader->setVec3(prefix + ".position", getPosition());
     pshader->setVec3(prefix + ".ambient", ambient_);
     pshader->setVec3(prefix + ".diffuse", diffuse_);
     pshader->setVec3(prefix + ".specular", specular_);
     pshader->setFloat(prefix + ".constant", constant_);
     pshader->setFloat(prefix + ".linear", linear_);
     pshader->setFloat(prefix + ".quadratic", quadratic_);
 }

 int PointLight::getIndex() const
 {
     return index++;
 }

 void PointLight::resetIndex()
 {
     index = 0;
 }

 void PointLight::updateFrame(const Camera& camera)
 {
     Object::updateFrame(camera);
 }

 void PointLight::setAttribute(glm::vec3 pos, glm::vec3 ambient, glm::vec3 diff, glm::vec3 spec, float constant, float linear, float quadratic)
 {
     setPosition(pos);
     ambient_ = ambient;
     diffuse_ = diff;
     specular_ = spec;
     constant_ = constant;
     linear_ = linear;
     quadratic_ = quadratic;
 }

 void DirectionalLight::setToShader(std::shared_ptr<Shader> pshader, int index) const
 {
     std::string prefix = getName();
     if (index != -1) {
         prefix += "[" + std::to_string(getIndex()) + "]";
     }
     pshader->setVec3(prefix + ".position", getPosition());
     pshader->setVec3(prefix + ".ambient", ambient_);
     pshader->setVec3(prefix + ".diffuse", diffuse_);
     pshader->setVec3(prefix + ".specular", specular_);
 }

 int DirectionalLight::getIndex() const
 {
     return index++;
 }


 void DirectionalLight::resetIndex()
 {
     index = 0;
 }

 void DirectionalLight::updateFrame(const Camera& camera)
 {
 }

 void DirectionalLight::setAttribute(glm::vec3 dir, glm::vec3 ambient, glm::vec3 diff, glm::vec3 spec)
 {
     direction_ = dir;
     ambient_ = ambient;
     diffuse_ = diff;
     specular_ = spec;
 }

 void SpotLight::setToShader(std::shared_ptr<Shader> pshader, int index) const
 {
     std::string prefix = getName();
     if (index != -1) {
         prefix += "[" + std::to_string(getIndex()) + "]";
     }
     pshader->setVec3(prefix + ".position", getPosition());
     pshader->setVec3(prefix + ".direction", direction_);
     pshader->setFloat(prefix + ".cutoff", cutoff_);
     pshader->setFloat(prefix + ".outerCutoff", outerCutoff_);
 }

 int SpotLight::getIndex() const
 {
     return index++;
 }

 void SpotLight::resetIndex()
 {
     index = 0;
 }

 void SpotLight::updateFrame(const Camera& camera)
 {
     direction_ = camera.forward();
     setPosition(camera.position());
 }

 void SpotLight::setAttribute(glm::vec3 pos, glm::vec3 dir, float cutoff, float outerCutoff)
 {
     setPosition(pos);
     direction_ = dir;
     cutoff_ = cutoff;
     outerCutoff_ = outerCutoff;
 }

 int PointLight::index = 0;
 int DirectionalLight::index = 0;
 int SpotLight::index = 0;*/
