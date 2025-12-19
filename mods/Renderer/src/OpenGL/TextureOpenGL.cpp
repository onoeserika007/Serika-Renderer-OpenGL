#include "OpenGL/TextureOpenGL.h"
#include <glad/glad.h>

#include "Renderer.h"
#include "Material/Uniform.h"
#include "OpenGL/EnumsOpenGL.h"
#include "Utils/OpenGLUtils.h"

TextureOpenGL::TextureOpenGL(const TextureInfo& texInfo, const SamplerInfo& smInfo, const TextureData& texData): Texture(texInfo, smInfo, texData)
{
}

TextureOpenGL::~TextureOpenGL()
{
}

std::shared_ptr<UniformSampler> TextureOpenGL::getUniformSampler(const Renderer &renderer) const {
	if (!sampler_) {
		auto&& texInfo = getTextureInfo();
		sampler_ = renderer.createUniformSampler(texInfo);
	}

	// update colorbuffer in case it rebuilt.
	sampler_->setTexture(*this);
	return sampler_;
}




TextureOpenGL2D::TextureOpenGL2D(const TextureInfo& texInfo, const SamplerInfo& smInfo, const TextureData& texData): TextureOpenGL(texInfo, smInfo, texData)
{
	assert(texInfo.target == TextureTarget::TextureTarget_TEXTURE_2D);
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

	// set borader sampler in case savalue_ptr(mpvalue_ptrvalue_ptr((ling out of bound
	GL_CHECK(glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, glm::value_ptr(OpenGL::cvtBorderColor(samplerInfo.borderColor))));

	auto textureData_ = getTextureData();;
	if (!textureData_.unitDataArray.empty()) {
		// 第一个参数指定了纹理目标(Target)。设置为GL_TEXTURE_2D意味着会生成与当前绑定的纹理对象在同一个目标上的纹理（任何绑定到GL_TEXTURE_1D和GL_TEXTURE_3D的纹理不会受到影响）。
		// 第二个参数为纹理指定多级渐远纹理的级别，如果你希望单独手动设置每个多级渐远纹理的级别的话。这里我们填0，也就是基本级别，也就是不手动设置
		// 第三个参数告诉OpenGL我们希望把纹理储存为何种格式。我们的图像只有RGB值，因此我们也把纹理储存为RGB值。
		// 第四个和第五个参数设置最终的纹理的宽度和高度。我们之前加载图像的时候储存了它们，所以我们使用对应的变量。
		// 下个参数应该总是被设为0（历史遗留的问题）。
		// 第七第八个参数定义了源图的格式和数据类型。我们使用RGB值加载这个图像，并把它们储存为char(byte)数组，我们将会传入对应值。
		// 最后一个参数是真正的图像数据。
		GL_CHECK(glTexImage2D(target, 0, openglTextureInfo.internalformat, textureInfo.width, textureInfo.height, textureInfo.border, openglTextureInfo.format, openglTextureInfo.elemtype, textureData_.unitDataArray[0]->rawData()));
	}
	else if (!textureData_.floatDataArray.empty()) {
		GL_CHECK(glTexImage2D(target, 0, openglTextureInfo.internalformat, textureInfo.width, textureInfo.height, textureInfo.border, openglTextureInfo.format, openglTextureInfo.elemtype, textureData_.floatDataArray[0]->rawData()));
	}
	else {
		GL_CHECK(glTexImage2D(target, 0, openglTextureInfo.internalformat, textureInfo.width, textureInfo.height, textureInfo.border, openglTextureInfo.format, openglTextureInfo.elemtype, nullptr));
	}

	if (textureInfo.useMipmaps) {
		GL_CHECK(glGenerateMipmap(target));
	}
}

void TextureOpenGL2D::copyDataTo(Texture &other) {
	TextureOpenGL::copyDataTo(other);

	// const auto& srcTexInfo = OpenGL::cvtTextureFormat(static_cast<TextureFormat>(getTextureInfo().format));
	// const auto& dstTexInfo = OpenGL::cvtTextureFormat(static_cast<TextureFormat>(other.getTextureInfo().format));

	auto srcTarget = multiSample() ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D;
	auto dstTarget = other.multiSample() ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D;

	auto srcTextureId = getId();
	auto dstTextureId = other.getId();

	auto w = width();
	auto h = height();
	GL_CHECK(glCopyImageSubData(
	srcTextureId, srcTarget, 0, 0, 0, 0,
	dstTextureId, dstTarget, 0, 0, 0, 0,
	w, h, 1
	));
}

TextureOpenGL2D::~TextureOpenGL2D()
{
	GL_CHECK(glDeleteTextures(1, &textureId_));
}


TextureOpenGLCube::TextureOpenGLCube(const TextureInfo& texInfo, const SamplerInfo& smInfo, const TextureData& texData): TextureOpenGL(texInfo, smInfo, texData)
{
	assert(texInfo.target == TextureTarget::TextureTarget_TEXTURE_CUBE_MAP);
	setupPipeline();
}

void TextureOpenGLCube::setupPipeline() {
	GLuint& textureId = textureId_;
	auto& samplerInfo = getSamplerInfo();
	auto& textureInfo = getTextureInfo();
	const auto& openglTextureInfo = OpenGL::cvtTextureFormat(static_cast<TextureFormat>(textureInfo.format));
	auto target = OpenGL::cvtTextureTarget(static_cast<TextureTarget>(textureInfo.target));
	// glGenTextures函数首先需要输入生成纹理的数量，然后把它们储存在第二个参数的unsigned int数组中
	GL_CHECK(glGenTextures(1, &textureId));
	// 激活纹理单元 纹理单元是状态无关的 是全局的
	GL_CHECK(glActiveTexture(GL_TEXTURE0));
	// 绑定纹理，让之后的任何纹理指令都对当前纹理生效
	GL_CHECK(glBindTexture(target, textureId));
	// 为当前绑定的纹理对象设置环绕、过滤方式
	GL_CHECK(glTexParameteri(target, GL_TEXTURE_WRAP_S, OpenGL::cvtWrap(samplerInfo.wrapS)));
	GL_CHECK(glTexParameteri(target, GL_TEXTURE_WRAP_T, OpenGL::cvtWrap(samplerInfo.wrapT)));
	GL_CHECK(glTexParameteri(target, GL_TEXTURE_WRAP_R, OpenGL::cvtWrap(samplerInfo.wrapR)));
	GL_CHECK(glTexParameteri(target, GL_TEXTURE_MIN_FILTER, OpenGL::cvtFilter(samplerInfo.filterMin)));
	GL_CHECK(glTexParameteri(target, GL_TEXTURE_MAG_FILTER, OpenGL::cvtFilter(samplerInfo.filterMag)));

	// set borader sampler in case savalue_ptr(mpvalue_ptrvalue_ptr((ling out of bound
	GL_CHECK(glTexParameterfv(target, GL_TEXTURE_BORDER_COLOR, glm::value_ptr(OpenGL::cvtBorderColor(samplerInfo.borderColor))));

	auto textureData_ = getTextureData();
	if (textureData_.unitDataArray.empty()) {
		// for point light shadow map case
		for (int i = 0; i < 6; i++) {
			GL_CHECK(glTexImage2D(
					GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
					0,
					openglTextureInfo.internalformat,
					textureInfo.width, textureInfo.height,
					textureInfo.border, openglTextureInfo.format, openglTextureInfo.elemtype,
					nullptr));
		}
	}
	else {
		for (int i = 0; i < textureData_.unitDataArray.size(); i++) {
			GL_CHECK(glTexImage2D(
					GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
					0,
					openglTextureInfo.internalformat,
					textureInfo.width, textureInfo.height,
					textureInfo.border, openglTextureInfo.format, openglTextureInfo.elemtype,
					textureData_.unitDataArray[i]->rawData()));
		}
	}
}


TextureOpenGLCube::~TextureOpenGLCube()
{
	GL_CHECK(glDeleteTextures(1, &textureId_));
}


TextureOpenGLBuffer::TextureOpenGLBuffer(const TextureInfo &texInfo, const SamplerInfo &smInfo,
	const TextureData &texData): TextureOpenGL(texInfo, smInfo, texData) {
	assert(texInfo.target == TextureTarget::TextureTarget_TEXTURE_BUFFER);
	setupPipeline();
}

void TextureOpenGLBuffer::setupPipeline() {

	auto& textureInfo = getTextureInfo();
	const auto& openglTextureInfo = OpenGL::cvtTextureFormat(static_cast<TextureFormat>(textureInfo.format));
	auto target = OpenGL::cvtTextureTarget(static_cast<TextureTarget>(textureInfo.target));

	// tbo
	GLuint TBO;
	GL_CHECK(glGenBuffers(1, &TBO));
	GL_CHECK(glBindBuffer(target, TBO));

	auto&& textureData_ = getTextureData();
	if (!textureData_.bufferData.empty()) {
		GL_CHECK(glBufferData(GL_TEXTURE_BUFFER, textureData_.bufferData.size() * sizeof(float), textureData_.bufferData.data(), GL_STATIC_DRAW));
	}

	// tex id
	GLuint& tex = textureId_;
	assert(target == GL_TEXTURE_BUFFER);
	GL_CHECK(glGenTextures(1, &tex));
	GL_CHECK(glActiveTexture(GL_TEXTURE0));
	GL_CHECK(glBindTexture(target, tex));
	// 注意你放进来的数据就是float32，还想怎么样
	GL_CHECK(glTexBuffer(GL_TEXTURE_BUFFER, openglTextureInfo.internalformat, TBO));
}

TextureOpenGLBuffer::~TextureOpenGLBuffer() {
	GL_CHECK(glDeleteTextures(1, &textureId_));
	GL_CHECK(glDeleteBuffers(1, &TBO));
}
