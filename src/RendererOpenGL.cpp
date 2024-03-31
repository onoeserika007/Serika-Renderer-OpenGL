#include "RendererOpenGL.h"
#include "UniformOpenGL.h"
#include "Base/GLMInc.h"
#include "ShaderGLSL.h"
#include "glad/glad.h"
#include "BufferAttribute.h"
#include "Geometry.h"
#include "Texture.h"
#include "OpenGL/EnumsOpenGL.h"
#include "Object.h"
#include "Camera.h"
#include "FrameBufferOpenGL.h"

void RendererOpenGL::init()
{
	modelUniformBlock_ = createUniformBlock("Model", sizeof(ModelUniformBlock));
}

std::shared_ptr<UniformBlock> RendererOpenGL::createUniformBlock(const std::string& name, int size)
{
	return std::make_shared<UniformBlockOpenGL>(name, size);
}

std::shared_ptr<UniformSampler> RendererOpenGL::createUniformSampler(const std::string& name, TextureTarget target, TextureFormat format)
{
	return std::make_shared<UniformSamplerOpenGL>(name, target, format);
}

std::shared_ptr<Shader> RendererOpenGL::createShader(const std::string& vsPath, const std::string& fsPsth)
{
	return ShaderGLSL::loadShader(vsPath, fsPsth);
}

std::shared_ptr<Texture> RendererOpenGL::createTexture(const TextureInfo& texInfo, const SamplerInfo& smInfo)
{
	return std::make_shared<Texture>(texInfo, smInfo);
}

std::shared_ptr<FrameBuffer> RendererOpenGL::createFrameBuffer(bool offScreen)
{
	return std::make_shared<FrameBufferOpenGL>(offScreen);
}

void RendererOpenGL::setupVertexAttribute(BufferAttribute& vertexAttribute)
{
	GLuint VBO;
	glGenBuffers(1, &VBO);
	// OpenGL允许我们同时绑定多个缓冲，只要它们是不同的缓冲类型
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	// 把之前定义的顶点数据复制到缓冲的内存中
	// 我们希望显卡如何管理给定的数据
	// GL_STATIC_DRAW ：数据不会或几乎不会改变。
	// GL_DYNAMIC_DRAW：数据会被改变很多。
	// GL_STREAM_DRAW ：数据每次绘制时都会改变。
	// 比如说一个缓冲中的数据将频繁被改变，那么使用的类型就是GL_DYNAMIC_DRAW或GL_STREAM_DRAW，这样就能确保显卡把数据放在能够高速写入的内存部分。
	glBufferData(GL_ARRAY_BUFFER, vertexAttribute.byte_size(), vertexAttribute.data(), GL_STATIC_DRAW);
	vertexAttribute.setVBO(VBO);
}

void RendererOpenGL::setupGeometry(Geometry& geometry)
{
	// setup attributes
	for (const auto& attr : geometry.getAttributeNameList()) {
		auto& data = geometry.getBufferData(attr);
		data.setupPipeline(*this);
	}

	// setup indices
	if (geometry.isMeshIndexed()) {
		GLuint EBO = 0;
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, geometry.getIndicesNum() * sizeof(unsigned), geometry.getIndicesRawData(), GL_STATIC_DRAW);
		geometry.setEBO(EBO);
	}
}

void RendererOpenGL::setupTexture(Texture& texture)
{
	if (texture.ready()) {
		return;
	}

	GLuint textureId;
	auto& samplerInfo = texture.getSamplerInfo();
	auto& textureInfo = texture.getTextureInfo();
	const auto& openglTextureInfo = OpenGL::cvtTextureFormat(static_cast<TextureFormat>(textureInfo.format));
	// desired_channels：希望图像数据被加载到的通道数。可以是 0、1、2 或 3。0 表示使用图像文件中的通道数，1 表示灰度图，2 表示灰度图的 Alpha 通道，3 表示 RGB 图像。
	// 如果文件中包含 Alpha 通道，但 desired_channels 设置为 3，那么 Alpha 通道将被丢弃。可以为 NULL，如果不关心。
	// glGenTextures函数首先需要输入生成纹理的数量，然后把它们储存在第二个参数的unsigned int数组中
	glGenTextures(1, &textureId);
	// 激活纹理单元 纹理单元是状态无关的 是全局的
	glActiveTexture(GL_TEXTURE0);
	// 绑定纹理，让之后的任何纹理指令都对当前纹理生效
	glBindTexture(GL_TEXTURE_2D, textureId);
	// 为当前绑定的纹理对象设置环绕、过滤方式
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, OpenGL::cvtWrap(samplerInfo.wrapS));
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, OpenGL::cvtWrap(samplerInfo.wrapR));
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, OpenGL::cvtFilter(samplerInfo.filterMin));
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, OpenGL::cvtFilter(samplerInfo.filterMag));

	auto prawData = texture.getpRawData();
	if (!prawData) {
		// 第一个参数指定了纹理目标(Target)。设置为GL_TEXTURE_2D意味着会生成与当前绑定的纹理对象在同一个目标上的纹理（任何绑定到GL_TEXTURE_1D和GL_TEXTURE_3D的纹理不会受到影响）。
		// 第二个参数为纹理指定多级渐远纹理的级别，如果你希望单独手动设置每个多级渐远纹理的级别的话。这里我们填0，也就是基本级别，也就是不手动设置
		// 第三个参数告诉OpenGL我们希望把纹理储存为何种格式。我们的图像只有RGB值，因此我们也把纹理储存为RGB值。
		// 第四个和第五个参数设置最终的纹理的宽度和高度。我们之前加载图像的时候储存了它们，所以我们使用对应的变量。
		// 下个参数应该总是被设为0（历史遗留的问题）。
		// 第七第八个参数定义了源图的格式和数据类型。我们使用RGB值加载这个图像，并把它们储存为char(byte)数组，我们将会传入对应值。
		// 最后一个参数是真正的图像数据。
		glTexImage2D(GL_TEXTURE_2D, 0, openglTextureInfo.internalformat, textureInfo.width, textureInfo.height, 0, openglTextureInfo.format, openglTextureInfo.type, nullptr);
	}
	else {
		glTexImage2D(GL_TEXTURE_2D, 0, openglTextureInfo.internalformat, textureInfo.width, textureInfo.height, 0, openglTextureInfo.format, openglTextureInfo.type, prawData->dataArray[0]->rawData());
	}

	texture.setId(textureId);
}

void RendererOpenGL::loadShaders(Material& material)
{
	auto shadingMode = material.shadingMode();
	switch (shadingMode)
	{
	case Shading_Unknown:
		break;
	case Shading_BaseColor:
		break;
	case Shading_BlinnPhong: 
		material.setShader(ShaderPass::Shader_Shadow_Pass, ShaderGLSL::loadShadowMapShader());
		material.setShader(ShaderPass::Shader_MainRender_Pass, ShaderGLSL::loadStandardMaterialShader());
		break;
	case Shading_PBR:
		break;
	case Shading_Skybox:
		break;
	case Shading_IBL_Irradiance:
		break;
	case Shading_IBL_Prefilter:
		break;
	case Shading_FXAA:
		break;
	default:
		break;
	}
}

void RendererOpenGL::setupMaterial(Material& material)
{
	auto& textures = material.getTextures();
	for (auto& [type, texture] : textures) {
		texture->setupPipeline(*this);
		const auto& texInfo = texture->getTextureInfo();
		const auto& samplerName = Texture::samplerName(static_cast<TextureType>(type));
		auto sampler = std::make_shared<UniformSamplerOpenGL>(samplerName, texInfo.target, texInfo.format);
		material.setUniform(sampler->name(), sampler);
	}

	if (!material.shaderReady()) {
		loadShaders(material);
		for (auto& [pass, shader] : material.getShaders()) {
			shader->setupPipeline(material);
		}
	}
}

void RendererOpenGL::setupStandardMaterial(StandardMaterial& material)
{
	auto& textures = material.getTextures();
	for (auto& [type, texture] : textures) {
		texture->setupPipeline(*this);
		const auto& texInfo = texture->getTextureInfo();
		const auto& samplerName = Texture::samplerName(static_cast<TextureType>(type));
		auto sampler = std::make_shared<UniformSamplerOpenGL>(samplerName, texInfo.target, texInfo.format);
		material.setUniform(sampler->name(), sampler);
	}

	if (!material.shaderReady()) {
		loadShaders(material);
		for (auto& [pass, shader] : material.getShaders()) {
			shader->setupPipeline(material);
		}
	}
}

void RendererOpenGL::setupObject(Object& object)
{
	auto pmaterial = object.getpMaterial();
	// 暂时强制所有vert shader都按照一套attribute的layout
	std::shared_ptr<Shader> pshader = nullptr;
	if (pmaterial) {
		pshader = pmaterial->getShader(ShaderPass::Shader_Shadow_Pass);
	}

	auto pgeometry = object.getpGeometry();
	if (pgeometry) {
		GLuint VAO;
		glGenVertexArrays(1, &VAO);
		glBindVertexArray(VAO);

		// attrs
		const auto& attrList = pgeometry->getAttributeNameList();
		for (const auto& attr : attrList) {
			auto loc = pshader->getAttributeLocation(attr);
			if (loc != -1) {
				//const auto& VBO = pgeometry_->getAttributeBuffer(attr);
				const auto& bufferData = pgeometry->getBufferData(attr);
				const auto& VBO = bufferData.getVBO();
				glBindBuffer(GL_ARRAY_BUFFER, VBO);
				// 指定顶点属性的解释方式（如何解释VBO中的数据）
				// 1. glVertexAttribPointer
				// attri的Location(layout location = 0) | item_size | 数据类型 | 是否Normalize to 0-1 | stride | 从Buffer起始位置开始的偏移
				glVertexAttribPointer(loc, bufferData.item_size(), GL_FLOAT, GL_FALSE, bufferData.item_size() * sizeof(float), (void*)0);
				// 以顶点属性位置值作为参数，启用顶点属性；顶点属性默认是禁用的
				glEnableVertexAttribArray(loc);
			}
		}

		// indices
		if (pgeometry->isMeshIndexed()) {
			auto EBO = pgeometry->getEBO();
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		}

		// set back
		object.setVAO(VAO);
		glBindVertexArray(NULL);
	}
}

void RendererOpenGL::useMaterial(Material& material)
{
}

void RendererOpenGL::useMaterial(Material& material, ShaderPass pass)
{
}

void RendererOpenGL::drawObject(Object& object, ShaderPass pass)
{
	object.updateFrame(*this);
	updateModelUniformBlock(object, *camera_, false);

	auto VAO = object.getVAO();
	auto pgeometry = object.getpGeometry();
	glBindVertexArray(VAO);
	if (pgeometry->isMesh()) {
		// primitive | 顶点数组起始索引 | 绘制indices数量
		glDrawArrays(GL_TRIANGLES, 0, pgeometry->getVeticesNum());
	}
	else {
		// primitive | nums | 索引类型 | 最后一个参数里我们可以指定EBO中的偏移量（或者传递一个索引数组，但是这是当你不在使用EBO的时候），但是我们会在这里填写0。
		glDrawElements(GL_TRIANGLES, pgeometry->getIndicesNum(), GL_UNSIGNED_INT, 0);
	}
	glBindVertexArray(NULL);
}

void RendererOpenGL::updateModelUniformBlock(Object& object, Camera& camera, bool shadowPass)
{
	ModelUniformBlock tmp{};
	tmp.uModel = object.getModelMatrix();
	tmp.uNormalToWorld = object.getNormalToWorld();
	tmp.uProjection = camera.GetProjectionMatrix();
	tmp.uView = camera.GetViewMatrix();
	tmp.uViewPos = camera.position();
	if (shadowPass) {
		// ?????
		tmp.uShadowMapMVP = tmp.uProjection * tmp.uView * tmp.uModel; 
	}

	modelUniformBlock_->setData(&tmp, sizeof(ModelUniformBlock));

	auto pmat = object.getpMaterial();
	pmat->setUniform(modelUniformBlock_->name(), modelUniformBlock_);
}

void RendererOpenGL::beginRenderPass(std::shared_ptr<FrameBuffer>& frameBuffer, const ClearStates& states)
{
	auto* fbo = dynamic_cast<FrameBufferOpenGL*>(frameBuffer.get());
	fbo->bind();

	GLbitfield clearBit = 0;
	if (states.colorFlag) {
		GL_CHECK(glClearColor(states.clearColor.r, states.clearColor.g, states.clearColor.b, states.clearColor.a));
		clearBit |= GL_COLOR_BUFFER_BIT;
	}
	if (states.depthFlag) {
		clearBit |= GL_DEPTH_BUFFER_BIT;
	}
	GL_CHECK(glClear(clearBit));
}

void RendererOpenGL::endRenderPass()
{
	// reset gl states
	GL_CHECK(glDisable(GL_BLEND));
	GL_CHECK(glDisable(GL_DEPTH_TEST));
	GL_CHECK(glDepthMask(true));
	GL_CHECK(glDisable(GL_CULL_FACE));
	GL_CHECK(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL));
}

void RendererOpenGL::setViewPort(int x, int y, int width, int height)
{
	GL_CHECK(glViewport(x, y, width, height));
}

void RendererOpenGL::waitIdle()
{
	GL_CHECK(glFinish());
}

RendererOpenGL::~RendererOpenGL()
{
}

void RendererOpenGL::clearTexture(Texture& texture)
{
}
