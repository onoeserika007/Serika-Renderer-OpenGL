#pragma once
#include <string>
#include <unordered_map>
#include <string>
#include "Shader.h"
#include <memory>
class Texture {
	struct PassKey
	{
		explicit PassKey() {}
	};
	static int texture_count_;
	static std::unordered_map<std::string, std::shared_ptr<Texture>> texture_map_;
	unsigned int texture_;
	int width, height, nrChannels;
	unsigned textureLoc = 0;
	std::string name_;
	Texture(const std::string& picture);
public:
	Texture() {}
	Texture(PassKey _, const std::string& picture): Texture(picture) {}
	static std::shared_ptr<Texture> loadTexture(const std::string& name, const std::string& picture);
	void use();
	void setUnit(std::shared_ptr<Shader> pshader, unsigned loc);
	void setName(std::string& name);
};

// 如果将静态成员变量的初始化放在头文件中，每个包含该头文件的源文件都会有一份独立的静态成员变量，导致重定义错误。
// 将初始化放在一个源文件中可以确保只有一个初始化，而其他源文件只是包含了声明，不会导致重复定义。