#pragma once
#include <memory>
#include <string>
#include "Base/GLMInc.h"
#include "Object.h"


// 派生类需要基类的定义，因此基类就要避免包括派生类
// 用前向声明替换直接include，在cpp文件中再include
class Object;
class Shader;
class Material;
class Geometry;
class Renderer;

const unsigned MAX_LIGHT_NUMS = 10;

struct LightData {
};

struct PointLightData: LightData {
    glm::vec3 position;

    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;

    float constant;
    float linear;
    float quadratic;
};

struct DirectionalLightData: LightData {
    glm::vec3 direction;

    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
};

struct SpotLightData : LightData {
    glm::vec3 position;
    glm::vec3 direction;
    float cutoff;
    float outerCutoff;
};

struct PointLightUniformBlock {
    PointLightData PointLights[MAX_LIGHT_NUMS];
    int numPointLight;
};

struct DirectionalLightUniformBlock {
    DirectionalLightData DirectionalLights[MAX_LIGHT_NUMS];
    int numDirectionalLight;
};

struct SpotLightUniformBlock {
    SpotLightData SpotLights[MAX_LIGHT_NUMS];
    int numSpotLight;
};

class Light: public Object{
public:
    enum LightType {
        PointLight,
        DirectionalLight,
        SpotLight
    };
    Light();
    Light(const std::string& name, glm::vec3 ambient, glm::vec3 diff, glm::vec3 spec);
    Light(std::shared_ptr<Geometry> pgeometry, std::shared_ptr<Material> pmaterial);
    Light(const std::string& name, std::shared_ptr<Geometry> pgeometry, std::shared_ptr<Material> pmaterial);
    Light(const std::string& name, glm::vec3 ambient, glm::vec3 diff, glm::vec3 spec, std::shared_ptr<Geometry> pgeometry, std::shared_ptr<Material> pmaterial);
    void setName(const std::string& name);
    void setAsPointLight(glm::vec3 pos, glm::vec3 ambient, glm::vec3 diff, glm::vec3 spec, float constant, float linear, float quadratic);
    void setAsDirectionalLight(glm::vec3 dir, glm::vec3 ambient, glm::vec3 diff, glm::vec3 spec);
    void setAsSpotLight(glm::vec3 pos, glm::vec3 dir, float cutoff, float outerCutoff);
    void setToShader(std::shared_ptr<Shader> pshader, int index = -1) const;
    void setColor(const glm::vec3& color);
    LightType getType() const;
    std::string getName() const;

    virtual void updateFrame(Renderer& renderer) override;

private:
    LightType type_;
    std::string name_in_shader_;
    std::shared_ptr<Camera> pcamera_;

    glm::vec3 ambient_;
    glm::vec3 diffuse_;
    glm::vec3 specular_;
    glm::vec3 lightColor_;
    glm::vec3 direction_;

    float constant_;
    float linear_;
    float quadratic_;
    float cutoff_;
    float outerCutoff_;
};

// virtual function version
// 虚函数写出来也太尼玛丑了，简单的才是好的
// ----------------------------------------------------------------------
//class PointLight :public Light {
//    static int index;
//    glm::vec3 ambient_;
//    glm::vec3 diffuse_;
//    glm::vec3 specular_;
//
//    float constant_;
//    float linear_;
//    float quadratic_;
//public:
//    PointLight(std::shared_ptr<Geometry> pgeometry, std::shared_ptr<Material> pmaterial);
//    virtual void setToShader(std::shared_ptr<Shader> pshader, int index = -1) const;
//    virtual int getIndex()const;
//    static void resetIndex();
//    virtual void updateFrame(const Camera& camera) override;
//    void setAttribute(glm::vec3 pos, glm::vec3 ambient, glm::vec3 diff, glm::vec3 spec, float constant, float linear, float quadratic);
//};
//
//class DirectionalLight :public Light {
//    static int index;
//    glm::vec3 ambient_;
//    glm::vec3 diffuse_;
//    glm::vec3 specular_;
//    glm::vec3 direction_;
//public:
//    virtual void setToShader(std::shared_ptr<Shader> pshader, int index = -1) const;
//    virtual int getIndex() const ;
//    static void resetIndex();
//    virtual void updateFrame(const Camera& camera) override;
//    void setAttribute(glm::vec3 dir, glm::vec3 ambient, glm::vec3 diff, glm::vec3 spec);
//};
//
//class SpotLight :public Light {
//    static int index;
//    glm::vec3 direction_;
//    float cutoff_;
//    float outerCutoff_;
//public:
//    virtual void setToShader(std::shared_ptr<Shader> pshader, int index = -1) const;
//    virtual int getIndex() const;
//    static void resetIndex();
//    virtual void updateFrame(const Camera& camera) override;
//    void setAttribute(glm::vec3 pos, glm::vec3 dir, float cutoff, float outerCutoff);
//};
