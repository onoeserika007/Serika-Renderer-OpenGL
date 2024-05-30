#include "RendererOpenGL.h"
#include "UniformOpenGL.h"
#include "Base/GLMInc.h"
#include "ShaderGLSL.h"
#include "glad/glad.h"
#include "BufferAttribute.h"
#include "Geometry.h"
#include "TextureOpenGL.h"
#include "OpenGL/EnumsOpenGL.h"
#include "Object.h"
#include "Camera.h"
#include "FrameBufferOpenGL.h"
#include "Utils/Logger.h"
#include "Utils/OpenGLUtils.h"
#include <GLFW/glfw3.h>

RendererOpenGL::RendererOpenGL(Camera& camera): Renderer(camera)
{
	;
}

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

std::shared_ptr<UniformSampler> RendererOpenGL::createUniformSampler(const TextureInfo& texInfo)
{
	return std::make_shared<UniformSamplerOpenGL>(texInfo);
}

std::shared_ptr<Shader> RendererOpenGL::createShader(const std::string& vsPath, const std::string& fsPsth)
{
	return ShaderGLSL::loadShader(vsPath, fsPsth);
}

std::shared_ptr<Texture> RendererOpenGL::createTexture(const TextureInfo& texInfo, const SamplerInfo& smInfo, const TextureData& texData)
{
	if (texInfo.target == TextureTarget::TextureTarget_2D) {
		return std::make_shared<TextureOpenGL2D>(texInfo, smInfo, texData);
	}
	else if(texInfo.target == TextureTarget::TextureTarget_CUBE){
		return std::make_shared<TextureOpenGLCube>(texInfo, smInfo, texData);
	}
	return nullptr;
}

std::shared_ptr<FrameBuffer> RendererOpenGL::createFrameBuffer(bool offScreen)
{
	return std::make_shared<FrameBufferOpenGL>(offScreen);
}

void RendererOpenGL::setupVertexAttribute(BufferAttribute& vertexAttribute)
{
	GLuint VBO;
	GL_CHECK(glGenBuffers(1, &VBO));
	// OpenGL允许我们同时绑定多个缓冲，只要它们是不同的缓冲类型
	GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, VBO));
	// 把之前定义的顶点数据复制到缓冲的内存中
	// 我们希望显卡如何管理给定的数据
	// GL_STATIC_DRAW ：数据不会或几乎不会改变。
	// GL_DYNAMIC_DRAW：数据会被改变很多。
	// GL_STREAM_DRAW ：数据每次绘制时都会改变。
	// 比如说一个缓冲中的数据将频繁被改变，那么使用的类型就是GL_DYNAMIC_DRAW或GL_STREAM_DRAW，这样就能确保显卡把数据放在能够高速写入的内存部分。
	GL_CHECK(glBufferData(GL_ARRAY_BUFFER, vertexAttribute.byte_size(), vertexAttribute.data(), GL_STATIC_DRAW));
	vertexAttribute.setVBO(VBO);
}

void RendererOpenGL::setupGeometry(Geometry& geometry)
{
	if (geometry.isPipelineReady()) {
		return;
	}

	// setup attributes
	for (const auto& attr : geometry.getAttributeNameList()) {
		auto& data = geometry.getBufferData(attr);
		data.setupPipeline(*this);
	}

	// setup indices
	if (geometry.isMeshIndexed()) {
		GLuint EBO = 0;
		GL_CHECK(glGenBuffers(1, &EBO));
		GL_CHECK(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO));
		GL_CHECK(glBufferData(GL_ELEMENT_ARRAY_BUFFER, geometry.getIndicesNum() * sizeof(unsigned), geometry.getIndicesRawData(), GL_STATIC_DRAW));
		geometry.setEBO(EBO);
	}

	geometry.setPipelineReady(true);
}

void RendererOpenGL::loadShaders(Material& material)
{
	auto shadingMode = material.shadingMode();
	switch (shadingMode)
	{
	case Shading_Unknown:
		break;
	case Shading_BaseColor:
		material.setShader(ShaderPass::Shader_Plain_Pass, ShaderGLSL::loadPlainPassShader());
		break;
	case Shading_BlinnPhong: 
		material.setShader(ShaderPass::Shader_Shadow_Pass, ShaderGLSL::loadShadowMapShader());
		material.setShader(ShaderPass::Shader_Plain_Pass, ShaderGLSL::loadPlainPassShader());
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
	// textures
	if (!material.texturesReady()) {

		material.clearTextures();
		for (auto& [type, textureData] : material.getTextureData()) {
			TextureInfo texInfo{};
			texInfo.width = textureData.dataArray[0]->width();
			texInfo.height = textureData.dataArray[0]->height();
			texInfo.target = TextureTarget::TextureTarget_2D;
			texInfo.format = TextureFormat::TextureFormat_RGBA8;
			texInfo.usage = TextureUsage_AttachmentColor | TextureUsage_RendererOutput;
			texInfo.multiSample = false;;
			texInfo.useMipmaps = false;

			SamplerInfo smInfo{};
			smInfo.filterMag = Filter_LINEAR;
			smInfo.filterMin = Filter_LINEAR;

			auto texture = createTexture(texInfo, smInfo, textureData);
			material.addTexture(texture);

			const auto& samplerName = Texture::samplerName(static_cast<TextureType>(type));
			auto sampler = std::make_shared<UniformSamplerOpenGL>(samplerName, (TextureTarget)texInfo.target, (TextureFormat)texInfo.format);
			sampler->setTexture(texture);
			material.setUniform(sampler->name(), sampler);
		}

		material.setTexturesReady(true);
	}

	// shaders
	if (!material.shaderReady()) {
		loadShaders(material); 
		for (auto& [pass, shader] : material.getShaders()) {
			shader->setupPipeline(material);
		}
		material.setShaderReady(true);
	}
}

void RendererOpenGL::setupObject(Object& object, ShaderPass shaderPass)
{

	auto pmaterial = object.getpMaterial();
	if (auto pmaterial = object.getpMaterial()) {

		// set shading mode
		pmaterial->setShadingMode(object.getShadingMode());
		pmaterial->setupPipeline(*this);

		if (auto pgeometry = object.getpGeometry()) {

			pgeometry->setupPipeline(*this);

			// if VBO ready
			if (object.isPipelineReady()) {
				return;
			}

			GLuint VAO;
			GL_CHECK(glGenVertexArrays(1, &VAO));
			GL_CHECK(glBindVertexArray(VAO));

			// attrs
			const auto& attrList = pgeometry->getAttributeNameList();
			for (const auto& attr : attrList) {
				if (auto pshader = pmaterial->getShader(shaderPass)) {
					auto loc = pshader->getAttributeLocation(attr);
					if (loc != -1) {
						//const auto& VBO = pgeometry_->getAttributeBuffer(attr);
						const auto& bufferData = pgeometry->getBufferData(attr);
						const auto& VBO = bufferData.getVBO();
						GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, VBO));
						// 指定顶点属性的解释方式（如何解释VBO中的数据）
						// 1. glVertexAttribPointer
						// attri的Location(layout location = 0) | item_size | 数据类型 | 是否Normalize to 0-1 | stride | 从Buffer起始位置开始的偏移
						GL_CHECK(glVertexAttribPointer(loc, bufferData.item_size(), GL_FLOAT, GL_FALSE, bufferData.item_size() * sizeof(float), (void*)0));
						// 以顶点属性位置值作为参数，启用顶点属性；顶点属性默认是禁用的
						GL_CHECK(glEnableVertexAttribArray(loc));
					}
				}
			}

			// indices
			if (pgeometry->isMeshIndexed()) {
				auto EBO = pgeometry->getEBO();
				GL_CHECK(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO));
			}

			// set back
			object.setVAO(VAO);
			GL_CHECK(glBindVertexArray(0));

			object.setPipelineReady(true);
		} // geometry
	} // material
}

void RendererOpenGL::useMaterial(Material& material)
{
}

void RendererOpenGL::useMaterial(Material& material, ShaderPass pass)
{
}

void RendererOpenGL::drawObject(Object& object, ShaderPass pass)
{
	setupObject(object, pass);
	GL_CHECK(;);
	object.updateFrame(*this);
	updateModelUniformBlock(object, camera_, false);
	auto pmaterial = object.getpMaterial();
	pmaterial->use(pass);

	auto VAO = object.getVAO();
	auto pgeometry = object.getpGeometry();
	GL_CHECK(glBindVertexArray(VAO));
	if (pgeometry->isMesh()) {
		// primitive | 顶点数组起始索引 | 绘制indices数量
		GL_CHECK(glDrawArrays(GL_TRIANGLES, 0, pgeometry->getVeticesNum()));
	}
	else {
		// primitive | nums | 索引类型 | 最后一个参数里我们可以指定EBO中的偏移量（或者传递一个索引数组，但是这是当你不在使用EBO的时候），但是我们会在这里填写0。
		GL_CHECK(glDrawElements(GL_TRIANGLES, pgeometry->getIndicesNum(), GL_UNSIGNED_INT, 0));
	}
	GL_CHECK(glBindVertexArray(0));
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

void RendererOpenGL::beginRenderPass(std::shared_ptr<FrameBuffer> frameBuffer, const ClearStates& states)
{
	if (frameBuffer) {
		frameBuffer->bind();
	}
	else {
		// otherwise bind to main
		GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, 0));
	}

	GLbitfield clearBit = 0;
	if (states.colorFlag) {
		GL_CHECK(glClearColor(states.clearColor.r, states.clearColor.g, states.clearColor.b, states.clearColor.a));
		clearBit |= GL_COLOR_BUFFER_BIT;
	}
	if (states.depthFlag) {
		glEnable(GL_DEPTH_TEST);
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
	width_ = width;
	height_ = height;
	GL_CHECK(glViewport(x, y, width, height));
}

void RendererOpenGL::waitIdle()
{
	GL_CHECK(glFinish());
}

void RendererOpenGL::renderToScreen(UniformSampler& outTex, int screen_width, int screen_height)
{
	outTex.setName("uTexture");

	static const char* VS = R"(
	layout (location = 0) in vec3 aPos;
	layout (location = 1) in vec2 aTexCoord;

	out vec2 TexCoord;

	void main()
	{
		gl_Position = vec4(aPos, 1.0);
		TexCoord = vec2(aTexCoord.x, aTexCoord.y);
	}
	)";

	static const char* FS = R"(
	in vec2 TexCoord;
	out vec4 FragColor;

	uniform sampler2D uTexture;

	void main()
	{
		FragColor = texture(uTexture, TexCoord);
	}
	)";

	// set up vertex data (and buffer(s)) and configure vertex attributes
	static float vertices[] = {
		// positions | texture coords
		1.f, 1.f, 0.0f, 1.0f, 1.0f,   // top right
		1.f, -1.f, 0.0f, 1.0f, 0.0f,  // bottom right
		-1.f, -1.f, 0.0f, 0.0f, 0.0f, // bottom left
		-1.f, 1.f, 0.0f, 0.0f, 1.0f   // top left
	};
	static unsigned int indices[] = {
		0, 1, 3, // first triangle
		1, 2, 3  // second triangle
	};

	static GLuint VAO, VBO, EBO;
	static unsigned int texture;
	static std::shared_ptr<ShaderGLSL> program;

	if (!program) {
		program = ShaderGLSL::loadFromRawSource(VS, FS);
		program->compileAndLink();
	}

	if (0 == VAO) {
		GL_CHECK(glGenVertexArrays(1, &VAO));
		GL_CHECK(glBindVertexArray(VAO));

		if (VBO == 0) {
			GL_CHECK(glGenBuffers(1, &VBO));
			GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, VBO));
			GL_CHECK(glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW));

			GL_CHECK(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), nullptr));
			GL_CHECK(glEnableVertexAttribArray(0));
			GL_CHECK(glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float))));
			GL_CHECK(glEnableVertexAttribArray(1));
		}

		if (EBO == 0) {
			GL_CHECK(glGenBuffers(1, &EBO));
			GL_CHECK(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO));
			GL_CHECK(glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW));
		}

		GL_CHECK(glBindVertexArray(GL_NONE));
	}

	//if (texture == 0) {
	//	glGenTextures(1, &texture);
	//	glActiveTexture(GL_TEXTURE0);
	//	glBindTexture(GL_TEXTURE_2D, texture);
	//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	//	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, screen_width, screen_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

	//	program->use();
	//	glUniform1i(glGetUniformLocation(program->getId(), "uTexture"), 0);
	//}
	//else {
	//	glActiveTexture(GL_TEXTURE0);
	//	glBindTexture(GL_TEXTURE_2D, texture);
	//	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, screen_width, screen_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
	//	program->use();
	//	glUniform1i(glGetUniformLocation(program->getId(), "uTexture"), 0);
	//}

	if (VAO && EBO && VBO && program) {
		// real frame buffer size

		GL_CHECK(glDisable(GL_BLEND));
		GL_CHECK(glDisable(GL_DEPTH_TEST));
		GL_CHECK(glDepthMask(true));
		GL_CHECK(glDisable(GL_CULL_FACE));
		GL_CHECK(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL));

		GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, 0));
		GL_CHECK(glViewport(0, 0, screen_width, screen_height));

		GL_CHECK(glClearColor(0.f, 0.f, 0.f, 0.0f));
		GL_CHECK(glClear(GL_COLOR_BUFFER_BIT));

		
		//  name of outTex here should be corresponding to that in shader.
		program->bindUniform(outTex);
		// 
		//GL_CHECK(glUniform1i(glGetUniformLocation(program->getId(), "uTexture"), 0));

		GL_CHECK(glBindVertexArray(VAO));
		GL_CHECK(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr));
	}
	else {
		LOGE("Failed render to screen!");
	}
}

RendererOpenGL::~RendererOpenGL()
{
}

void RendererOpenGL::clearTexture(Texture& texture)
{
}
