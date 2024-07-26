#include "../../include/OpenGL/RendererOpenGL.h"
#include "../../include/OpenGL/UniformOpenGL.h"
#include "../../include/Base/Globals.h"
#include "../../include/OpenGL/ShaderGLSL.h"
#include "../../ThirdParty/glad/include/glad/glad.h"
#include "../../include/Geometry/BufferAttribute.h"
#include "../../include/Geometry/Geometry.h"
#include "../../include/OpenGL/TextureOpenGL.h"
#include "../../include/OpenGL/EnumsOpenGL.h"
#include "../../include/Geometry/Object.h"
#include "../../include/FCamera.h"
#include "../../include/OpenGL/FrameBufferOpenGL.h"
#include "../../include/Utils/Logger.h"
#include "../../include/Utils/OpenGLUtils.h"
#include "../../include/Light.h"
#include "../../include/Geometry/UMesh.h"
#include <../../ThirdParty/glfw/include/GLFW/glfw3.h>

#include "../../include/RenderPass/RenderPassGeometry.h"
#include "Material/FMaterial.h"

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

RendererOpenGL::RendererOpenGL(const std::shared_ptr<FCamera>& camera): Renderer(camera)
{
	// identify renderer type
	renderer_type_ = ERendererType::RendererType_OPENGL;
}

void RendererOpenGL::init()
{
	// The name in shader
	modelUniformBlock_ = createUniformBlock("Model", sizeof(ModelUniformBlock));
	shadowUniformBlock_ = createUniformBlock("ShadowCube", sizeof(ShadowCubeUniformBlock));
	lightUniformBlock_ = createUniformBlock("Light", sizeof(LightDataUniformBlock));

	// place holder
	setupShadowMapBuffer(shadowMapPlaceholder_, 1, 1, false, false, false);
	setupShadowMapBuffer(shadowMapCubePlaceholder_, 1, 1, false, true, false);
	// in case uniform don't have a texture binding to it.
	shadowMapUniformSampler_ = shadowMapPlaceholder_->getUniformSampler(*this);
	shadowMapCubeUniformSampler_ = shadowMapCubePlaceholder_->getUniformSampler(*this);
}

std::shared_ptr<UniformBlock> RendererOpenGL::createUniformBlock(const std::string& name, int size) const {
	return std::make_shared<UniformBlockOpenGL>(name, size);
}

std::shared_ptr<UniformSampler> RendererOpenGL::createUniformSampler(const std::string& name, TextureTarget target, TextureFormat format) const {
	return std::make_shared<UniformSamplerOpenGL>(name, target, format);
}

std::shared_ptr<UniformSampler> RendererOpenGL::createUniformSampler(const TextureInfo& texInfo) const {
	return std::make_shared<UniformSamplerOpenGL>(texInfo);
}

std::shared_ptr<Shader> RendererOpenGL::createShader(const std::string& vsPath, const std::string& fsPsth)
{
	return ShaderGLSL::loadShader(vsPath, fsPsth);
}

std::shared_ptr<Texture> RendererOpenGL::createTexture(const TextureInfo& texInfo, const SamplerInfo& smInfo, const TextureData& texData) const {
	if (texInfo.target == TextureTarget::TextureTarget_TEXTURE_2D) {
		return std::make_shared<TextureOpenGL2D>(texInfo, smInfo, texData);
	}
	else if(texInfo.target == TextureTarget::TextureTarget_TEXTURE_CUBE_MAP){
		return std::make_shared<TextureOpenGLCube>(texInfo, smInfo, texData);
	}
	return nullptr;
}

std::shared_ptr<FrameBuffer> RendererOpenGL::createFrameBuffer(bool offScreen)
{
	return std::make_shared<FrameBufferOpenGL>(offScreen);
}

void RendererOpenGL::setupColorBuffer(std::shared_ptr<Texture> &outBuffer, int width, int height, bool force, bool bCubeMap, TextureTarget texTarget, TextureFormat
                                      texFormat) const {
	if (outBuffer) {
		const TextureInfo& texInfo = outBuffer->getTextureInfo();
		force = force || texInfo.width != width || texInfo.height != height
				|| texInfo.target != texTarget
				|| texInfo.format != texFormat;
	}

	bool bMultisample = texTarget == TextureTarget_TEXTURE_2D_MULTISAMPLE;
	if (!outBuffer || outBuffer->multiSample() != bMultisample || force) {
		TextureInfo texInfo{};
		texInfo.width = width;
		texInfo.height = height;
		texInfo.target = texTarget;
		texInfo.format = texFormat;
		texInfo.usage = TextureUsage_AttachmentColor | TextureUsage_RendererOutput;
		texInfo.multiSample = bMultisample;
		texInfo.useMipmaps = false;

		SamplerInfo smInfo{};
		smInfo.filterMag = Filter_LINEAR;
		smInfo.filterMin = Filter_LINEAR;

		outBuffer = createTexture(texInfo, smInfo, {});
	}
}

void RendererOpenGL::setupDepthBuffer(std::shared_ptr<Texture> &outBuffer, bool multiSample, bool force) const {
	TextureTarget target = TextureTarget_TEXTURE_2D;
	if (outBuffer) {
		const TextureInfo& texInfo = outBuffer->getTextureInfo();
		force = force || texInfo.width != width() || texInfo.height != height();
		force = force || texInfo.target != target || texInfo.format != TextureFormat_FLOAT32;
	}

	if (!outBuffer || outBuffer->multiSample() != multiSample || force) {
		TextureInfo texInfo{};
		texInfo.width = width();
		texInfo.height = height();
		texInfo.target = target;
		texInfo.format = TextureFormat::TextureFormat_FLOAT32;
		texInfo.usage = TextureUsage::TextureUsage_AttachmentDepth;
		texInfo.multiSample = multiSample;
		texInfo.useMipmaps = false;

		SamplerInfo smInfo{};
		smInfo.filterMag = Filter_NEAREST;
		smInfo.filterMin = Filter_NEAREST;

		outBuffer = createTexture(texInfo, smInfo, {});
	}
}

void RendererOpenGL::setupShadowMapBuffer(std::shared_ptr<Texture> &outBuffer, int width, int height,
                                          bool multiSample, bool bCubeMap, bool force) const {
	TextureTarget target = bCubeMap? TextureTarget_TEXTURE_CUBE_MAP: multiSample? TextureTarget_TEXTURE_2D_MULTISAMPLE : TextureTarget_TEXTURE_2D;
	if (outBuffer) {
		const TextureInfo& texInfo = outBuffer->getTextureInfo();
		force = force || texInfo.width != width || texInfo.height != height
				|| texInfo.target != target
				|| texInfo.format != TextureFormat_FLOAT32;
	}

	if (!outBuffer || outBuffer->multiSample() != multiSample || force) {
		TextureInfo texInfo{};
		texInfo.width = width;
		texInfo.height = height;
		texInfo.target = target;
		texInfo.format = TextureFormat::TextureFormat_FLOAT32;
		texInfo.usage = TextureUsage::TextureUsage_AttachmentColor | TextureUsage::TextureUsage_Sampler;
		texInfo.type = bCubeMap? TEXTURE_TYPE_SHADOWMAP_CUBE: TEXTURE_TYPE_SHADOWMAP;
		texInfo.multiSample = multiSample;
		texInfo.useMipmaps = false;

		SamplerInfo smInfo{};
		smInfo.filterMag = Filter_NEAREST;;
		smInfo.filterMin = Filter_NEAREST;
		// smInfo.wrapS = Wrap_CLAMP_TO_EDGE;
		// smInfo.wrapT = Wrap_CLAMP_TO_EDGE;
		smInfo.wrapS = bCubeMap? Wrap_CLAMP_TO_EDGE : Wrap_CLAMP_TO_BORDER;
		smInfo.wrapT = bCubeMap? Wrap_CLAMP_TO_EDGE : Wrap_CLAMP_TO_BORDER;
		smInfo.wrapR = bCubeMap? Wrap_CLAMP_TO_EDGE : Wrap_CLAMP_TO_BORDER;
		smInfo.borderColor = BorderColor::Border_WHITE;

		outBuffer = createTexture(texInfo, smInfo, {});
	}
}

void RendererOpenGL::setupVertexAttribute(const BufferAttribute &vertexAttribute) const {
	// bind ref to
	GLuint& VBO = vertexAttribute.VBO;
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
}

void RendererOpenGL::setupGeometry(FGeometry& geometry) const {
	// realy return
	if (geometry.isPipelineReady()) return;

	// setup attributes
	for(auto&& [_, data]: geometry.getBufferData()) {
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
void RendererOpenGL::loadShaders(FMaterial& material) const {
	auto shadingMode = material.shadingMode();
	switch (shadingMode)
	{
	case Shading_Unknown:
		break;
	case Shading_BaseColor:
		material.setShader(ShaderPass::Shader_ForwardShading_Pass, ShaderGLSL::loadBaseColorShader());
		material.setShader(ShaderPass::Shader_Geometry_Pass, ShaderGLSL::loadGeometryShader());
		material.setShader(ShaderPass::Shader_Shadow_Pass, ShaderGLSL::loadShadowPassShader());
		material.setShader(ShaderPass::Shader_Shadow_Cube_Pass, ShaderGLSL::loadShadowCubePassShader());
		break;
	case Shading_BlinnPhong:
		material.setShader(ShaderPass::Shader_ForwardShading_Pass, ShaderGLSL::loadBlinnPhongShader());
		material.setShader(ShaderPass::Shader_Geometry_Pass, ShaderGLSL::loadGeometryShader());
		material.setShader(ShaderPass::Shader_Shadow_Pass, ShaderGLSL::loadShadowPassShader());
		material.setShader(ShaderPass::Shader_Shadow_Cube_Pass, ShaderGLSL::loadShadowCubePassShader());
		break;
	case Shading_PBR:
		break;
	case Shading_Skybox:
		material.setShader(ShaderPass::Shader_ForwardShading_Pass, ShaderGLSL::loadSkyBoxShader());
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

void RendererOpenGL::setupMaterial(FMaterial& material) const {
	if (material.isPipelineReady()) return;

	// textures
	if (!material.texturesReady()) {

		material.clearTextures_runtime();
		for (auto& [type, textureData] : material.getTextureData()) {
			const bool bIsCubeMap = textureData.loadedTextureType == TEXTURE_TYPE_CUBE;
			TextureInfo texInfo{};
			texInfo.width = textureData.dataArray[0]->width();
			texInfo.height = textureData.dataArray[0]->height();
			texInfo.type = textureData.loadedTextureType;
			texInfo.target = bIsCubeMap? TextureTarget_TEXTURE_CUBE_MAP: TextureTarget_TEXTURE_2D;
			texInfo.format = TextureFormat_RGBA8;
			texInfo.usage = TextureUsage_Sampler;
			texInfo.multiSample = false;;
			texInfo.useMipmaps = false;

			SamplerInfo smInfo{};
			smInfo.filterMag = Filter_LINEAR;
			smInfo.filterMin = Filter_LINEAR;
			// for skybox
			smInfo.wrapR = bIsCubeMap? Wrap_CLAMP_TO_EDGE : Wrap_REPEAT;
			smInfo.wrapS = bIsCubeMap? Wrap_CLAMP_TO_EDGE : Wrap_REPEAT;
			smInfo.wrapR = bIsCubeMap? Wrap_CLAMP_TO_EDGE : Wrap_REPEAT;

			auto texture = createTexture(texInfo, smInfo, textureData);
			material.setTexture_runtime(texInfo.type, texture);

			const auto& samplerName = Texture::samplerName(static_cast<TextureType>(type));
			auto sampler = std::make_shared<UniformSamplerOpenGL>(samplerName, (TextureTarget)texInfo.target, (TextureFormat)texInfo.format);
			sampler->setTexture(*texture);
			material.setUniformSampler(sampler->name(), sampler);
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

void RendererOpenGL::setupMesh(const std::shared_ptr<UMesh> &mesh, ShaderPass shaderPass) const {
	// return early
	if (mesh->isPipelineReady()) return;

	auto pmaterial = mesh->getMaterial();
	if (auto pmaterial = mesh->getMaterial()) {

		// set shading mode
		pmaterial->setShadingMode(mesh->getShadingMode());
		setupMaterial(*pmaterial);

	} // material

	if (auto pgeometry = mesh->getGeometry()) {

		setupGeometry(*pgeometry);

		// if VBO ready
		if (mesh->isPipelineReady()) {
			return;
		}

		GLuint& VAO = mesh->VAO;
		GL_CHECK(glGenVertexArrays(1, &VAO));
		GL_CHECK(glBindVertexArray(VAO));

		// attrs
		for (auto&& [attr, data]: pgeometry->getBufferData()) {
			if (auto pshader = ShaderGLSL::loadDefaultShader()) {
				// 错误的初始化shader，用于查找location，可能导致错误的顶点数据绑定，需要格外小心。
				// 当然最简单的方法就是直接硬编码顺序
				auto loc = pshader->getAttributeLocation(FGeometry::getAttributeName(attr));
				if (loc != -1) {
					const auto& VBO = data.VBO;
					GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, VBO));
					// 指定顶点属性的解释方式（如何解释VBO中的数据）
					// 1. glVertexAttribPointer
					// attri的Location(layout location = 0) | item_size | 数据类型 | 是否Normalize to 0-1 | stride | 从Buffer起始位置开始的偏移
					GL_CHECK(glVertexAttribPointer(loc, data.elem_size(), GL_FLOAT, GL_FALSE, data.elem_size() * sizeof(float), (void*)0));
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
		GL_CHECK(glBindVertexArray(0));

		mesh->setPipelineReady(true);

	} // geometry

}

// main render
void RendererOpenGL::draw(const std::shared_ptr<UMesh> &mesh, const ShaderPass pass, const std::shared_ptr<ULight> &shadowLight, const std::
                          shared_ptr<Shader> &overrideShader)
{
	if (!mesh) return; // null check

	if (mesh->drawable()) {
		setupMesh(mesh, pass);
		mesh->updateFrame(*this);

		updateModelUniformBlock(mesh, mainCamera_, shadowLight);
		// use shader forcely and explicitly
		if (overrideShader) {
			loadGlobalUniforms(*overrideShader);
			overrideShader->bindHoldingResources();
		}
		// use shaders with mesh
		else {
			loadGlobalUniforms(mesh);
			const auto pmaterial = mesh->getMaterial();
			pmaterial->use(pass); // bind shader resources
		}

		const auto VAO = mesh->VAO;
		auto pgeometry = mesh->getGeometry();
		GL_CHECK(glBindVertexArray(VAO));
		if (pgeometry->isMesh()) {
			// primitive | 顶点数组起始索引 | 绘制indices数量
			GL_CHECK(glDrawArrays(GL_TRIANGLES, 0, pgeometry->getVeticesNum()));
		}
		else {
			// primitive | nums | 索引类型 | 最后一个参数里我们可以指定EBO中的偏移量（或者传递一个索引数组，但是这是当你不在使用EBO的时候），但是我们会在这里填写0。
			GL_CHECK(glDrawElements(GL_TRIANGLES, pgeometry->getIndicesNum(), GL_UNSIGNED_INT, nullptr));
		}
		GL_CHECK(glBindVertexArray(0));
	}

	for (auto&& child: mesh->getChildren()) {
		if (auto childMesh = std::dynamic_pointer_cast<UMesh>(child)) {
			childMesh->setShadingMode(mesh->getShadingMode());
			draw(childMesh, pass, shadowLight, overrideShader);
		}
	}

}

#define GL_STATE_SET(var, gl_state) if (var) GL_CHECK(glEnable(gl_state)); else GL_CHECK(glDisable(gl_state));
#define GL_COLOR_SET(color) GL_CHECK(glBlendColor(color.x, color.y, color.z, color.w));

void RendererOpenGL::updateRenderStates(const RenderStates &inRenderStates) {

	renderStates = inRenderStates;

	// blend
	GL_STATE_SET(inRenderStates.blend, GL_BLEND)
	GL_CHECK(glBlendEquationSeparate(OpenGL::cvtBlendFunction(inRenderStates.blendParams.blendFuncRgb),
									 OpenGL::cvtBlendFunction(inRenderStates.blendParams.blendFuncAlpha)));
	GL_CHECK(glBlendFuncSeparate(OpenGL::cvtBlendFactor(inRenderStates.blendParams.blendSrcRgb),
								 OpenGL::cvtBlendFactor(inRenderStates.blendParams.blendDstRgb),
								 OpenGL::cvtBlendFactor(inRenderStates.blendParams.blendSrcAlpha),
								 OpenGL::cvtBlendFactor(inRenderStates.blendParams.blendDstAlpha)));
	// blend color
	GL_COLOR_SET(inRenderStates.blendParams.blendColor);

	// depth
	GL_STATE_SET(inRenderStates.depthTest, GL_DEPTH_TEST);
	GL_CHECK(glDepthMask(inRenderStates.depthMask));
	GL_CHECK(glDepthFunc(OpenGL::cvtDepthFunc(inRenderStates.depthFunc)));

	// cull face
	GL_STATE_SET(inRenderStates.cullFace, GL_CULL_FACE);
	GL_CHECK(glCullFace(OpenGL::cvtCullMode(inRenderStates.faceToCull)));
	GL_CHECK(glPolygonMode(GL_FRONT_AND_BACK, OpenGL::cvtPolygonMode(inRenderStates.polygonMode)));

	GL_CHECK(glLineWidth(inRenderStates.lineWidth));
	GL_CHECK(glEnable(GL_PROGRAM_POINT_SIZE));
}

void RendererOpenGL::setCullFaceEnabled(const bool bEnabled) {
	GL_STATE_SET(bEnabled, GL_CULL_FACE);
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

void RendererOpenGL::dump(const
                          std::shared_ptr<Shader> &program, const bool bBlend, std::shared_ptr<FrameBuffer> targetFrameBuffer, int dstColorBuffer)
{
	static GLuint VAO = 0;
	static GLuint VBO = 0;
	static GLuint EBO = 0;

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

		if (targetFrameBuffer) {
			// blend ni kita no
			targetFrameBuffer->bind();
			targetFrameBuffer->setWriteBuffer(dstColorBuffer, false);
		}
		// 只有直出时才管理状态
		else {
			GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, 0));
			GL_CHECK(glDisable(GL_BLEND));
			GL_CHECK(glDisable(GL_DEPTH_TEST));
			GL_CHECK(glDepthMask(true));
			GL_CHECK(glDisable(GL_CULL_FACE));
			GL_CHECK(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL));
		}

		if (bBlend) {
			// blend的状态由caller控制，这里也是无状态的
		}
		else {
			GL_CHECK(glClearColor(0.f, 0.f, 0.f, 0.0f));
			GL_CHECK(glClear(GL_COLOR_BUFFER_BIT));
		}

		// load Uniforms
		loadGlobalUniforms(*program);
		program->bindHoldingResources(); // where really 'use()'

		GL_CHECK(glBindVertexArray(VAO));
		GL_CHECK(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr));
	}
	else {
		LOGE("Failed render to screen!");
	}
}
;;

std::shared_ptr<ShaderGLSL> RendererOpenGL::getToScreenColorProgram(const std::shared_ptr<Texture> &srcTex) const {
	auto&& sampler = srcTex->getUniformSampler(*this);
	sampler->setName("uTexture");

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
		float gamma = 1.0;
		FragColor = texture(uTexture, TexCoord);
		FragColor.rgb = pow(FragColor.rgb, vec3(1.0/gamma));
	}
	)";

	static std::shared_ptr<ShaderGLSL> program;
	static Serika::UUID<ShaderResources> resourceUUID;

	if (!program) {
		program = ShaderGLSL::loadFromRawSource(VS, FS);
		program->compileAndLink();

		auto&& resources = noObjectContextShaderResources[resourceUUID.get()];
		if (!resources) {
			resources = std::make_shared<ShaderResources>();
			program->setResources(resources);
		}
	}

	program->setUniformSampler(sampler->name(), sampler); // this target will change frequently, bind every time.

	// name of outTex here should be corresponding to that in shader.

	return program;
}

std::shared_ptr<ShaderGLSL> RendererOpenGL::getToScreenDepthProgram(const std::shared_ptr<Texture> &srcTex) const {
	auto&& sampler = srcTex->getUniformSampler(*this);
	sampler->setName("uTexture");

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
	static Serika::UUID<ShaderResources> resourceUUID;

	if (!program) {
		program = ShaderGLSL::loadFromRawSource(VS, FS);
		program->compileAndLink();

		auto&& resources = noObjectContextShaderResources[resourceUUID.get()];
		if (!resources) {
			resources = std::make_shared<ShaderResources>();
			program->setResources(resources);
		}
	}

	program->setUniformSampler(sampler->name(), sampler);

	return program;
}

std::shared_ptr<ShaderGLSL> RendererOpenGL::getToScreenCubeDepthProgram(const std::shared_ptr<Texture> &srcTex) const {
	auto&& sampler = srcTex->getUniformSampler(*this);
	sampler->setName("uCubeShadow");

	static const char* VS = R"(
		#version 430 core
		layout(location = 0) in vec3 aPos;
		layout(location = 1) in vec2 aTexCoord;
		layout(location = 2) in vec3 aNormal;

		out vec2 TexCoord;
		out vec3 vFragPos;

		layout(std140) uniform Model {
		    mat4 uModel;
		    mat4 uView;
		    mat4 uProjection;
		    mat4 uNormalToWorld;
		    mat4 uShadowMapMVP;
		    vec3 uViewPos;
		    bool uUseShadowMap;
		    bool uUseShadowMapCube;
		    bool uUseEnvMap;
		};

		void main() {
		    TexCoord = aTexCoord;
		    vFragPos = vec3(uModel * vec4(aPos, 1.0));
		    gl_Position = uProjection * uView * uModel * vec4(aPos, 1.0);
		}
	)";

	static const char* FS = R"(
		#version 430 core

		in vec3 vFragPos;
		layout(location = 0) out vec4 FragColor;

		layout(std140) uniform ShadowCube {
		    mat4 uShadowVPs[6];
		    float uFarPlane;
		};

		uniform samplerCube uCubeShadow;

		layout(std140) uniform Light {
		    int uLightType;

		    vec3 uLightPosition;
		    vec3 uLightDirection;
		    vec3 uLightAmbient;
		    vec3 uLightDiffuse;
		    vec3 uLightSpecular;

		    float uLightCutoff;
		    float uLightOuterCutoff;
		    float uLightConstant;
		    float uLightLinear;
		    float uLightQuadratic;
		};

		void main() {
		    // Get vector between fragment position and light position
		    vec3 lightDir = uLightPosition - vFragPos;
		    // Use the light to fragment vector to sample from the depth map
		    float closestDepth = texture(uCubeShadow, -lightDir).r;
		    // It is currently in linear range between [0,1]. Re-transform back to original value
		    // closestDepth *= uFarPlane;
		    FragColor  = vec4(vec3(closestDepth), 1.f);
		}
	)";

	static std::shared_ptr<ShaderGLSL> program;
	static Serika::UUID<ShaderResources> resourceUUID;

	if (!program) {
		program = ShaderGLSL::loadFromRawSource(VS, FS);
		program->compileAndLink();

		auto&& resources = noObjectContextShaderResources[resourceUUID.get()];
		if (!resources) {
			resources = std::make_shared<ShaderResources>();
			program->setResources(resources);
		}
	}

	program->setUniformSampler(sampler->name(), sampler);

	return program;
}

std::shared_ptr<ShaderGLSL> RendererOpenGL::getDefferedShadingProgram(const std::vector<std::shared_ptr<Texture>> &gBuffers) const {

	static std::shared_ptr<ShaderGLSL> program;
	static Serika::UUID<ShaderResources> resourceUUID;

	if (!program) {
		program = ShaderGLSL::loadDefferedBlinnPhongShader();
		program->compileAndLink();

		auto&& resources = noObjectContextShaderResources[resourceUUID.get()];
		if (!resources) {
			resources = std::make_shared<ShaderResources>();
			program->setResources(resources);
		}
	}

	// bind gbuffers
	for(int i = 0; i < gBuffers.size(); i++) {
		const auto& gbuffer = gBuffers[i];
		auto&& sampler = gbuffer->getUniformSampler(*this);
		sampler->setName(RenderPassGeometry::GBUFFER_NAMES[i]);
		program->setUniformSampler(sampler->name(), sampler);
	}

	return program;
}

RendererOpenGL::~RendererOpenGL()
{
}

void RendererOpenGL::clearTexture(Texture& texture)
{
}


