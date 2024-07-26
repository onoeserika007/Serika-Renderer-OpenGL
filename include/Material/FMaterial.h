#pragma once
#include <memory>
#include <string>
#include <vector>

#include <unordered_map>
#include "Base/Globals.h"
#include "Texture.h"
#include "Uniform.h"

class ShaderResources;

enum AlphaMode {
	Alpha_Opaque,
	Alpha_Blend,
};


class Shader;
class ULight;
class Uniform;
class Renderer;

// 应该区分元信息和运行时资源
// 比如shader是运行时资源，shadingMode是元信息，但是shader需要根据shadingMode的改变重新编译加载
// shader还是现在在外部手动加载的，所以需要引入shadingMode
// shadowMapTexture 就在Material setup时加载
class FMaterial {
public:
	FMaterial();
	FMaterial(std::string name, std::shared_ptr<Shader> pshader);

	// getters
	bool shaderReady() const;
	bool texturesReady() const;
	bool isPipelineReady() const { return shaderReady() && texturesReady(); }
	std::shared_ptr<Shader> getShader(ShaderPass pass);
	std::unordered_map<ShaderPass, std::shared_ptr<Shader>>& getShaders();
	std::shared_ptr<ShaderResources> getShaderResources() const;
	std::shared_ptr<UniformSampler> getUniformSampler(const std::string& name) const;
	std::unordered_map<int, TextureData>& getTextureData();
	EShadingModel shadingMode();
	std::vector<std::string>& getDefines();

	// setters and adders
	void setUniformSampler(const std::string& name, const std::shared_ptr<UniformSampler>& uniform);
	void setUniformBlock(const std::string& name, const std::shared_ptr<UniformBlock> &uniform);
	void setShaderResources(const std::shared_ptr<ShaderResources>& shader_resources);
	void setShader(ShaderPass pass, std::shared_ptr<Shader> pshader);
	void setShaderReady(bool setValue);
	void setTexturesReady(bool ready);

	// Texture Data
	void setTextureData(TextureType textureType, const TextureData &texData);
	void setTexture_runtime(TextureType texType, const std::shared_ptr<Texture> &pTex);
	void clearTextures_runtime();
	void setShadingMode(EShadingModel mode);
	void addDefine(const std::string& define);

	// sample
	template<typename T>
	T sample2D(float u, float v, TextureType texType, FilterMode filterMode) const;

	// use
	void use(ShaderPass pass);
	void setupPipeline(Renderer& renderer);
	virtual ~FMaterial() {}

	// attributes
	bool hasEmission() const;
	glm::vec3 getEmission() const;
	glm::vec3 getEmission(float u, float v) const;

protected:

private:
	EShadingModel shadingMode_ = EShadingModel::Shading_BaseColor;

	// unifroms包括uniform block和sampler，每次渲染的时候都需要重新绑定到对应的shader
	mutable std::shared_ptr<ShaderResources> shaderResources_;

	std::unordered_map<int, TextureData> textureData_; // TextureType(int) -> textureData

	// just find a place to throw textures in, managing its life-cycle
	mutable std::unordered_map<int, std::shared_ptr<Texture>> texturesRuntime_;

	// shaders
	std::unordered_map<ShaderPass, std::shared_ptr<Shader>> shaders_;
	std::vector<std::string> defines_;
	bool shaderReady_ = false; // shader setupPipeline or not
	bool texturesReady_ = false; // have textures been loaded to piepleine?

	// Material Attributes
	glm::vec3 emission {};
};

template<typename T>
T FMaterial::sample2D(float u, float v, TextureType texType, FilterMode filterMode) const {
	if (texturesRuntime_.contains(texType)) {
		auto&& texture = texturesRuntime_[texType];
		auto&& texData = texture->getTextureData();
		if (!texData.dataArray.empty()) {
			T ret = texData.dataArray[0]->sample2D(u, v, filterMode);
			return ret;
		}
	}
	return {};
}
