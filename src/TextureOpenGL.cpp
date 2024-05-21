#include "TextureOpenGL.h"
#include <glad/glad.h>
#include "OpenGL/EnumsOpenGL.h"
#include "Utils//OpenGLUtils.h"

TextureOpenGL::TextureOpenGL(const TextureInfo& texInfo, const SamplerInfo& smInfo): Texture(texInfo, smInfo)
{
}

TextureOpenGL::TextureOpenGL(const TextureInfo& texInfo, const SamplerInfo& smInfo, const TextureData& texData): Texture(texInfo, smInfo, texData)
{
}

TextureOpenGL::~TextureOpenGL()
{
}


TextureOpenGL2D::TextureOpenGL2D(const TextureInfo& texInfo, const SamplerInfo& smInfo) : TextureOpenGL(texInfo, smInfo)
{
	assert(texInfo.target == TextureTarget::TextureTarget_2D);
	setupPipeline();
}

TextureOpenGL2D::TextureOpenGL2D(const TextureInfo& texInfo, const SamplerInfo& smInfo, const TextureData& texData): TextureOpenGL(texInfo, smInfo, texData)
{
	assert(texInfo.target == TextureTarget::TextureTarget_2D);
	setupPipeline();
}

void TextureOpenGL2D::setupPipeline()
{
	GLuint& textureId = textureId_;
	auto& samplerInfo = getSamplerInfo();
	auto& textureInfo = getTextureInfo();
	const auto& openglTextureInfo = OpenGL::cvtTextureFormat(static_cast<TextureFormat>(textureInfo.format));
	auto target = multiSample() ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D;
	// desired_channels：希望图像数据被加载到的通道数。可以是 0、1、2 或 3。0 表示使用图像文件中的通道数，1 表示灰度图，2 表示灰度图的 Alpha 通道，3 表示 RGB 图像。
	// 如果文件中包含 Alpha 通道，但 desired_channels 设置为 3，那么 Alpha 通道将被丢弃。可以为 NULL，如果不关心。
	// glGenTextures函数首先需要输入生成纹理的数量，然后把它们储存在第二个参数的unsigned int数组中
	GL_CHECK(glGenTextures(1, &textureId));
	// 激活纹理单元 纹理单元是状态无关的 是全局的
	GL_CHECK(glActiveTexture(GL_TEXTURE0));
	// 绑定纹理，让之后的任何纹理指令都对当前纹理生效
	GL_CHECK(glBindTexture(target, textureId));
	// 为当前绑定的纹理对象设置环绕、过滤方式
	GL_CHECK(glTexParameteri(target, GL_TEXTURE_WRAP_S, OpenGL::cvtWrap(samplerInfo.wrapS)));
	GL_CHECK(glTexParameteri(target, GL_TEXTURE_WRAP_T, OpenGL::cvtWrap(samplerInfo.wrapR)));
	GL_CHECK(glTexParameteri(target, GL_TEXTURE_MIN_FILTER, OpenGL::cvtFilter(samplerInfo.filterMin)));
	GL_CHECK(glTexParameteri(target, GL_TEXTURE_MAG_FILTER, OpenGL::cvtFilter(samplerInfo.filterMag)));

	auto textureData_ = getTextureData();
	if (textureData_.dataArray.empty()) {
		// 第一个参数指定了纹理目标(Target)。设置为GL_TEXTURE_2D意味着会生成与当前绑定的纹理对象在同一个目标上的纹理（任何绑定到GL_TEXTURE_1D和GL_TEXTURE_3D的纹理不会受到影响）。
		// 第二个参数为纹理指定多级渐远纹理的级别，如果你希望单独手动设置每个多级渐远纹理的级别的话。这里我们填0，也就是基本级别，也就是不手动设置
		// 第三个参数告诉OpenGL我们希望把纹理储存为何种格式。我们的图像只有RGB值，因此我们也把纹理储存为RGB值。
		// 第四个和第五个参数设置最终的纹理的宽度和高度。我们之前加载图像的时候储存了它们，所以我们使用对应的变量。
		// 下个参数应该总是被设为0（历史遗留的问题）。
		// 第七第八个参数定义了源图的格式和数据类型。我们使用RGB值加载这个图像，并把它们储存为char(byte)数组，我们将会传入对应值。
		// 最后一个参数是真正的图像数据。
		GL_CHECK(glTexImage2D(target, 0, openglTextureInfo.internalformat, textureInfo.width, textureInfo.height, 0, openglTextureInfo.format, openglTextureInfo.type, nullptr));
	}
	else {
		GL_CHECK(glTexImage2D(target, 0, openglTextureInfo.internalformat, textureInfo.width, textureInfo.height, 0, openglTextureInfo.format, openglTextureInfo.type, textureData_.dataArray[0]->rawData()));
	}
}

TextureOpenGL2D::~TextureOpenGL2D()
{
	GL_CHECK(glDeleteTextures(1, &textureId_));
}

TextureOpenGLCube::TextureOpenGLCube(const TextureInfo& texInfo, const SamplerInfo& smInfo): TextureOpenGL(texInfo, smInfo)
{
}

TextureOpenGLCube::TextureOpenGLCube(const TextureInfo& texInfo, const SamplerInfo& smInfo, const TextureData& texData): TextureOpenGL(texInfo, smInfo, texData)
{
}


TextureOpenGLCube::~TextureOpenGLCube()
{
}
