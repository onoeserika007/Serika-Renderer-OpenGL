#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "OpenGL/RendererOpenGL.h"
#include "OpenGL/UniformOpenGL.h"
#include "Base/Globals.h"
#include "OpenGL/ShaderGLSL.h"
#include "Geometry/BufferAttribute.h"
#include "Geometry/Geometry.h"
#include "OpenGL/TextureOpenGL.h"
#include "OpenGL/EnumsOpenGL.h"
#include "Geometry/Object.h"
#include "FCamera.h"
#include "FScene.h"
#include "OpenGL/FrameBufferOpenGL.h"
#include "Utils/Logger.h"
#include "Utils/OpenGLUtils.h"
#include "ULight.h"
#include "Base/ThreadPool.h"
#include "Geometry/Triangle.h"
#include "Geometry/UMesh.h"

#include "RenderPass/RenderPassGeometry.h"
#include "Material/FMaterial.h"

// set up vertex data (and buffer(s)) and configure vertex attributes
const std::vector<float> ToScreenRectangleVertices = {
	// positions | texture coords
	1.f, 1.f, 0.0f, 1.0f, 1.0f,   // top right
	1.f, -1.f, 0.0f, 1.0f, 0.0f,  // bottom right
	-1.f, -1.f, 0.0f, 0.0f, 0.0f, // bottom left
	-1.f, 1.f, 0.0f, 0.0f, 1.0f   // top left
};

const std::vector<GLuint> ToScreenRectangleIndices = {
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
	sceneUniformBlock_ = createUniformBlock("Scene", sizeof(SceneUniformBlock));

	// place holder
	setupColorBuffer(envCubeMapPlaceholder_, 1, 1, false, true, TextureTarget_TEXTURE_CUBE_MAP, TextureFormat_RGBA8, TEXTURE_TYPE_CUBE);
	setupColorBuffer(abientOcclusionPlaceholader_, 1, 1, false, false);
	setupShadowMapBuffer(shadowMapPlaceholder_, 1, 1, false, false, false);
	setupShadowMapBuffer(shadowMapCubePlaceholder_, 1, 1, false, true, false);

	// generate ssao random vecs
	std::vector<float> samples;
	for (int i = 0; i < 64; i++) {
		glm::vec3 wo;
		float pdf;
		MathUtils::UniformHemisphereSampleByVolume(wo, pdf, {}, {0.f, 1.f, 0.f}, true, 0, MathUtils::grayCode(i));
		samples.push_back(wo.x);
		samples.push_back(wo.y);
		samples.push_back(wo.z);
	}
	ssaoKernelTexture_ = createBufferTexture(8, 8, samples, TextureFormat_RGB32F);
	ssaoKernelUniformSampler_ = ssaoKernelTexture_->getUniformSampler(*this);
	ssaoKernelUniformSampler_->setName("uSSAOKernel");

	auto noisies = Buffer<glm::vec3>::makeBuffer(4, 4);
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			auto randomVec = glm::vec3(MathUtils::get_random_float(), 0.f, MathUtils::get_random_float());
			noisies->getPixelRef(i, j) = randomVec;
		}
	}
	TextureData randVecsData {};
	randVecsData.floatDataArray.push_back(noisies);;
	setupColorBuffer(noiseTexture_, 4, 4, false, false, TextureTarget_TEXTURE_2D, TextureFormat_RGB32F, TEXTURE_TYPE_NONE, randVecsData);
	// noiseTexture_ = createBufferTexture(8, 8, noisies, TextureFormat_RGB32F);
	noiseUniformSampler_ = noiseTexture_->getUniformSampler(*this);
	noiseUniformSampler_->setName("uRandomVecs");

	// in case uniform don't have a texture binding to it, then nsight will got error
	envCubeMapUniformSampler_ = envCubeMapPlaceholder_->getUniformSampler(*this);
	shadowMapUniformSampler_ = shadowMapPlaceholder_->getUniformSampler(*this);
	shadowMapCubeUniformSampler_ = shadowMapCubePlaceholder_->getUniformSampler(*this);
	abientOcclusionUniformSampler_ = abientOcclusionPlaceholader_->getUniformSampler(*this);
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
	else if (texInfo.target == TextureTarget_TEXTURE_BUFFER) {
		return std::make_shared<TextureOpenGLBuffer>(texInfo, smInfo, texData);
	}
	return nullptr;
}

std::shared_ptr<FrameBuffer> RendererOpenGL::createFrameBuffer(bool offScreen)
{
	return std::make_shared<FrameBufferOpenGL>(offScreen);
}

void RendererOpenGL::setupVertexAttribute(BufferAttribute &vertexAttribute) const {
	if (vertexAttribute.isPipelineSetup()) return;
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
	vertexAttribute.setPipelineReady(true);
	vertexAttribute.deleter_ = [VBO = vertexAttribute.VBO] {
		GL_CHECK(glDeleteBuffers(1, &VBO));
	};
}

void RendererOpenGL::setupGeometry(FGeometry& geometry) const {
	// realy return
	if (geometry.isPipelineReady()) return;

	// setup VAO
	GLuint& VAO = geometry.VAO;
	GL_CHECK(glGenVertexArrays(1, &VAO));
	GL_CHECK(glBindVertexArray(VAO));

	// setup attributes
	for(auto&& [_, data]: geometry.getBufferData()) {
		setupVertexAttribute(data);
	}

	// setup indices
	if (geometry.isMeshIndexed()) {
		GLuint EBO = 0;
		GL_CHECK(glGenBuffers(1, &EBO));
		GL_CHECK(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO));
		GL_CHECK(glBufferData(GL_ELEMENT_ARRAY_BUFFER, geometry.getIndicesNum() * sizeof(unsigned), geometry.getIndicesRawData(), GL_STATIC_DRAW));
		GL_CHECK(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
		geometry.setEBO(EBO);
	}

	// attrs
	for (auto&& [attr, data]: geometry.getBufferData()) {
		// if (auto pshader = ShaderGLSL::loadDefaultShader()) {
		// 	// 错误的初始化shader，用于查找location，可能导致错误的顶点数据绑定，需要格外小心。
		// 	// 当然最简单的方法就是直接硬编码顺序
		// }
		// now use enum as loc
		const auto& VBO = data.VBO;;
		GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, VBO));
		// 指定顶点属性的解释方式（如何解释VBO中的数据）
		// 1. glVertexAttribPointer
		// attri的Location(layout location = 0) | item_size | 数据类型 | 是否Normalize to 0-1 | stride | 从Buffer起始位置开始的偏移
		GL_CHECK(glVertexAttribPointer(attr, data.elem_size(), GL_FLOAT, GL_FALSE, data.elem_size() * sizeof(float), (void*)0));
		// 以顶点属性位置值作为参数，启用顶点属性；顶点属性默认是禁用的
		GL_CHECK(glEnableVertexAttribArray(attr));
	}

	// indices
	if (geometry.isMeshIndexed()) {
		auto EBO = geometry.getEBO();
		GL_CHECK(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO));
	}

	// set back
	GL_CHECK(glBindVertexArray(0));

	geometry.setPipelineReady(true);

	// LOGD("Geometry Setup - VAO: %d", geometry.VAO);
	geometry.deleter_ = [VAO = geometry.VAO, EBO = geometry.EBO]() {
		GL_CHECK(glDeleteBuffers(1, &EBO));
		GL_CHECK(glDeleteVertexArrays(1, &VAO));
		// LOGD("Geometry Teardown - VAO: %d", VAO);
	};
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
		material.setShader(ShaderPass::Shader_ForwardShading_Pass, ShaderGLSL::loadBlinnPhongShader());
		material.setShader(ShaderPass::Shader_Geometry_Pass, ShaderGLSL::loadGeometryShader());
		material.setShader(ShaderPass::Shader_Shadow_Pass, ShaderGLSL::loadShadowPassShader());
		material.setShader(ShaderPass::Shader_Shadow_Cube_Pass, ShaderGLSL::loadShadowCubePassShader());
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
			texInfo.width = textureData.unitDataArray[0]->width();
			texInfo.height = textureData.unitDataArray[0]->height();
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
		}

		material.setTexturesReady(true);
	}

	// shaders
	if (!material.shaderReady()) {
		loadShaders(material); 
		for (auto& [pass, shader] : material.getShaders()) {
			shader->setupPipeline(material);
		}

		auto&& matobj = material.getMaterialObject();
		for (auto&& [type, texture]: matobj->texturesRuntime_) {
			/****这里是唯一正确装配sampler name的地方 **/
			const auto& samplerName = Texture::samplerName(static_cast<TextureType>(type));
			const auto& texInfo = texture->getTextureInfo();
			auto sampler = std::make_shared<UniformSamplerOpenGL>(samplerName, (TextureTarget)texInfo.target, (TextureFormat)texInfo.format);
			sampler->setTexture(*texture);
			material.setUniformSampler(sampler->name(), sampler);
		}

		material.setShaderReady(true);
	}
}

void RendererOpenGL::setupMesh(const std::shared_ptr<UMesh> &mesh, ShaderPass shaderPass) {
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
	} // geometry

}

// main render
void RendererOpenGL::drawMesh(const std::shared_ptr<UMesh> &mesh, const ShaderPass pass, const std::shared_ptr<ULight> &shadowLight, const std::
                          shared_ptr<Shader> &overrideShader)
{
	if (!mesh) return; // null check

	auto&& config = Config::getInstance();;

	// draw mesh
	if (mesh->drawable()) {
		setupMesh(mesh, pass);
		// update model unifor
		updateMainUniformBlock(mesh, mainCamera_, shadowLight);

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

		auto pgeometry = mesh->getGeometry();
		const auto VAO = pgeometry->VAO;
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

	for (auto&& child: mesh->getMeshes()) {
		child->setShadingMode(mesh->getShadingMode());
		drawMesh(child, pass, shadowLight, overrideShader);
	}

}

void RendererOpenGL::drawDebugBBoxes(const std::shared_ptr<BVHNode> &node, int depth, const std::shared_ptr<UObject> &rootObject) {

	if (!node || depth > 10) return;
	auto&& config = Config::getInstance();;
	if (config.bUseBVH) {
		if (node->primitive) {
			if (auto&& obj = std::dynamic_pointer_cast<UObject>(node->primitive)) {
				// new root
				drawDebugBBox(node->bbox, obj, depth);
				auto&& meshBVHRoot = obj->getBVH()->getRoot();
				drawDebugBBoxes(meshBVHRoot, depth + 1, obj);
			}
			else {
				// if (!node->left && !node->right)
					drawDebugBBox(node->bbox, rootObject, depth);
				drawDebugBBoxes(node->left, depth + 1, rootObject);
				drawDebugBBoxes(node->right, depth + 1, rootObject);
			}
		}
		else {
			// if (depth > 25) drawDebugBBox(node->bbox, {});
			// if (!node->left && !node->right)
				drawDebugBBox(node->bbox, rootObject, depth);
			drawDebugBBoxes(node->left, depth + 1, rootObject);
			drawDebugBBoxes(node->right, depth + 1, rootObject);
		}
	}

}

void RendererOpenGL::drawDebugBBox(const BoundingBox &bbox, const std::shared_ptr<UObject> &holdingObject, int depth) {
	// if (depth != 17) return;
	if (!bbox_draw_cache_.contains(bbox.getUUID())) {
		glm::vec3 vertices[8] = {};
		bbox.getCorners(vertices);;

		static GLuint indices[24] = {
			0, 1, 1, 2, 2, 3, 3, 0, // Near face
			4, 5, 5, 6, 6, 7, 7, 4, // Far face
			1, 6, 2, 5, 3, 4, 0, 7  // Connecting edges
		};
		// std::cout << "Setting up bbox" << std::endl;
		GLuint VBO, VAO, EBO;
		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);
		glGenBuffers(1, &EBO);

		glBindVertexArray(VAO);

		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);

		bbox_draw_cache_[bbox.getUUID()] = VAO;
	}

	if (bbox_draw_cache_.contains(bbox.getUUID())) {
		int uuid = bbox.getUUID();
		const RenderStates render_states = renderStates_;
		GLuint VAO = bbox_draw_cache_[bbox.getUUID()];
		auto&& debugShader = getDebugBBoxProgram();

		// now this is not neccessary, we apply model matrix before build bvh
		// updateModelUniformBlock(holdingMesh, mainCamera_, {}); // update model matrix if root mesh changed
		updateMainUniformBlock({}, mainCamera_, {});
		loadGlobalUniforms(*debugShader);
		debugShader->bindHoldingResources();
		debugShader->setFloat("uLayerDepth", depth);

		auto parentUUID = 0;
		if (holdingObject) {
			parentUUID = holdingObject->getUUID();
			debugShader->setInt("uParentUUID", holdingObject->getUUID());
		}

		// 设置线框模式
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glDisable(GL_DEPTH_TEST);
		glDepthMask(GL_FALSE);

		// 绑定VAO并绘制元素
		glBindVertexArray(VAO);
		glDrawElements(GL_LINES, 24, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);

		// 恢复填充模式
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glDepthMask(GL_TRUE);
		glEnable(GL_DEPTH_TEST);

		updateRenderStates(render_states);
	}
}

void RendererOpenGL::drawWorldAxis(const std::shared_ptr<UMesh> &holdingMesh) {
	// 设置顶点数据和缓冲
	// 设置顶点数据和缓冲
	static constexpr float vertices[] = {
		// X轴
		0.0f,  0.0f,  0.0f,
		1.0f,  0.0f,  0.0f,
		// X箭头
		1.0f,  0.0f,  0.0f,
		0.9f,  0.05f,  0.05f,
		0.9f, -0.05f,  0.05f,
		0.9f, -0.05f, -0.05f,
		0.9f,  0.05f, -0.05f,

		// Y轴
		0.0f,  0.0f,  0.0f,
		0.0f,  1.0f,  0.0f,
		// Y箭头
		0.0f,  1.0f,  0.0f,
		0.05f,  0.9f,  0.05f,
	   -0.05f,  0.9f,  0.05f,
	   -0.05f,  0.9f, -0.05f,
		0.05f,  0.9f, -0.05f,

		// Z轴
		0.0f,  0.0f,  0.0f,
		0.0f,  0.0f,  1.0f,
		// Z箭头
		0.0f,  0.0f,  1.0f,
		0.05f,  0.05f,  0.9f,
	   -0.05f,  0.05f,  0.9f,
	   -0.05f, -0.05f,  0.9f,
		0.05f, -0.05f,  0.9f,
   };

	static const unsigned int indices[] = {
		0, 1,        // X轴
		2, 3, 2, 4,  // X箭头
		2, 5, 2, 6,

		7, 8,        // Y轴
		9, 10, 9, 11, // Y箭头
		9, 12, 9, 13,

		14, 15,      // Z轴
		16, 17, 16, 18, // Z箭头
		16, 19, 16, 20,
	};

	static GLuint VBO, VAO, EBO;
	if (VBO == 0 || VAO == 0 || EBO == 0) {
		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);
		glGenBuffers(1, &EBO);

		glBindVertexArray(VAO);

		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);

		// glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}

	if (auto && debugShader = getDrawDebuglineProgram()) {
		// update MVP
		updateMainUniformBlock(holdingMesh, mainCamera_, {});
		loadGlobalUniforms(*debugShader);
		debugShader->bindHoldingResources();

		glBindVertexArray(VAO);
		// set opengl state
		glDisable(GL_DEPTH_TEST);
		glDepthMask(GL_FALSE);
		// 绘制X轴
		debugShader->setVec3("uLineColor", glm::vec3(1.f, 0.f, 0.f));
		glDrawElements(GL_LINES, 2, GL_UNSIGNED_INT, 0);
		glDrawElements(GL_LINES, 8, GL_UNSIGNED_INT, (void*)(2 * sizeof(unsigned int)));

		// 绘制Y轴
		debugShader->setVec3("uLineColor", glm::vec3(0.f, 1.f, 0.f));
		glDrawElements(GL_LINES, 2, GL_UNSIGNED_INT, (void*)(10 * sizeof(unsigned int)));
		glDrawElements(GL_LINES, 8, GL_UNSIGNED_INT, (void*)(12 * sizeof(unsigned int)));

		// 绘制Z轴
		debugShader->setVec3("uLineColor", glm::vec3(0.f, 0.f, 1.f));
		glDrawElements(GL_LINES, 2, GL_UNSIGNED_INT, (void*)(20 * sizeof(unsigned int)));
		glDrawElements(GL_LINES, 8, GL_UNSIGNED_INT, (void*)(22 * sizeof(unsigned int)));

		// recover
		glDepthMask(GL_TRUE);
		glEnable(GL_DEPTH_TEST);
	}

}

void RendererOpenGL::drawDebugPoint_Impl(const std::shared_ptr<FPoint> &point) {
	if (point->VAO_ == 0) {
		GLuint VBO;
		glm::vec3 pointVertices[1] = { point->position_ };

		glGenVertexArrays(1, &point->VAO_);
		glGenBuffers(1, &VBO);
		glBindVertexArray(point->VAO_);

		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		// size means byte size, prevent array from degradating to const ptr.
		glBufferData(GL_ARRAY_BUFFER, sizeof(pointVertices), pointVertices, GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
		glEnableVertexAttribArray(0);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);

		// if 0.f, do not remove point
		if (point->persistTime_ > 0.f) {
			std::weak_ptr<Renderer> self = shared_from_this();
			auto releaseResource = [persistentTime = point->persistTime_, uuid = point->getUUID(), self]() {
				std::this_thread::sleep_for(std::chrono::duration<double>(persistentTime));
				if (auto&& renderer = self.lock())
				{
					renderer->remove_point_debug_task_safe(uuid);
				}
			};
			point->deleter_ = [VBO = VBO, VAO = point->VAO_]() {
				glDeleteBuffers(1, &VBO);
				glDeleteVertexArrays(1, &VAO);
			};
			auto&& pool = FThreadPool::getInst();
			pool.pushTask(releaseResource); //
		}
	}

	// draw point
	{
		auto && debugShader = getDrawDebuglineProgram();
		// update MVP
		updateMainUniformBlock({}, mainCamera_, {});
		loadGlobalUniforms(*debugShader);
		debugShader->bindHoldingResources();

		// set opengl state
		glDisable(GL_DEPTH_TEST);
		glDepthMask(GL_FALSE);

		glBindVertexArray(point->VAO_);
		debugShader->setVec3("uLineColor", glm::vec3(0.f, 1.f, 0.f));
		glPointSize(point->pointSize_);
		glDrawArrays(GL_POINTS, 0, 1);
		glBindVertexArray(0);

		// recover
		glDepthMask(GL_TRUE);
		glEnable(GL_DEPTH_TEST);
	}
}


void RendererOpenGL::drawDebugLine_Impl(const std::shared_ptr<FLine> &line) {
	if (line->VAO_ == 0) {
		GLuint VBO;
		glm::vec3 lineVertex[2] = {line->start_, line->end_};

		glGenVertexArrays(1, &line->VAO_);
		glGenBuffers(1, &VBO);
		glBindVertexArray(line->VAO_);

		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		// size means byte size, prevent array from degradating to const ptr.
		glBufferData(GL_ARRAY_BUFFER, sizeof(lineVertex), lineVertex, GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
		glEnableVertexAttribArray(0);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);

		// if 0.f, do not remove line
		if (line->persistTime_ > 0.f) {
			std::weak_ptr<Renderer> self = shared_from_this();
			auto releaseResource = [persistentTime = line->persistTime_, uuid = line->getUUID(), self]() {
				std::this_thread::sleep_for(std::chrono::duration<double>(persistentTime));
				if (auto&& renderer = self.lock())
				{
					renderer->remove_line_debug_task_safe(uuid);
				}
			};
			line->deleter_ = [VBO = VBO, VAO = line->VAO_] {
				glDeleteBuffers(1, &VBO);
				glDeleteVertexArrays(1, &VAO);
			};
			auto&& pool = FThreadPool::getInst();
			pool.pushTask(releaseResource); //
		}

	}

	// draw line
	{
		auto && debugShader = getDrawDebuglineProgram();
		// update MVP
		updateMainUniformBlock({}, mainCamera_, {});
		loadGlobalUniforms(*debugShader);
		debugShader->bindHoldingResources();

		// set opengl state
		glDisable(GL_DEPTH_TEST);
		glDepthMask(GL_FALSE);

		glBindVertexArray(line->VAO_);
		debugShader->setVec3("uLineColor", glm::vec3(0.f, 1.f, 0.f));
		glDrawArrays(GL_LINES, 0, 2);
		glBindVertexArray(0);

		// recover
		glDepthMask(GL_TRUE);
		glEnable(GL_DEPTH_TEST);
	}

}

void RendererOpenGL::drawDebugTriangle_Impl(const std::shared_ptr<Triangle> &triangle) {
	if (triangle->VAO_ == 0) {
		GLuint VBO;
		glm::vec3 triVertices[3] = {triangle->v0_, triangle->v1_, triangle->v2_};

		GL_CHECK(glGenVertexArrays(1, &triangle->VAO_));

		glGenBuffers(1, &VBO);
		glBindVertexArray(triangle->VAO_);

		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		// size means byte size, prevent array from degradating to const ptr.
		glBufferData(GL_ARRAY_BUFFER, sizeof(triVertices), triVertices, GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
		glEnableVertexAttribArray(0);

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);

		// if 0.f, do not remove line
		if (triangle->persistTime_ > 0.f) {
			std::weak_ptr<Renderer> self = shared_from_this();
			auto releaseResource = [persistTime = triangle->persistTime_, uuid = triangle->getUUID(), self]() {
				std::this_thread::sleep_for(std::chrono::duration<double>(persistTime));
				if (auto&& renderer = self.lock())
				{
					renderer->remove_triangle_debug_task_safe(uuid);
				}
			};

			triangle->deleter_ = [VBO = VBO, VAO = triangle->VAO_] {
				glDeleteBuffers(1, &VBO);
				glDeleteVertexArrays(1, &VAO);
			};

			auto&& pool = FThreadPool::getInst();
			pool.pushTask(releaseResource); //
		}

	}

	// draw triangle
	{
		auto && debugShader = getDrawDebuglineProgram();
		// update MVP
		updateMainUniformBlock({}, mainCamera_, {});
		loadGlobalUniforms(*debugShader);
		debugShader->bindHoldingResources();

		// set opengl state
		GL_CHECK(glDisable(GL_DEPTH_TEST));
		glDepthMask(GL_FALSE);
		GL_CHECK(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL));
		GL_CHECK(glEnable(GL_CULL_FACE));

		GL_CHECK(glBindVertexArray(triangle->VAO_));
		debugShader->setVec3("uLineColor", glm::vec3(0.f, 1.f, 0.f));
		GL_CHECK(glDrawArrays(GL_TRIANGLES, 0, 3));
		glBindVertexArray(0);

		// recover
		glDepthMask(GL_TRUE);
		glEnable(GL_DEPTH_TEST);
		GL_CHECK(glDisable(GL_CULL_FACE));
	}
}

#define GL_STATE_SET(var, gl_state) if (var) GL_CHECK(glEnable(gl_state)); else GL_CHECK(glDisable(gl_state));
#define GL_COLOR_SET(color) GL_CHECK(glBlendColor(color.x, color.y, color.z, color.w));

void RendererOpenGL::updateRenderStates(const RenderStates &inRenderStates) {

	renderStates_ = inRenderStates;

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
	GL_CHECK(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
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
			GL_CHECK(glBufferData(GL_ARRAY_BUFFER, ToScreenRectangleVertices.size() * sizeof(float), ToScreenRectangleVertices.data(), GL_STATIC_DRAW));

			GL_CHECK(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), nullptr));
			GL_CHECK(glEnableVertexAttribArray(0));
			GL_CHECK(glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float))));
			GL_CHECK(glEnableVertexAttribArray(1));
		}

		if (EBO == 0) {
			GL_CHECK(glGenBuffers(1, &EBO));
			GL_CHECK(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO));
			GL_CHECK(glBufferData(GL_ELEMENT_ARRAY_BUFFER, ToScreenRectangleIndices.size() * sizeof(GLuint), ToScreenRectangleIndices.data(), GL_STATIC_DRAW));
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
		if (!targetFrameBuffer) { // do tone mapping lastly
			if (auto&& glslShader = std::dynamic_pointer_cast<ShaderGLSL>(program)) {
				auto&& config = Config::getInstance();
				glslShader->setFloat("uExposure", config.Exposure);
				glslShader->setBool("uUseToneMapping", config.bUseHDR);
			}
		}

		GL_CHECK(glBindVertexArray(VAO));
		GL_CHECK(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr));
	}
	else {
		LOGE("Failed render to screen!");
	}

	// GLuint VAO = 0;
	// GLuint VBO = 0;
	// GLuint EBO = 0;

	// bounds error, may related to context lost, rebind every frame to fix this problem
	// glDeleteBuffers(1, &EBO);
	// glDeleteBuffers(1, &VBO);
	// glDeleteVertexArrays(1, &VAO);
}
;;

std::shared_ptr<ShaderGLSL> RendererOpenGL::getToScreenColorProgram(const std::shared_ptr<Texture> &srcTex) const {
	auto&& sampler = srcTex->getUniformSampler(*this);
	sampler->setName("uTexture");

	static const char* VS = R"(
	layout (location = 0) in vec3 aPos;
	layout (location = 1) in vec2 aTexCoord;

	out vec2 TexCoords;

	void main()
	{
		gl_Position = vec4(aPos, 1.0);
		TexCoords = aTexCoord;
	}
	)";

	static const char* FS = R"(
	in vec2 TexCoords;
	out vec4 FragColor;

	uniform sampler2D uTexture;
	uniform float uExposure;
	uniform bool uUseToneMapping;

	void main()
	{
	    const float gamma = 2.2;
	    vec3 hdrColor = texture(uTexture, TexCoords).rgb;

	    // 曝光色调映射
	    vec3 mapped = vec3(1.0) - exp(-hdrColor * uExposure);
	    // Gamma校正
	    mapped = pow(mapped, vec3(1.0 / gamma));
		// hdrColor = pow(hdrColor, vec3(1.0 / gamma));
		if (uUseToneMapping) FragColor = vec4(mapped, 1.0);
		else FragColor = vec4(hdrColor, 1.0);
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

std::shared_ptr<ShaderGLSL> RendererOpenGL::getDebugBBoxProgram() const {
	static std::shared_ptr<ShaderGLSL> program;
	static Serika::UUID<ShaderResources> resourceUUID;

	if (!program) {
		program = ShaderGLSL::loadShader(
			"assets/shader/Debug/BoundingBox/bbox.vert",
			"assets/shader/Debug/BoundingBox/bbox.frag");
		program->compileAndLink();

		auto&& resources = noObjectContextShaderResources[resourceUUID.get()];
		if (!resources) {
			resources = std::make_shared<ShaderResources>();
			program->setResources(resources);
		}
	}

	return program;
}

std::shared_ptr<ShaderGLSL> RendererOpenGL::getDrawDebuglineProgram() const {
	static std::shared_ptr<ShaderGLSL> program;
	static Serika::UUID<ShaderResources> resourceUUID;

	if (!program) {
		program = ShaderGLSL::loadShader(
			"assets/shader/Debug/WorldAxis/DebugLine.vert",
			"assets/shader/Debug/WorldAxis/DebugLine.frag");
		program->compileAndLink();

		auto&& resources = noObjectContextShaderResources[resourceUUID.get()];
		if (!resources) {
			resources = std::make_shared<ShaderResources>();
			program->setResources(resources);
		}
	}

	return program;
}

std::shared_ptr<ShaderGLSL> RendererOpenGL::getDefferedShadingProgram(const std::vector<std::shared_ptr<Texture>> &gBuffers, EShadingModel shadingModel) const {
	static std::shared_ptr<ShaderGLSL> program;;
	static Serika::UUID<ShaderResources> resourceUUID;

	if (!program) {
		if (shadingModel == Shading_BlinnPhong) {
			program = ShaderGLSL::loadDefferedBlinnPhongShader();
		}
		else if (shadingModel == Shading_PBR) {
			program = ShaderGLSL::loadDefferedPBRShader();
		}

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

std::shared_ptr<ShaderGLSL> RendererOpenGL::
getSSAOProgram(const std::vector<std::shared_ptr<Texture>> &gBuffers) const {
	static std::shared_ptr<ShaderGLSL> program;
	static Serika::UUID<ShaderResources> resourceUUID;

	if (!program) {
		program = ShaderGLSL::loadSSAOPassShader();
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

std::shared_ptr<ShaderGLSL> RendererOpenGL::getSSAOBlurProgram(const std::shared_ptr<Texture> &ssaoInput) const {
	static std::shared_ptr<ShaderGLSL> program;
	static Serika::UUID<ShaderResources> resourceUUID;

	if (!program) {
		program = ShaderGLSL::loadSSAOBlurShader();
		program->compileAndLink();

		auto&& resources = noObjectContextShaderResources[resourceUUID.get()];
		if (!resources) {
			resources = std::make_shared<ShaderResources>();
			program->setResources(resources);
		}
	}

	// bind
	auto&& sampler = ssaoInput->getUniformSampler(*this);
	sampler->setName("uSSAOInput");
	program->setUniformSampler(sampler->name(), sampler);

	return program;
}

RendererOpenGL::~RendererOpenGL()
{
}

void RendererOpenGL::clearTexture(Texture& texture)
{
}


