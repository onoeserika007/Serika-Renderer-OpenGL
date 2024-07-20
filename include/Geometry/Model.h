#pragma once

#include <string>
#include <memory>
#include <vector>

#include "Mesh.h"
#include "Object.h"
#include "assimp/Importer.hpp"
#include "assimp/scene.h"

class Material;
class UObject;
class Camera;
class Texture;
class ULight;
class Shader;
class Renderer;
class UMesh;

class UModel: public UMesh{
public:
	template <typename... Args>
	static std::shared_ptr<UModel> makeModel(Args&&... args);

	// std::vector<std::shared_ptr<UMesh>> &getMeshes();
	void loadModel(const std::string& path);
private:
	UModel();
	/* model data */
	// std::vector<std::shared_ptr<UMesh>> meshes;
	std::string directory;
	std::vector<std::shared_ptr<Texture>> textures;

	/* function */
	void processNode(aiNode* node, const aiScene* scene);

	std::shared_ptr<UMesh> processMesh(aiMesh *mesh, const aiScene *scene);

	// std::vector<std::shared_ptr<Texture>> loadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName);
};

template<typename ... Args>
std::shared_ptr<UModel> UModel::makeModel(Args &&...args) {
	return std::shared_ptr<UModel>(new UModel(std::forward<Args>(args)...));
}
