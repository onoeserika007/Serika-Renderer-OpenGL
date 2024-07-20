#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <string>
#include <memory>
#include <vector>

class Material;
class UObject;
class Camera;
class Texture;
class ULight;
class Shader;
class Renderer;
class UMesh;

class Model {
public:
	Model(const std::string& path);
	void setScale(float x, float y, float z);

	//void setupPipeline(Renderer& renderer);
	std::vector<std::shared_ptr<UMesh>> &getMeshes();
private:
	/* model data */
	std::vector<std::shared_ptr<UMesh>> meshes;
	std::string directory;
	std::vector<std::shared_ptr<Texture>> textures;

	/* function */
	void loadModel(const std::string& path);
	void processNode(aiNode* node, const aiScene* scene);

	std::shared_ptr<UMesh> processMesh(aiMesh *mesh, const aiScene *scene);

	// std::vector<std::shared_ptr<Texture>> loadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName);
};