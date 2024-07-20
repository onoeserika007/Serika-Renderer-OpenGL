#include "RendererOpenGL.h"
#include "UniformOpenGL.h"
#include "Base/GLMInc.h"
#include "ShaderGLSL.h"
#include "glad/glad.h"
#include "BufferAttribute.h"
#include "Geometry/Geometry.h"
#include "TextureOpenGL.h"
#include "OpenGL/EnumsOpenGL.h"
#include "../include/Geometry/Object.h"
#include "Camera.h"
#include "FrameBufferOpenGL.h"
#include "Utils/Logger.h"
#include "Utils/OpenGLUtils.h"
#include "Light.h"
#include "Geometry/Mesh.h"
#include <GLFW/glfw3.h>

// set up vertex data (and buffer(s)) and configure vertex attributes
constexpr float ToScreenRectangleVertices[] = {
	// positions | texture coords
	1.f, 1.f, 0.0f, 1.0f, 1.0f,   // top right
	1.f, -1.f, 0.0f, 1.0f, 0.0f,  // bottom right
	-1.f, -1.f, 0.0f, 0.0f, 0.0f, // bottom left
	-1.f, 1.f, 0.0f, 0.0f, 1.0f   // top left
};
constexpr unsigned int ToScreenRectangleIndices[] = {
	0, 1, 3, // first triangle
	1, 2, 3  // second triangle
};

RendererOpenGL::RendererOpenGL(const std::shared_ptr<Camera>& camera): Renderer(camera)
{
	;
}

void RendererOpenGL::init()
{
	// The name in shader
	modelUniformBlock_ = createUniformBlock("Model", sizeof(ModelUniformBlock));
	lightUniformBlock_ = createUniformBlock("Light", sizeof(LightDataUniformBlock));
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

// Each kind of shading is consist of multiple renderpass.
void RendererOpenGL::loadShaders(Material& material)
{
	auto shadingMode = material.shadingMode();
	switch (shadingMode)
	{
	case Shading_Unknown:
		break;
	case Shading_BaseColor:
		material.setShader(ShaderPass::Shader_Plain_Pass, ShaderGLSL::loadPlainPassShader());
		material.setShader(ShaderPass::Shader_Geometry_Pass, ShaderGLSL::loadGeometryShader());
		material.setShader(ShaderPass::Shader_Shadow_Pass, ShaderGLSL::loadShadowPassShader());
		break;
	case Shading_BlinnPhong:
		material.setShader(ShaderPass::Shader_Plain_Pass, ShaderGLSL::loadPlainPassShader());
		material.setShader(ShaderPass::Shader_Geometry_Pass, ShaderGLSL::loadGeometryShader());
		material.setShader(ShaderPass::Shader_Shadow_Pass, ShaderGLSL::loadShadowPassShader());
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
			texInfo.usage = TextureUsage_Sampler;
			texInfo.multiSample = false;;
			texInfo.useMipmaps = false;

			SamplerInfo smInfo{};
			smInfo.filterMag = Filter_LINEAR;
			smInfo.filterMin = Filter_LINEAR;

			auto texture = createTexture(texInfo, smInfo, textureData);
			material.addTexture(texture);

			const auto& samplerName = Texture::samplerName(static_cast<TextureType>(type));
			auto sampler = std::make_shared<UniformSamplerOpenGL>(samplerName, (TextureTarget)texInfo.target, (TextureFormat)texInfo.format);
			sampler->setTexture(*texture);
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

void RendererOpenGL::setupMesh(UMesh &mesh, ShaderPass shaderPass)
{

	auto pmaterial = mesh.getpMaterial();
	if (auto pmaterial = mesh.getpMaterial()) {

		// set shading mode
		pmaterial->setShadingMode(mesh.getShadingMode());
		pmaterial->setupPipeline(*this);

		if (auto pgeometry = mesh.getpGeometry()) {

			pgeometry->setupPipeline(*this);

			// if VBO ready
			if (mesh.isPipelineReady()) {
				return;
			}

			GLuint VAO;
			GL_CHECK(glGenVertexArrays(1, &VAO));
			GL_CHECK(glBindVertexArray(VAO));

			// attrs
			const auto& attrList = pgeometry->getAttributeNameList();
			for (const auto& attr : attrList) {
				// uniform updated in Material::ues(ShaderPass)
				pmaterial->use(shaderPass);
				// shaderPass used here.
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
			mesh.setVAO(VAO);
			GL_CHECK(glBindVertexArray(0));

			mesh.setPipelineReady(true);
		} // geometry
	} // material
}

void RendererOpenGL::useMaterial(Material& material)
{
}

void RendererOpenGL::useMaterial(Material& material, ShaderPass pass)
{
}

void RendererOpenGL::draw(UMesh &mesh, const ShaderPass pass, const std::shared_ptr<Camera>& shadowCamera)
{
	if (mesh.drawable()) {
		setupMesh(mesh, pass);
		mesh.updateFrame(*this);

		updateModelUniformBlock(mesh, *mainCamera_, shadowCamera);
		loadUniformBlocks(mesh);

		auto pmaterial = mesh.getpMaterial();
		pmaterial->use(pass);

		auto VAO = mesh.getVAO();
		auto pgeometry = mesh.getpGeometry();
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

	for (auto&& child: mesh.getChildren()) {
		if (auto* childMesh = dynamic_cast<UMesh*>(child.get())) {
			draw(*childMesh, pass, shadowCamera);
		}
	}

}

#define GL_STATE_SET(var, gl_state) if (var) GL_CHECK(glEnable(gl_state)); else GL_CHECK(glDisable(gl_state));

void RendererOpenGL::updateRenderStates(RenderStates &renderStates) {
	renderStates_ = std::make_shared<RenderStates>(renderStates);

	// blend
	GL_STATE_SET(renderStates.blend, GL_BLEND)
	GL_CHECK(glBlendEquationSeparate(OpenGL::cvtBlendFunction(renderStates.blendParams.blendFuncRgb),
									 OpenGL::cvtBlendFunction(renderStates.blendParams.blendFuncAlpha)));
	GL_CHECK(glBlendFuncSeparate(OpenGL::cvtBlendFactor(renderStates.blendParams.blendSrcRgb),
								 OpenGL::cvtBlendFactor(renderStates.blendParams.blendDstRgb),
								 OpenGL::cvtBlendFactor(renderStates.blendParams.blendSrcAlpha),
								 OpenGL::cvtBlendFactor(renderStates.blendParams.blendDstAlpha)));

	// depth
	GL_STATE_SET(renderStates.depthTest, GL_DEPTH_TEST);
	GL_CHECK(glDepthMask(renderStates.depthMask));
	GL_CHECK(glDepthFunc(OpenGL::cvtDepthFunc(renderStates.depthFunc)));

	GL_STATE_SET(renderStates.cullFace, GL_CULL_FACE);
	GL_CHECK(glPolygonMode(GL_FRONT_AND_BACK, OpenGL::cvtPolygonMode(renderStates.polygonMode)));

	GL_CHECK(glLineWidth(renderStates.lineWidth));
	GL_CHECK(glEnable(GL_PROGRAM_POINT_SIZE));

}

RenderStates RendererOpenGL::getRenderStates() {
	RenderStates ret;
	if (renderStates_) {
		ret = *renderStates_;
	}
	return ret;
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
	GL_CHECK(glDepthMask(false));
	GL_CHECK(glDisable(GL_CULL_FACE));
	GL_CHECK(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL));
}

void RendererOpenGL::setRenderViewPort(int x, int y, int width, int height) {
	width_ = width;
	height_ = height;
	GL_CHECK(glViewport(x, y, width, height));
}

void RendererOpenGL::waitIdle()
{
	GL_CHECK(glFinish());
}

void RendererOpenGL::renderToScreen(UniformSampler& outTex, int screen_width, int screen_height, bool bFromColor)
{
	outTex.setName("uTexture");

	static GLuint VAO = 0;
	static GLuint VBO = 0;
	static GLuint EBO = 0;

	std::shared_ptr<ShaderGLSL> program;
	if (bFromColor) {
		program = getToScreenColorProgram(outTex);
	}
	else {
		program = getToScreenDepthProgram(outTex);
	}

	if (0 == VAO) {
		GL_CHECK(glGenVertexArrays(1, &VAO));
		GL_CHECK(glBindVertexArray(VAO));

		if (VBO == 0) {
			GL_CHECK(glGenBuffers(1, &VBO));
			GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, VBO));
			GL_CHECK(glBufferData(GL_ARRAY_BUFFER, sizeof(ToScreenRectangleVertices), ToScreenRectangleVertices, GL_STATIC_DRAW));

			GL_CHECK(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), nullptr));
			GL_CHECK(glEnableVertexAttribArray(0));
			GL_CHECK(glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float))));
			GL_CHECK(glEnableVertexAttribArray(1));
		}

		if (EBO == 0) {
			GL_CHECK(glGenBuffers(1, &EBO));
			GL_CHECK(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO));
			GL_CHECK(glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(ToScreenRectangleIndices), ToScreenRectangleIndices, GL_STATIC_DRAW));
		}

		GL_CHECK(glBindVertexArray(GL_NONE));
	}

	if (VAO && EBO && VBO ) {
		GL_CHECK(glDisable(GL_BLEND));
		GL_CHECK(glDisable(GL_DEPTH_TEST));
		GL_CHECK(glDepthMask(true));
		GL_CHECK(glDisable(GL_CULL_FACE));
		GL_CHECK(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL));

		GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, 0));
		GL_CHECK(glViewport(0, 0, screen_width, screen_height));

		GL_CHECK(glClearColor(0.f, 0.f, 0.f, 0.0f));
		GL_CHECK(glClear(GL_COLOR_BUFFER_BIT));

		// name of outTex here should be corresponding to that in shader.
		program->use();
		program->bindUniform(outTex);

		GL_CHECK(glBindVertexArray(VAO));
		GL_CHECK(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr));
	}
	else {
		LOGE("Failed render to screen!");
	}
}

std::shared_ptr<ShaderGLSL> RendererOpenGL::getToScreenColorProgram(UniformSampler& outTex) {
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

	static std::shared_ptr<ShaderGLSL> program;

	if (!program) {
		program = ShaderGLSL::loadFromRawSource(VS, FS);
		program->compileAndLink();
	}

	return program;
}

std::shared_ptr<ShaderGLSL> RendererOpenGL::getToScreenDepthProgram(UniformSampler& outTex) {
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
		float Depth = texture(uTexture, TexCoord).x;
		FragColor = vec4(vec3(Depth), 1.f);
	}
	)";

	static std::shared_ptr<ShaderGLSL> program;

	if (!program) {
		program = ShaderGLSL::loadFromRawSource(VS, FS);
		program->compileAndLink();
	}

	return program;
}

RendererOpenGL::~RendererOpenGL()
{
}

void RendererOpenGL::clearTexture(Texture& texture)
{
}


