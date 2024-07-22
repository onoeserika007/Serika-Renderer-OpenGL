#include "Material.h"
#include "Texture.h"
#include "Shader.h"
#include "Light.h"
#include "Base/GLMInc.h"
#include "Utils/utils.h"
#include "Renderer.h"

 Material::Material() {

 }


 void Material::setShader(ShaderPass pass, std::shared_ptr<Shader> pshader)
 {
	 shaders_[pass] = pshader;
	 shaderReady_ = false;
 }

 std::shared_ptr<Shader> Material::getShader(ShaderPass pass)
 {
	 if (shaders_.count(pass)) {
		 return shaders_[pass];
	 }
	 return std::shared_ptr<Shader>();
 }

 std::unordered_map<ShaderPass, std::shared_ptr<Shader>>& Material::getShaders()
 {
	 // TODO: 在此处插入 return 语句
	 return shaders_;
 }

 bool Material::shaderReady()
 {
	 return shaderReady_;
 }

 void Material::setShaderReady(bool setValue)
 {
	 shaderReady_ = setValue;
 }

 bool Material::texturesReady()
 {
	 return texturesReady_;;
 }

 void Material::setTexturesReady(bool ready)
 {
	 texturesReady_ = ready;
 }

void Material::setUniformSampler(const std::string &name, const std::shared_ptr<UniformSampler> &uniform) {
 	getShaderResources()->samplers[name] = uniform;
}

void Material::setUniformBlock(const std::string &name, const std::shared_ptr<UniformBlock> &uniform) {
 	getShaderResources()->blocks[name] = uniform;
}

std::shared_ptr<ShaderResources> Material::getShaderResources() const {
 	if (!shaderResources_) {
 		shaderResources_ = std::make_shared<ShaderResources>();
 	}
 	return shaderResources_;
}

void Material::setShaderResources(const std::shared_ptr<ShaderResources> &shader_resources) {
 	shaderResources_ = shader_resources;
}


 void Material::addDefine(const std::string& define)
 {
	 defines_.push_back(define);
 }

 std::vector<std::string>& Material::getDefines()
 {
	 return defines_;
 }

void Material::setTextureData(int textureType, TextureData ptex) {
	 textureData_[textureType] = ptex;
 }

 std::unordered_map<int, TextureData>& Material::getTextureData()
 {
	 // TODO: 在此处插入 return 语句
	 return textureData_;
 }

 void Material::addTexture(std::shared_ptr<Texture> pTex)
 {
	 texturesRuntime_.push_back(pTex);
 }

 void Material::clearTextures()
 {
	 texturesRuntime_.clear();
 }

 EShadingMode Material::shadingMode() {
	 return shadingMode_;
 }

 void Material::setShadingMode(EShadingMode mode) {
 	setShaderReady(shaderReady() && (mode == shadingMode_));
	 shadingMode_ = mode;
 }

 void Material::use(ShaderPass pass)
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

void Material::setupPipeline(Renderer &renderer) {
 	renderer.setupMaterial(*this);
}
