#include "stb_image.h"
#include "glad/glad.h"
#include "Texture.h"
#include <iostream>

int Texture::texture_count_ = 0;
// 报错是因为没有声明/定义类静态成员texture_map_
std::unordered_map<std::string, Texture> Texture::texture_map_;

Texture Texture::loadTexture(const std::string& picture) {
	if (texture_map_.count(picture)) {
		return texture_map_[picture];
	}
	else {
		auto& ret = texture_map_[picture] = Texture(picture);
		return ret;
	}
}

Texture::Texture(const std::string& picture) :texture_(0){
	// desired_channels：希望图像数据被加载到的通道数。可以是 0、1、2 或 3。0 表示使用图像文件中的通道数，1 表示灰度图，2 表示灰度图的 Alpha 通道，3 表示 RGB 图像。
	// 如果文件中包含 Alpha 通道，但 desired_channels 设置为 3，那么 Alpha 通道将被丢弃。可以为 NULL，如果不关心。
	// glGenTextures函数首先需要输入生成纹理的数量，然后把它们储存在第二个参数的unsigned int数组中
	glGenTextures(1, &texture_);
	// 激活纹理单元
	glActiveTexture(GL_TEXTURE0 + texture_count_);
	// 绑定纹理，让之后的任何纹理指令都对当前纹理生效
	glBindTexture(GL_TEXTURE_2D, texture_);
	// 为当前绑定的纹理对象设置环绕、过滤方式
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	//std::cout << picture << std::endl;
	// 你可能注意到纹理上下颠倒了！这是因为OpenGL要求y轴0.0坐标是在图片的底部的，但是图片的y轴0.0坐标通常在顶部。
	// 很幸运，stb_image.h能够在图像加载时帮助我们翻转y轴，只需要在加载任何图像前加入以下语句即可：
	stbi_set_flip_vertically_on_load(true);
	unsigned char* data = stbi_load(picture.c_str(), &width, &height, &nrChannels, 0);
	if (data) {
		// 第一个参数指定了纹理目标(Target)。设置为GL_TEXTURE_2D意味着会生成与当前绑定的纹理对象在同一个目标上的纹理（任何绑定到GL_TEXTURE_1D和GL_TEXTURE_3D的纹理不会受到影响）。
		// 第二个参数为纹理指定多级渐远纹理的级别，如果你希望单独手动设置每个多级渐远纹理的级别的话。这里我们填0，也就是基本级别，也就是不手动设置
		// 第三个参数告诉OpenGL我们希望把纹理储存为何种格式。我们的图像只有RGB值，因此我们也把纹理储存为RGB值。
		// 第四个和第五个参数设置最终的纹理的宽度和高度。我们之前加载图像的时候储存了它们，所以我们使用对应的变量。
		// 下个参数应该总是被设为0（历史遗留的问题）。
		// 第七第八个参数定义了源图的格式和数据类型。我们使用RGB值加载这个图像，并把它们储存为char(byte)数组，我们将会传入对应值。
		// 最后一个参数是真正的图像数据。
		if (nrChannels == 3) {
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		}
		else if (nrChannels == 4) {
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		}
		// 当调用glTexImage2D时，当前绑定的纹理对象就会被附加上纹理图像。然而，目前只有基本级别(Base-level)的纹理图像被加载了，如果要使用多级渐远纹理，我们必须手动设置所有不同的图像（不断递增第二个参数）。
		// 或者，直接在生成纹理之后调用glGenerateMipmap。这会为当前绑定的纹理自动生成所有需要的多级渐远纹理。
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else {
		std::cout << "Failed to load texture" << std::endl;
	}
	// 使用库去释放内存，不要自己释放！
	stbi_image_free(data);
	texture_count_++;
}

void Texture::use() {
	glBindTexture(GL_TEXTURE_2D, texture_);
}