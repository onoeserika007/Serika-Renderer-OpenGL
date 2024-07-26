#pragma once
#include <unordered_map>
#include "../Utils/ImageUtils.h"
#include "../Utils/UniversalUtils.h"
#include <string>
#include <memory>
#include <mutex>

#include "assimp/mesh.h"
#include "assimp/scene.h"

class UObject;
class UMesh;
class UModel;

class ResourceLoader {
public:
	std::unordered_map<std::string, std::shared_ptr<Buffer<RGBA>>> textureDataCache_;
	std::shared_ptr<Buffer<RGBA>> loadTexture(const std::string& path);

	std::unordered_map<std::string, std::string> shaderSourceCache_;
	std::string loadShader(const std::string& path);
	std::shared_ptr<UModel> loadModel(const std::string& path, bool bUseThreadPool = false);
	std::shared_ptr<UMesh> loadSkyBox(const std::string& path);

	ResourceLoader(const ResourceLoader&) = delete;
	ResourceLoader& operator=(const ResourceLoader&) = delete;
	static ResourceLoader& getInstance();
private:
	ResourceLoader() = default;

	std::string loadingDirectory;

	std::shared_ptr<UModel> processNode(aiNode* node, const aiScene* scene, bool bMultiThread);
	std::shared_ptr<UMesh> processMesh(aiMesh *mesh, const aiScene *scene);

	std::mutex texCacheMutex_;
};
