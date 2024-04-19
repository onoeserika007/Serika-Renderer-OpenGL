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

 void Material::setUniform(const std::string& name, std::shared_ptr<Uniform> uniform)
 {
	 uniforms_[name] = uniform;
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

void Material::setTexture(int textureType, std::shared_ptr<Texture> ptex) {
	 textures_[textureType] = ptex;
 }

void Material::setTexture(std::shared_ptr<Texture> ptex)
{
	setTexture(ptex->getTextureInfo().type, ptex);
}

 std::unordered_map<int, std::shared_ptr<Texture>>& Material::getTextures()
 {
	 // TODO: 在此处插入 return 语句
	 return textures_;
 }

 ShadingMode Material::shadingMode() {
	 return shadingMode_;
 }

 void Material::setShadingMode(ShadingMode mode) {
	 shadingMode_ = mode;
 }


 std::string Material::getName() {
	return shaderStructName_;
}

 void Material::setLight(std::shared_ptr<Light> light) const {
	light->setToShader(pshader_);
}

 void Material::setLightArray(const std::vector<std::shared_ptr<Light>>& lightArray)
 {
	 // 假设shader中的light全部以数组形式存储
	 int pointIdx = 0, dirIdx = 0, spotIdx = 0;
	 for (const auto& light : lightArray) {
		 if (light->getType() == Light::PointLight) {
			 light->setToShader(pshader_, pointIdx++);
		 }
		 else if (light->getType() == Light::DirectionalLight) {
			 light->setToShader(pshader_, dirIdx++);
		 }
		 else if (light->getType() == Light::SpotLight) {
			 light->setToShader(pshader_, spotIdx++);
		 }
		 //light->setToShader(pshader_, 0);
	 }
 }

 void Material::use()
 {
 }

 void Material::use(ShaderPass pass)
 {
	 if (!shaders_.count(pass)) {
		 LOGE("Rendering pass missing corresponding shader!");
	 }
	 else {
		 shaders_[pass]->use();
	 }
 }

 void Material::setName(const std::string& name) {
	shaderStructName_ = name;
}

 void Material::init()
 {
 }
