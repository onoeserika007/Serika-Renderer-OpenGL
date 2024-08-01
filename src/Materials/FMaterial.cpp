#include "Material/FMaterial.h"
#include "Material/Texture.h"
#include "Material/Shader.h"
#include "Base/Globals.h"
#include "Utils/UniversalUtils.h"
#include "Renderer.h"
#include <cmath>

#include "app.h"
#include "Base/ResourceLoader.h"

FMaterial::FMaterial() {
}

void FMaterial::setShader(ShaderPass pass, std::shared_ptr<Shader> pshader) {
    shaders_[pass] = pshader;
    shaderReady_ = false;
}

std::shared_ptr<Shader> FMaterial::getShader(ShaderPass pass) {
    if (shaders_.count(pass)) {
        return shaders_[pass];
    }
    return std::shared_ptr<Shader>();
}

std::unordered_map<ShaderPass, std::shared_ptr<Shader> > &FMaterial::getShaders() {
    // TODO: 在此处插入 return 语句
    return shaders_;
}

FMaterial::FMaterial(const FMaterial &other)
    : materialInfoUniformBlock_(), shaderResources_(), shaders_(), uuid_()  {
    shadingModel_ = other.shadingModel_;
    shaderReady_ = false;
    material_info_ = other.material_info_;
    materialObject_ = other.materialObject_;
}

bool FMaterial::shaderReady() const {
    return shaderReady_;
}

void FMaterial::setShaderReady(bool setValue) {
    shaderReady_ = setValue;
}

bool FMaterial::texturesReady() const {
    if (auto&& matobj = getMaterialObject()) {
        return matobj->texturesReady_;
    }
    return false;
}

void FMaterial::setTexturesReady(bool ready) {
    if (auto&& matobj = getMaterialObject()) {
        matobj->texturesReady_ = ready;
    }
}

void FMaterial::setUniformSampler(const std::string &name, const std::shared_ptr<UniformSampler> &uniform) {
    getShaderResources()->samplers[name] = uniform;
}

void FMaterial::setUniformBlock(const std::string &name, const std::shared_ptr<UniformBlock> &uniform) {
    getShaderResources()->blocks[name] = uniform;
}

std::shared_ptr<ShaderResources> FMaterial::getShaderResources() const {
    if (!shaderResources_) {
        shaderResources_ = std::make_shared<ShaderResources>();
    }
    return shaderResources_;
}

std::shared_ptr<UniformSampler> FMaterial::getUniformSampler(const std::string &name) const {
    if (shaderResources_) {
        return shaderResources_->samplers[name];
    }
    return {};
}

void FMaterial::setShaderResources(const std::shared_ptr<ShaderResources> &shader_resources) {
    shaderResources_ = shader_resources;
}

void FMaterial::addDefine(const std::string &define) {
    if (auto&& matobj = getMaterialObject()) {
        matobj->defines_.push_back(define);
    }
}

std::vector<std::string> &FMaterial::getDefines() {
    static std::vector<std::string> empty;
    if (auto&& matobj = getMaterialObject()) {
        return matobj->defines_;
    }
    return empty;
}

void FMaterial::setTextureData(const TextureType textureType, const TextureData &texData) {
    if (auto &&matobj = getMaterialObject()) {
        matobj->textureData_[textureType] = texData;
    }
}

void FMaterial::setTextureData(TextureType textureType, const std::string &texPath) {
    auto&& loader = ResourceLoader::getInstance();
    auto&& typeDefine = Texture::samplerDefine(textureType);
    auto bufferData = loader.loadTexture(texPath);
    TextureData texData;
    texData.unitDataArray = { bufferData };
    texData.path = texPath;
    texData.loadedTextureType = textureType;

    this->setTextureData(textureType, texData);
    this->addDefine(typeDefine);
}

std::unordered_map<int, TextureData> &FMaterial::getTextureData() {
    static std::unordered_map<int, TextureData> empty;
    if (auto &&matobj = getMaterialObject()) {
        return matobj->textureData_;
    }
    return empty;
}

void FMaterial::setTexture_runtime(TextureType texType, const std::shared_ptr<Texture> &pTex) {
    if (auto &&matobj = getMaterialObject()) {
        matobj->texturesRuntime_[texType] = pTex;
    }
}

void FMaterial::clearTextures_runtime() {
    if (auto &&matobj = getMaterialObject()) {
        matobj->texturesRuntime_.clear();
    }
}

EShadingModel FMaterial::shadingMode() {
    return shadingModel_;
}

void FMaterial::setShadingMode(EShadingModel mode) {
    setShaderReady(shaderReady() && (mode == shadingModel_));
    shadingModel_ = mode;
}
;;

void FMaterial::use(ShaderPass pass) {
    if (!shaders_.contains(pass)) {
        LOGE("Rendering pass missing corresponding shader - %d", pass);
        throw std::exception();;
    } else {
        if (!materialInfoUniformBlock_) {
            auto&& app = App::getInstance();
            if (auto&& renderer = app.getRenderer()) {
                materialInfoUniformBlock_ = renderer->createUniformBlock("MaterialInfo", sizeof(MaterialInfoBlock));
            }
        }
        if (materialInfoUniformBlock_) {
            materialInfoUniformBlock_->setData(&material_info_, sizeof(MaterialInfoBlock));
            // 注意，当shader在material管制下时，是不持有resource的，所以应该向material里添加
            // shaders_[pass]->setUniformBlock(materialInfoUniformBlock_->name(), materialInfoUniformBlock_);
            this->setUniformBlock(materialInfoUniformBlock_->name(), materialInfoUniformBlock_);
        }
        // these will be called each rendering loop
        shaders_[pass]->bindResources(getShaderResources());
    }
}

std::shared_ptr<MaterialResource> FMaterial::getMaterialObject() const {
    if (!materialObject_) {
        materialObject_ = std::make_shared<MaterialResource>();
    }
    return materialObject_;
}

void FMaterial::setupPipeline(Renderer &renderer) {
    renderer.setupMaterial(*this);
}

bool FMaterial::hasEmission() const {
    if (auto &&matobj = getMaterialObject()) {
        return matobj->textureData_.contains(TEXTURE_TYPE_EMISSIVE) || matobj->textureData_.
               contains(TEXTURE_TYPE_EMISSION_COLOR) || glm::length(material_info_.uEmission) > M_EPSILON;
    }
    return {};
}

glm::vec3 FMaterial::getEmission() const {
    if (!hasEmission()) return {};
    return material_info_.uEmission;
}

glm::vec3 FMaterial::getEmission(float u, float v) const {
    if (!hasEmission()) return {};
    if (auto &&matobj = getMaterialObject()) {
        if (matobj->textureData_.contains(TEXTURE_TYPE_EMISSIVE)) {
            auto &&ret = sample2D<RGBA>(u, v, TEXTURE_TYPE_EMISSIVE, FilterMode::Filter_NEAREST);
            return {ret.x, ret.y, ret.z};
        }
        if (matobj->textureData_.contains(TEXTURE_TYPE_EMISSION_COLOR)) {
            auto &&ret = sample2D<RGBA>(u, v, TEXTURE_TYPE_EMISSION_COLOR, FilterMode::Filter_NEAREST);
            return {ret.x, ret.y, ret.z};
        }
    }
    return {};
}

glm::vec3 FMaterial::getDiffuse() const { return material_info_.uAlbedo; }

glm::vec3 FMaterial::getDiffuse(float u, float v) const {
    if (auto &&matobj = getMaterialObject()) {
        if (matobj->textureData_.contains(TEXTURE_TYPE_DIFFUSE)) {
            auto &&ret = sample2D<RGBA>(u, v, TEXTURE_TYPE_DIFFUSE, FilterMode::Filter_NEAREST);
            return {ret.x, ret.y, ret.z};
        }
    }

    return {};
}

glm::vec3 FMaterial::getSpecular(float u, float v) const {
    if (auto &&matobj = getMaterialObject()) {
        if (matobj->textureData_.contains(TEXTURE_TYPE_SPECULAR)) {
            auto &&ret = sample2D<RGBA>(u, v, TEXTURE_TYPE_SPECULAR, FilterMode::Filter_NEAREST);
            return {ret.x, ret.y, ret.z};
        }
    }

    return {};
}

glm::vec3 FMaterial::evalRadiance(const glm::vec3 &wi, const glm::vec3 &wo, const glm::vec3 &N, const float u,
                                  const float v) const {
    switch (shadingModel_) {
        case Shading_BaseColor:
        case Shading_Lambertian: {
            // calculate the contribution of diffuse model
            return dot(N, wo) > 0.f ? material_info_.uAlbedo / M_PI_DEFINED : glm::vec3(0.f);
        }
        case Shading_BlinnPhong: {
            // calculate the contribution of diffuse model
            float cosalpha = dot(N, wo) > 0.f;
            if (cosalpha > 0.f) {
                glm::vec3 f_r{};
                glm::vec3 V = -wi;
                glm::vec3 L = wo;
                glm::vec3 H = glm::normalize(V + L);
                f_r += material_info_.uAlbedo / M_PI_DEFINED;
                f_r += getSpecular(u, v) * (material_info_.uSpecular + 2) / (2.f * M_PI_DEFINED) *
                        float(pow(dot(H, N), material_info_.uSpecular)); // here disney specular represents specularExponent
                return f_r;
            } else {
                return {};
            }
        }
        case Shading_PBR: {
            float cosalpha = glm::dot(N, wo);
            if (cosalpha > M_EPSILON) {
                float roughness = 0.02f;
                glm::vec3 V = -wi;
                glm::vec3 L = wo;
                glm::vec3 H = glm::normalize(V + L);
                float D = MathUtils::Trowbridge_Reitz_GGX_D(N, H, roughness);
                float G = MathUtils::Schick_GGXSmith_G(N, V, L, roughness);
                float F = MathUtils::Schick_Fresnel_F(cosalpha, 0.50f);
                float diffsue = glm::dot(N, V) + 0.5f;
                float divide = 1.0f / (4.f * std::max(glm::dot(N, L), M_EPSILON) * std::max(glm::dot(N, V), M_EPSILON));
                float Specular = D * F * G * divide;
                //std::cout << D <<"  " << G <<"  "<< F<<"  "<< divide<<"  "<< Specular<<std::endl;
                return diffsue * material_info_.uAlbedo / M_PI_DEFINED + getSpecular(u, v) * Specular;
            } else return {};
        }
        default: {
            return {};
        }
    }
}

std::shared_ptr<FMaterial> FMaterial::Clone() const {
    return std::make_shared<FMaterial>(*this);
}
