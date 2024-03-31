#pragma once
#include <iostream>
#include <unordered_map>
#include "Utils//ImageUtils.h"
#include "Utils/utils.h"
#include <string>

class ResourceLoader {
public:
	static std::unordered_map<std::string, std::shared_ptr<Buffer<RGBA>>> textureDataCache_;
	static std::shared_ptr<Buffer<RGBA>> loadTexture(const std::string& path);

	static std::unordered_map<std::string, std::string> shaderSourceCache_;
	static std::string loadShader(const std::string& path);
};
