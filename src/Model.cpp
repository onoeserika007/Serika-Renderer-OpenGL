#include "Model.h"
#include "Renderer.h"
#include "Camera.h"
#include "Object.h"
#include "Texture.h"
#include "Material.h"
#include "Geometry.h"
#include "BufferAttribute.h"
#include "ResourceLoader.h"
#include "Shader.h"
#include "Light.h"
#include <iostream>

Model::Model(const std::string& path)
{
	loadModel(path);
}

void Model::draw(Renderer& renderer)
{
	for (auto& mesh : meshes) {
		mesh->draw(renderer);
	}
}

void Model::setLightArray(std::vector<std::shared_ptr<Light>> lightArray)
{
	for (auto& mesh : meshes) {
		mesh->setLightArray(lightArray);
	}
}

void Model::setScale(float x, float y, float z)
{
	for (auto& mesh : meshes) {
		mesh->setScale(x, y, z);
	}
}

//void Model::setupPipeline(Renderer& renderer)
//{
//	for (auto& mesh : meshes) {
//		mesh->setupPipeline(renderer);
//	}
//}

std::vector<std::shared_ptr<Object>>& Model::getMeshes()
{
	// TODO: 在此处插入 return 语句
	return meshes;
}

void Model::setShader(std::shared_ptr<Shader> pshader)
{
	for (auto& mesh : meshes) {
		mesh->setpshader(pshader->clone());
	}
}

void Model::loadModel(const std::string& path)
{
	Assimp::Importer importer;
	/*aiProcess_GenNormals：如果模型不包含法向量的话，就为每个顶点创建法线。
	aiProcess_SplitLargeMeshes：将比较大的网格分割成更小的子网格，如果你的渲染有最大顶点数限制，只能渲染较小的网格，那么它会非常有用。
	aiProcess_OptimizeMeshes：和上个选项相反，它会将多个小网格拼接为一个大的网格，减少绘制调用从而进行优化。*/
	const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);

	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		std::cout << "ERROR::ASSIMP::" << importer.GetErrorString() << std::endl;
		return;
	}
	directory = path.substr(0, path.find_last_of('/'));

	processNode(scene->mRootNode, scene);
}

void Model::processNode(aiNode* node, const aiScene* scene)
{
	// 处理节点所有的网格（如果有的话）
	for (unsigned int i = 0; i < node->mNumMeshes; i++)
	{
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		meshes.push_back(processMesh(mesh, scene));
	}
	// 接下来对它的子节点重复这一过程
	for (unsigned int i = 0; i < node->mNumChildren; i++)
	{
		processNode(node->mChildren[i], scene);
	}
}

std::shared_ptr<Object> Model::processMesh(aiMesh* mesh, const aiScene* scene)
{
	std::vector<float> PosArray;
	std::vector<float> TexCoordArray;
	std::vector<float> NormalArray;
	std::vector<unsigned int> indices;
	// 因为assimp的mesh里只存了一个指向texture的index
	for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
		PosArray.emplace_back(mesh->mVertices[i].x);
		PosArray.emplace_back(mesh->mVertices[i].y);
		PosArray.emplace_back(mesh->mVertices[i].z);

		if (mesh->mTextureCoords[0]) // 网格是否有纹理坐标？
		{
			TexCoordArray.emplace_back(mesh->mTextureCoords[0][i].x);
			TexCoordArray.emplace_back(mesh->mTextureCoords[0][i].y);
		}
		else {
			TexCoordArray.emplace_back(0.0f);
			TexCoordArray.emplace_back(0.0f);
		}

		NormalArray.emplace_back(mesh->mNormals[i].x);
		NormalArray.emplace_back(mesh->mNormals[i].y);
		NormalArray.emplace_back(mesh->mNormals[i].z);
	}

	BufferAttribute posAttrs(PosArray, 3), texcoordAttrs(TexCoordArray, 2), normalAttrs(NormalArray, 3);
	auto pgeomerty = std::make_shared<Geometry>();
	pgeomerty->setAttribute("aPos", posAttrs, true);
	pgeomerty->setAttribute("aTexCoord", texcoordAttrs);
	pgeomerty->setAttribute("aNormal", normalAttrs);
	// 处理索引
	for (unsigned int i = 0; i < mesh->mNumFaces; i++)
	{
		aiFace face = mesh->mFaces[i];
		for (unsigned int j = 0; j < face.mNumIndices; j++)
			indices.push_back(face.mIndices[j]);
	}
	pgeomerty->setIndex(indices);

	// 处理材质
	auto pmaterial = std::make_shared<StandardMaterial>();
	//std::vector<aiTextureType> types = { aiTextureType_DIFFUSE, aiTextureType_SPECULAR, aiTextureType_NORMALS, aiTextureType_UNKNOWN };
	if (mesh->mMaterialIndex >= 0)
	{
		aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
		for (int i = 0; i <= aiTextureType_TRANSMISSION; i++) {
			//const auto& type = types[i];
			const auto& type = static_cast<aiTextureType>(i);
			const auto& typeDefine = Texture::samplerDefine(static_cast<TextureType>(i));
			//const auto& typeName = std::to_string(type);
			for (unsigned int j = 0; j < material->GetTextureCount(type); j++)
			{
				aiString str;
				material->GetTexture(type, j, &str);

				const std::string picture = directory + "/" + str.C_Str();
				auto bufferData = ResourceLoader::loadTexture(picture);
				TextureData texData;
				texData.dataArray = { bufferData };
				texData.path = picture;

				// texture->loadTextureData(directory + "/" + str.C_Str());
				// pmaterial->loadMap(directory + "/" + str.C_Str(), typeName);
				pmaterial->setTextureData(static_cast<TextureType>(i), texData);
				pmaterial->addDefine(typeDefine);
			}
		}
	}


	auto pobject = std::make_shared<Object>(pgeomerty, pmaterial);
	return pobject;
}

//std::vector<std::shared_ptr<Texture>> Model::loadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName)
//{
//	std::vector<std::shared_ptr<Texture>> textures;
//	for (unsigned int i = 0; i < mat->GetTextureCount(type); i++)
//	{
//		// 获取每个纹理的文件位置，它会将结果储存在一个aiString中
//		aiString str;
//		mat->GetTexture(type, i, &str);
//		auto texture = Texture::loadTexture(directory + str.C_Str(), typeName);
//		textures.push_back(texture);
//	}
//	return textures;
//}