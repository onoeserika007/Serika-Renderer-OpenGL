#include "Base/ResourceLoader.h"

#include <future>

#include "Geometry/BufferAttribute.h"
#include "assimp/Importer.hpp"
#include "assimp/postprocess.h"
#include "Base/ThreadPool.h"
#include "Geometry/Geometry.h"
#include "Geometry/Primitives.h"
#include "Geometry/UMesh.h"


std::shared_ptr<Buffer<RGBA>> ResourceLoader::loadTexture(const std::string& path) {

	texCacheMutex_.lock();
	if (textureDataCache_.contains(path)) {
		auto&& buffer = textureDataCache_[path];
		texCacheMutex_.unlock();
		return buffer;
	}
	texCacheMutex_.unlock();

	auto&& buffer = ImageUtils::readImageRGBA(path, true);

	texCacheMutex_.lock();
	textureDataCache_[path] = buffer;
	texCacheMutex_.unlock();

	// copy ctor
	return buffer? Buffer<RGBA>::makeBuffer(*buffer) : nullptr;
}

std::string ResourceLoader::loadShader(const std::string& path)
{
	if (path.empty()) return {};
	if (!shaderSourceCache_.contains(path)) {
		auto shaderSource = readFile(path);
		shaderSourceCache_[path] = std::string(shaderSource.data(), shaderSource.data() + shaderSource.size());
	}
	return shaderSourceCache_[path];
}

std::shared_ptr<UMesh> ResourceLoader::loadMesh(const std::string &path, bool bUseThreadPool) {
	Assimp::Importer importer;
	/* aiProcess_GenNormals：如果模型不包含法向量的话，就为每个顶点创建法线。
	aiProcess_SplitLargeMeshes：将比较大的网格分割成更小的子网格，如果你的渲染有最大顶点数限制，只能渲染较小的网格，那么它会非常有用。
	aiProcess_OptimizeMeshes：和上个选项相反，它会将多个小网格拼接为一个大的网格，减少绘制调用从而进行优化。
	*/
	TEST_TIME_COST(const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs), Assimp_Import);

	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		std::cout << "ERROR::ASSIMP::" << importer.GetErrorString() << std::endl;
		return {};
	}

	*scene->mRootNode; // ??? deref here can fix load mistake

	loadingDirectory = path.substr(0, path.find_last_of('/'));

	auto&& retMesh = UMesh::makeMesh();
	TEST_TIME_COST(processNode(scene->mRootNode, scene, retMesh, bUseThreadPool), Process_Node);

	return retMesh;
}

void ResourceLoader::processNode(aiNode *node, const aiScene *scene, std::shared_ptr<UMesh> &parentMesh,
                                 bool bMultiThread) {

	auto&& threadPool = FThreadPool::getInst();

	if (bMultiThread) {
		std::vector<std::future<std::shared_ptr<UMesh>>> futures;
		// 处理节点所有的网格（如果有的话）
		for (unsigned int i = 0; i < node->mNumMeshes; i++)
		{
			aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
			auto&& future = threadPool.pushTask([mesh, scene, this] { return processMesh(mesh, scene); } );
			futures.emplace_back(std::move(future));
		}

		for (auto&& loadedMesh: futures) {
			parentMesh->addMesh(loadedMesh.get());
		}
	}
	else {
		// 处理节点所有的网格（如果有的话）
		for (unsigned int i = 0; i < node->mNumMeshes; i++)
		{
			aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
			parentMesh->addMesh(processMesh(mesh, scene));
		}
	}

	// 接下来对它的子节点重复这一过程
	for (unsigned int i = 0; i < node->mNumChildren; i++)
	{
		processNode(node->mChildren[i], scene, parentMesh, false);
	}
}

std::shared_ptr<UMesh> ResourceLoader::processMesh(aiMesh *mesh, const aiScene *scene) {
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

		if (mesh->mNormals) // check if has normal
		{
			NormalArray.emplace_back(mesh->mNormals[i].x);
			NormalArray.emplace_back(mesh->mNormals[i].y);
			NormalArray.emplace_back(mesh->mNormals[i].z);
		}
	}

	BufferAttribute posAttrs(PosArray, 3), texcoordAttrs(TexCoordArray, 2), normalAttrs(NormalArray, 3);
	auto pgeomerty = std::make_shared<FGeometry>();
	pgeomerty->setAttribute(EBA_Position, posAttrs, true);
	pgeomerty->setAttribute(EBA_TexCoord, texcoordAttrs);
	pgeomerty->setAttribute(EBA_Normal, normalAttrs);
	// 处理索引
	for (unsigned int i = 0; i < mesh->mNumFaces; i++)
	{
		aiFace face = mesh->mFaces[i];
		for (unsigned int j = 0; j < face.mNumIndices; j++)
			indices.push_back(face.mIndices[j]);
	}
	pgeomerty->setIndex(indices);

	// 处理材质
	auto pmaterial = std::make_shared<FMaterial>();
	//std::vector<aiTextureType> types = { aiTextureType_DIFFUSE, aiTextureType_SPECULAR, aiTextureType_NORMALS, aiTextureType_UNKNOWN };
	if (static_cast<int>(mesh->mMaterialIndex) >= 0)
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

				const std::string picture = loadingDirectory + "/" + str.C_Str();
				auto bufferData = loadTexture(picture);
				TextureData texData;
				texData.dataArray = { bufferData };
				texData.path = picture;
				texData.loadedTextureType = static_cast<TextureType>(i);

				pmaterial->setTextureData(static_cast<TextureType>(i), texData);
				pmaterial->addDefine(typeDefine);
			}
		}
	}

	return UMesh::makeMesh(pgeomerty, pmaterial);
}

std::shared_ptr<UMesh> ResourceLoader::loadSkyBox(const std::string &path) {
	if (path.empty() || !StringUtils::isDirectory(path)) {
		LOGW("Path not compatible with sky box, return empty!");
		return {};
	}
	loadingDirectory = path.substr(0, path.find_last_of('/'));

	auto&& mesh = MeshMakers::loadCubeMesh(true); // load reverse face to support cull face

	auto&& pMaterial = std::make_shared<FMaterial>();

	std::vector<std::shared_ptr<Buffer<RGBA>>> skyboxTex (6);
	std::vector<std::future<void>> futures;

	auto&& thredPool = FThreadPool::getInst();
	futures.emplace_back(thredPool.pushTask([&] { skyboxTex[0] = loadTexture(StringUtils::appendToDir(path, "right.jpg")); }));
	futures.emplace_back(thredPool.pushTask([&] { skyboxTex[1] = loadTexture(StringUtils::appendToDir(path, "left.jpg")); }));
	futures.emplace_back(thredPool.pushTask([&] { skyboxTex[2] = loadTexture(StringUtils::appendToDir(path, "top.jpg")); }));
	futures.emplace_back(thredPool.pushTask([&] { skyboxTex[3] = loadTexture(StringUtils::appendToDir(path, "bottom.jpg")); }));
	futures.emplace_back(thredPool.pushTask([&] { skyboxTex[4] = loadTexture(StringUtils::appendToDir(path, "front.jpg")); }));
	futures.emplace_back(thredPool.pushTask([&] { skyboxTex[5] = loadTexture(StringUtils::appendToDir(path, "back.jpg")); }));
	for (auto&& f: futures) f.wait();

	TextureData texData;
	texData.dataArray = std::move(skyboxTex);
	texData.loadedTextureType = TEXTURE_TYPE_CUBE;
	texData.path = path;
	const auto& typeDefine = Texture::samplerDefine(TEXTURE_TYPE_CUBE);
	pMaterial->addDefine(typeDefine);
	pMaterial->setTextureData(TEXTURE_TYPE_CUBE, texData);

	mesh->setMaterial(pMaterial);
	return mesh;
}

ResourceLoader & ResourceLoader::getInstance() {
	static ResourceLoader loader;
	return loader;
}

