#ifndef SERIKA_RENDERER_INCLUDE_MATERIAL_FMATERIAL_H
#define SERIKA_RENDERER_INCLUDE_MATERIAL_FMATERIAL_H
#include <memory>
#include <string>
#include <vector>
#include <unordered_map>
#include "Base/Globals.h"
#include "Texture.h"
#include "Uniform.h"
#include "Base/Config.h"

class ShaderResources;

enum AlphaMode {
	Alpha_Opaque,
	Alpha_Blend,
};

struct MaterialInfoBlock {
	alignas(16) glm::vec3 uAlbedo {};
	alignas(16) glm::vec3 uEmission {};
	// Disney attributes
	alignas(4) float uSubsurface = 0.0;
	alignas(4) float uMetallic = 0.0;
	alignas(4) float uSpecular = 0.0;
	alignas(4) float uSpecularTint = 0.0;
	alignas(4) float uRoughness = 1.0;
	alignas(4) float uAnisotropic = 0.0;
	alignas(4) float uSheen = 0.0;
	alignas(4) float uSheenTint = 0.0;
	alignas(4) float uClearcoat = 0.0;
	alignas(4) float uClearcoatGloss = 0.0;
	alignas(4) float uIOR = 1.0;
	alignas(4) float uTransmission = 0.0;
};


class Shader;
class ULight;
class Uniform;
class Renderer;

struct MaterialResource {
	std::vector<std::string> defines_;
	std::unordered_map<int, TextureData> textureData_; // TextureType(int) -> textureData

	// just find a place to throw textures in, managing its life-cycle
	mutable std::unordered_map<int, std::shared_ptr<Texture>> texturesRuntime_;
	bool texturesReady_ = false; // have textures been loaded to piepleine?
	bool loadMipmap = false;
	void checkMipmapSetting() {
		auto&& config = Config::getInstance();
		if (config.bUseMipmaps != loadMipmap) {
			texturesReady_ = false;
			loadMipmap = config.bUseMipmaps;
		}
	}
};

// 应该区分元信息和运行时资源
// 比如shader是运行时资源，shadingMode是元信息，但是shader需要根据shadingMode的改变重新编译加载
// shader还是现在在外部手动加载的，所以需要引入shadingMode
// shadowMapTexture 就在Material setup时加载
class FMaterial {
public:
	FMaterial();
	FMaterial(std::string name, std::shared_ptr<Shader> pshader);
	FMaterial(const FMaterial& other);

	// getters
	bool shaderReady() const;
	bool samplerReady() const { return bSamplerReady_; }
	bool texturesReady() const;
	bool isPipelineReady() const { return shaderReady() && texturesReady() && samplerReady(); }
	std::shared_ptr<Shader> getShader(ShaderPass pass);
	std::unordered_map<ShaderPass, std::shared_ptr<Shader>>& getShaders();
	std::shared_ptr<ShaderResources> getShaderResources() const;
	std::shared_ptr<UniformSampler> getUniformSampler(const std::string& name) const;
	std::unordered_map<int, TextureData>& getTextureData();
	EShadingModel shadingMode();
	std::vector<std::string>& getDefines();

	// setters and adders
	void setUniformSampler(const std::string& name, const std::shared_ptr<UniformSampler>& uniform);
	void clearUniformSamplers();
	void setUniformBlock(const std::string& name, const std::shared_ptr<UniformBlock> &uniform);
	void setShaderResources(const std::shared_ptr<ShaderResources>& shader_resources);
	void setShader(ShaderPass pass, std::shared_ptr<Shader> pshader);
	void setShaderReady(bool setValue);
	void setSamplerReady(bool bState) { bSamplerReady_ = bState; }
	void setTexturesReady(bool ready);

	// Texture Data
	void setTextureData(TextureType textureType, const TextureData &texData);
	void setTextureData(TextureType textureType, const std::string& texPath);
	void setTexture_runtime(TextureType texType, const std::shared_ptr<Texture> &pTex);
	void checkMipmaps() const;
	void clearTextures_runtime();
	void setShadingMode(EShadingModel mode);
	void addDefine(const std::string& define);

	// sample
	template<typename T>
	T sample2D(float u, float v, TextureType texType, FilterMode filterMode) const;

	// use
	void use(ShaderPass pass);
	std::shared_ptr<MaterialResource> getMaterialObject() const;

	virtual ~FMaterial() = default;

	// attributes
	bool hasEmission() const;
	glm::vec3 getEmission() const;
	glm::vec3 getEmission(float u, float v) const;
	glm::vec3 getDiffuse() const;
	glm::vec3 getDiffuse(float u, float v) const;
	glm::vec3 getSpecular(float u, float v) const;
	void setEmission(const glm::vec3& color) { material_info_.uEmission = color; }
	void setDiffuse(const glm::vec3& color) { material_info_.uAlbedo = color; }

	// wi represent ViewDir, incident // wo represents LightDir, goes off the place
    glm::vec3 evalRadiance(const glm::vec3 &wi, const glm::vec3 &wo, const glm::vec3 &N, const float u = 0.f, const float v = 0.f) const;

	// Disney attributes
	MaterialInfoBlock material_info_;

	/** Copy Instance*/
	std::shared_ptr<FMaterial> Clone() const;

protected:

private:
	EShadingModel shadingModel_ = EShadingModel::Shading_BaseColor;

	std::shared_ptr<UniformBlock> materialInfoUniformBlock_;

	// unifroms包括uniform block和sampler，每次渲染的时候都需要重新绑定到对应的shader
	mutable std::shared_ptr<ShaderResources> shaderResources_;
	mutable std::shared_ptr<MaterialResource> materialObject_;

	// shaders
	std::unordered_map<ShaderPass, std::shared_ptr<Shader>> shaders_;
	bool bShaderReady_ = false; // shader setupPipeline or not
	bool bSamplerReady_ = false; // has samplers bound to material

	// Material Attributes // explicitly assigned, prior to texture
	Serika::UUID<FMaterial> uuid_;
};

template<typename T>
T FMaterial::sample2D(float u, float v, TextureType texType, FilterMode filterMode) const {
	if (materialObject_) {
		if (materialObject_->texturesRuntime_.contains(texType)) {
			auto&& texture = materialObject_->texturesRuntime_[texType];
			auto&& texData = texture->getTextureData();
			if (!texData.unitDataArray.empty()) {
				T ret = texData.unitDataArray[0]->sample2D(u, v, filterMode);
				return ret;
			}
		}
	}
	return {};
}

#endif // SERIKA_RENDERER_INCLUDE_MATERIAL_FMATERIAL_H
