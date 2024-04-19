#pragma once
#include <memory>
#include <string>

#include <unordered_map>
#include "Base/GLMInc.h"

enum AlphaMode {
	Alpha_Opaque,
	Alpha_Blend,
};

enum ShadingMode {
	Shading_Unknown = 0,
	Shading_BaseColor,
	Shading_BlinnPhong,
	Shading_PBR,
	Shading_Skybox,
	Shading_IBL_Irradiance,
	Shading_IBL_Prefilter,
	Shading_FXAA,
};

enum ShaderPass {
	Shader_Shadow_Pass,
	Shader_Plain_Pass,
	Shader_ToScreen_Pass
};

class Texture;
class Shader;
class Light;
class Uniform;
class Renderer;

// 应该区分元信息和运行时资源
// 比如shader是运行时资源，shadingMode是元信息，但是shader需要根据shadingMode的改变重新编译加载
// shader还是现在在外部手动加载的，所以需要引入shadingMode
// shadowMapTexture 就在Material setup时加载
class Material {
protected:
	std::string shaderStructName_;
	std::shared_ptr<Shader> pshader_;
	ShadingMode shadingMode_;
	std::unordered_map<std::string, std::shared_ptr<Uniform>> uniforms_; // name -> Uniform
	std::unordered_map<int, std::shared_ptr<Texture>> textures_; // TextureType(int) -> texture
	std::unordered_map<ShaderPass, std::shared_ptr<Shader>> shaders_;
	std::vector<std::string> defines_;
	bool shaderReady_ = false; // shader setupPipeline or not

	//std::unordered_map<std::string, 

public:
	Material();
	Material(std::string name, std::shared_ptr<Shader> pshader);

	static const char* shadingModelStr(ShadingMode model);

	//void loadShader(const std::string& vertexPath, const std::string& fragmentPath);

	void setpShader(std::shared_ptr<Shader> pshader);

	void resetShader();

	std::shared_ptr<Shader> getpshader();

	void setShader(ShaderPass pass, std::shared_ptr<Shader> pshader);

	std::shared_ptr<Shader> getShader(ShaderPass pass);

	std::unordered_map<ShaderPass, std::shared_ptr<Shader>>& getShaders();

	bool shaderReady();

	void setShaderReady(bool setValue);

	void setUniform(const std::string& name, std::shared_ptr<Uniform> uniform);

	std::shared_ptr<Uniform> getUniform(const std::string& name);

	std::unordered_map<std::string, std::shared_ptr<Uniform>>& getUniforms();

	void setTexture(int textureType, std::shared_ptr<Texture> ptex);

	void setTexture(std::shared_ptr<Texture> ptex);

	std::unordered_map<int, std::shared_ptr<Texture>>& getTextures();

	ShadingMode shadingMode();

	void setShadingMode(ShadingMode mode);

	void addDefine(const std::string& define);

	std::vector<std::string>& getDefines();

	std::string getName();

	void setLight(std::shared_ptr<Light> light) const;

	void setLightArray(const std::vector<std::shared_ptr<Light>>& lightArray);

	void use();

	void use(ShaderPass pass);

	virtual ~Material() {}

	virtual void setName(const std::string& name);

	virtual void init();

	virtual void setupPipeline(Renderer& renderer) = 0;
};

class PhongMaterial : public Material{
	glm::vec3 ambient_;
	glm::vec3 diffuse_;
	glm::vec3 specular_;
	float shininess_;
public:

	PhongMaterial();
	PhongMaterial(std::string name, glm::vec3 ambient, glm::vec3 diffuse, glm::vec3 specular, float shininess);

	virtual void init() override;
	void print();

	virtual void setName(const std::string name);
};

class StandardMaterial : public Material {
public:
	StandardMaterial();
	StandardMaterial(const std::string& name);

	virtual void setupPipeline(Renderer& renderer) override;
	//virtual void use(Renderer& renderer) override;
};