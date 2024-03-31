#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <string>
#include <memory>
#include <vector>

class Material;
class Object;
class Camera;
class Texture;
class Light;
class Shader;
class Renderer;

class Model {
public:
	Model(const std::string& path);
	void draw(Renderer& renderer);
	void setShader(std::shared_ptr<Shader> pshader);
	void setLightArray(std::vector<std::shared_ptr<Light>> lightArray);
	void setScale(float x, float y, float z);

	void setupPipeline(Renderer& renderer);
private:
	/* model data */
	std::vector<std::shared_ptr<Object>> meshes;
	std::string directory;
	std::vector<std::shared_ptr<Texture>> textures;

	/* function */
	void loadModel(const std::string& path);
	void processNode(aiNode* node, const aiScene* scene);
	std::shared_ptr<Object> processMesh(aiMesh* mesh, const aiScene* scene);
	//std::vector<std::shared_ptr<Texture>> loadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName);
};