#include "Material/FMaterial.h"
#include "Material/Texture.h"
#include "Material/Shader.h"
#include "Base/Globals.h"
#include "Utils/UniversalUtils.h"
#include "Renderer.h"

 FMaterial::FMaterial() {

 }

 void FMaterial::setShader(ShaderPass pass, std::shared_ptr<Shader> pshader)
 {
	 shaders_[pass] = pshader;
	 shaderReady_ = false;
 }

 std::shared_ptr<Shader> FMaterial::getShader(ShaderPass pass)
 {
	 if (shaders_.count(pass)) {
		 return shaders_[pass];
	 }
	 return std::shared_ptr<Shader>();
 }

 std::unordered_map<ShaderPass, std::shared_ptr<Shader>>& FMaterial::getShaders()
 {
	 // TODO: 在此处插入 return 语句
	 return shaders_;
 }

 bool FMaterial::shaderReady() const {
	 return shaderReady_;
 }

 void FMaterial::setShaderReady(bool setValue)
 {
	 shaderReady_ = setValue;
 }

 bool FMaterial::texturesReady() const {
	 return texturesReady_;;
 }

 void FMaterial::setTexturesReady(bool ready)
 {
	 texturesReady_ = ready;
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
}

void FMaterial::setShaderResources(const std::shared_ptr<ShaderResources> &shader_resources) {
 	shaderResources_ = shader_resources;
}

 void FMaterial::addDefine(const std::string& define)
 {
	 defines_.push_back(define);
 }

std::vector<std::string>& FMaterial::getDefines()
 {
	 return defines_;
 }

void FMaterial::setTextureData(const TextureType textureType, const TextureData &texData) {
	 textureData_[textureType] = texData;
 }

 std::unordered_map<int, TextureData>& FMaterial::getTextureData()
 {
	 // TODO: 在此处插入 return 语句
	 return textureData_;
 }

 void FMaterial::setTexture_runtime(TextureType texType, const std::shared_ptr<Texture> &pTex)
 {
	 texturesRuntime_[texType] = pTex;
 }

 void FMaterial::clearTextures_runtime()
 {
	 texturesRuntime_.clear();
 }

 EShadingModel FMaterial::shadingMode() {
	 return shadingMode_;
 }

 void FMaterial::setShadingMode(EShadingModel mode) {
 	setShaderReady(shaderReady() && (mode == shadingMode_));
	 shadingMode_ = mode;
 }
;;

 void FMaterial::use(ShaderPass pass)
 {
	 if (!shaders_.contains(pass)) {
		 LOGE("Rendering pass missing corresponding shader - %d", pass);
	 	throw std::exception("missing corresponding shader");
	 }
	 else {
	 	// these will be called each rendering loop
	 	shaders_[pass]->bindResources(getShaderResources());
	 }
 }

void FMaterial::setupPipeline(Renderer &renderer) {
 	renderer.setupMaterial(*this);
}

bool FMaterial::hasEmission() const {
 	return textureData_.contains(TEXTURE_TYPE_EMISSIVE) || textureData_.contains(TEXTURE_TYPE_EMISSION_COLOR) || glm::length(emission) > M_EPSILON;
}

glm::vec3 FMaterial::getEmission() const {
 	if (!hasEmission()) return {};
 	return emission;
}

glm::vec3 FMaterial::getEmission(float u, float v) const {
 	if (!hasEmission()) return {};
 	if (textureData_.contains(TEXTURE_TYPE_EMISSIVE)) {
 		auto&& ret = sample2D<RGBA>(u, v, TEXTURE_TYPE_EMISSIVE, FilterMode::Filter_NEAREST);
 		return {ret.x, ret.y, ret.z};
 	}
 	if (textureData_.contains(TEXTURE_TYPE_EMISSION_COLOR)) {
 		auto&& ret = sample2D<RGBA>(u, v, TEXTURE_TYPE_EMISSION_COLOR, FilterMode::Filter_NEAREST);
 		return {ret.x, ret.y, ret.z};
 	}
 	return {};
}
