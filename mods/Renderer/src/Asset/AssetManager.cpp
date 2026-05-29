#include "Asset/AssetManager.h"

#include "Application.h"
#include "Base/ResourceLoader.h"
#include "Geometry/UMesh.h"
#include "OpenGL/ShaderGLSL.h"
#include "Renderer.h"
#include "Utils/SRKLogger.h"

auto AssetManager::getInstance() -> AssetManager & {
	static AssetManager instance;
	return instance;
}

auto AssetManager::stats() const -> AssetStats {
	std::lock_guard<std::mutex> lock(mutex_);
	AssetStats stats;

	auto textureIt = storages_.find(std::type_index(typeid(Texture)));
	if (textureIt != storages_.end()) {
		stats.textureCount = textureIt->second->loadedCount();
	}

	auto meshIt = storages_.find(std::type_index(typeid(UMesh)));
	if (meshIt != storages_.end()) {
		stats.meshCount = meshIt->second->loadedCount();
	}

	auto shaderIt = storages_.find(std::type_index(typeid(Shader)));
	if (shaderIt != storages_.end()) {
		stats.shaderCount = shaderIt->second->loadedCount();
	}

	return stats;
}

template <>
auto AssetManager::load<Texture>(const Texture &desc) -> std::shared_ptr<Texture> {
	std::shared_ptr<Texture> cachedTexture = findLoadedResource<Texture>(desc);
	if (cachedTexture != nullptr) {
		LOGD("AssetManager texture cache hit: %s", desc.getTextureData().path.c_str());
		return cachedTexture;
	}

	TextureInfo textureInfo = desc.getTextureInfo();
	SamplerInfo samplerInfo = desc.getSamplerInfo();
	TextureData textureData = desc.getTextureData();
	LOGD("AssetManager texture cache miss, loading: %s", textureData.path.c_str());

	if (textureData.unitDataArray.empty() && !textureData.path.empty()) {
		std::shared_ptr<Buffer<RGBA>> cpuData = ResourceLoader::getInstance().loadTexture(textureData.path);
		if (cpuData == nullptr) {
			LOGE("AssetManager failed to load texture CPU data: %s", textureData.path.c_str());
			return nullptr;
		}

		textureData.unitDataArray.push_back(cpuData);
		if (textureInfo.width == 0) {
			textureInfo.width = cpuData->width();
		}
		if (textureInfo.height == 0) {
			textureInfo.height = cpuData->height();
		}
	}

	std::shared_ptr<Renderer> renderer = Application::getRenderer();
	if (renderer == nullptr) {
		LOGE("AssetManager failed to create texture, renderer is null: %s", textureData.path.c_str());
		return nullptr;
	}

	std::shared_ptr<Texture> texture = renderer->createTexture(textureInfo, samplerInfo, textureData);
	if (texture == nullptr) {
		LOGE("AssetManager renderer failed to create texture: %s", textureData.path.c_str());
		return nullptr;
	}

	storeLoadedResource<Texture>(desc, texture);
	LOGD("AssetManager loaded texture: %s", textureData.path.c_str());
	return texture;
}

template <>
auto AssetManager::load<UMesh>(const MeshDesc &desc) -> std::shared_ptr<UMesh> {
	std::shared_ptr<UMesh> cachedMesh = findLoadedResource<UMesh>(desc);
	if (cachedMesh != nullptr) {
		LOGD("AssetManager mesh cache hit: %s", desc.path.c_str());
		return cachedMesh;
	}

	LOGD("AssetManager mesh cache miss, loading: %s", desc.path.c_str());
	std::shared_ptr<UMesh> mesh = ResourceLoader::getInstance().loadMesh(desc.path);
	if (mesh == nullptr) {
		LOGE("AssetManager failed to load mesh: %s", desc.path.c_str());
		return nullptr;
	}

	storeLoadedResource<UMesh>(desc, mesh);
	LOGD("AssetManager loaded mesh: %s", desc.path.c_str());
	return mesh;
}

template <>
auto AssetManager::load<Shader>(const ShaderDesc &desc) -> std::shared_ptr<Shader> {
	std::shared_ptr<Shader> cachedShader = findLoadedResource<Shader>(desc);
	if (cachedShader != nullptr) {
		LOGD("AssetManager shader cache hit: vs=%s fs=%s gs=%s", desc.vertexPath.c_str(), desc.fragmentPath.c_str(), desc.geometryPath.c_str());
		return cachedShader;
	}

	LOGD("AssetManager shader cache miss, loading: vs=%s fs=%s gs=%s", desc.vertexPath.c_str(), desc.fragmentPath.c_str(), desc.geometryPath.c_str());
	std::shared_ptr<ShaderGLSL> shader = ShaderGLSL::loadShader(desc.vertexPath, desc.fragmentPath, desc.geometryPath);
	if (shader == nullptr) {
		LOGE("AssetManager failed to load shader files: vs=%s fs=%s gs=%s", desc.vertexPath.c_str(), desc.fragmentPath.c_str(), desc.geometryPath.c_str());
		return nullptr;
	}

	shader->compileAndLink();
	storeLoadedResource<Shader>(desc, shader);
	LOGD("AssetManager loaded shader: vs=%s fs=%s gs=%s", desc.vertexPath.c_str(), desc.fragmentPath.c_str(), desc.geometryPath.c_str());
	return shader;
}
