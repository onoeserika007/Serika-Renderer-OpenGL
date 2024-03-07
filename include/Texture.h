#include <string>
#include <unordered_map>
#include <string>
class Texture {
	static int texture_count_;
	unsigned int texture_;
	int width, height, nrChannels;
	static std::unordered_map<std::string, Texture> texture_map_;
	Texture(const std::string& picture);
public:
	Texture() {}
	static Texture loadTexture(const std::string& picture);
	void use();
};

// 如果将静态成员变量的初始化放在头文件中，每个包含该头文件的源文件都会有一份独立的静态成员变量，导致重定义错误。
// 将初始化放在一个源文件中可以确保只有一个初始化，而其他源文件只是包含了声明，不会导致重复定义。