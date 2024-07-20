#include "Material.h"
#include "Texture.h"
#include "Shader.h"
#include "Light.h"
#include "Base/GLMInc.h"
#include "Utils/utils.h"
#include "Renderer.h"

 Material::Material() :pshader_(nullptr), shaderStructName_("material") {

 }

 Material::Material(std::string name, std::shared_ptr<Shader> pshader) : Material() {
	shaderStructName_ = name;
	pshader_ = pshader;
}

// void Material::loadShader(const std::string& vertexPath, const std::string& fragmentPath) {
//	pshader_ = Shader::loadShader(vertexPath, fragmentPath);
//}

 void Material::setpShader(std::shared_ptr<Shader> pshader) {
	 //resetShader();
	pshader_ = pshader;
	setShaderReady(false);
	//init();
}

 void Material::resetShader()
 {

 }

 std::shared_ptr<Shader> Material::getpshader() {
	return pshader_;
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

 void Material::setUniform(const std::string& name, std::shared_ptr<Uniform> uniform)
 {
	 uniforms_[name] = std::move(uniform);
 }

 std::shared_ptr<Uniform> Material::getUniform(const std::string& name)
 {
	 return uniforms_[name];
 }

 void Material::addDefine(const std::string& define)
 {
	 defines_.push_back(define);
 }

 std::vector<std::string>& Material::getDefines()
 {
	 return defines_;
 }

 std::unordered_map<std::string, std::shared_ptr<Uniform>>& Material::getUniforms()
 {
	 // TODO: 在此处插入 return 语句
	 return uniforms_;
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

 ShadingMode Material::shadingMode() {
	 return shadingMode_;
 }

 void Material::setShadingMode(ShadingMode mode) {
 	setShaderReady(shaderReady() && (mode == shadingMode_));
	 shadingMode_ = mode;
 }

 std::string Material::getName() {
	return shaderStructName_;
}

 void Material::setLight(std::shared_ptr<ULight> light) const {
	light->setToShader(pshader_);
}

 void Material::setLightArray(const std::vector<std::shared_ptr<ULight>>& lightArray)
 {
	 // // 假设shader中的light全部以数组形式存储
	 // int pointIdx = 0, dirIdx = 0, spotIdx = 0;
	 // for (const auto& light : lightArray) {
		//  if (light->getType() == Light::PointLight) {
		// 	 light->setToShader(pshader_, pointIdx++);
		//  }
		//  else if (light->getType() == Light::DirectionalLight) {
		// 	 light->setToShader(pshader_, dirIdx++);
		//  }
		//  else if (light->getType() == Light::SpotLight) {
		// 	 light->setToShader(pshader_, spotIdx++);
		//  }
		//  //light->setToShader(pshader_, 0);
	 // }
 }

 void Material::use()
 {
 }

 void Material::use(ShaderPass pass)
 {
	 if (!shaders_.contains(pass)) {
		 LOGE("Rendering pass missing corresponding shader!");
	 }
	 else {
		 shaders_[pass]->use();
		 for (auto& [_, puniform] : uniforms_) {
			 shaders_[pass]->bindUniform(*puniform);
		 }
	 }
 }

 void Material::setName(const std::string& name) {
	shaderStructName_ = name;
}

 void Material::init()
 {
 }
