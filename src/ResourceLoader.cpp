#include "../include/Base/ResourceLoader.h"

#include "Geometry/Model.h"

std::unordered_map<std::string, std::shared_ptr<Buffer<RGBA>>> ResourceLoader::textureDataCache_;
std::unordered_map<std::string, std::string> ResourceLoader::shaderSourceCache_;

std::shared_ptr<Buffer<RGBA>> ResourceLoader::loadTexture(const std::string& path) {
	if (!textureDataCache_.count(path)) {
		textureDataCache_[path] = ImageUtils::readImageRGBA(path, true);
	}
	// copy ctor
	return std::make_shared<Buffer<RGBA>>(*textureDataCache_[path]);
}

std::string ResourceLoader::loadShader(const std::string& path)
{
	if (!shaderSourceCache_.count(path)) {
		auto shaderSource = readFile(path);
		shaderSourceCache_[path] = std::string(shaderSource.data(), shaderSource.data() + shaderSource.size());
	}
	return shaderSourceCache_[path];
}

std::shared_ptr<UModel> ResourceLoader::loadModel(const std::string &path) {
	auto ret = UModel::makeModel();
	ret->loadModel(path);
	return ret;
}
