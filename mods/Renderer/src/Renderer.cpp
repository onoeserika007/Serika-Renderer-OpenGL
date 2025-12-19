#include "Renderer.h"

#include <ULight.h>
#include <Material/Shader.h>

#include "FCamera.h"
#include "FScene.h"
#include "Geometry/Object.h"

#include "Material/Uniform.h"
#include "Base/Globals.h"
#include "Geometry/UMesh.h"

Renderer::Renderer(const std::shared_ptr<FCamera> &camera) {
	// 在基类的构造函数里调用虚函数是无效的！会调用基类的实现，但是基类是纯虚函数，没有实现，所以会发生链接错误
	// modelUniformBlock_ = createUniformBlock("Model", sizeof(ModelUniformBlock));
	mainCamera_ = camera;
	viewCamera_ = camera;
}

std::shared_ptr<Texture> Renderer::createBufferTexture(int width, int height, const std::vector<float> &bufferData,
                                                       TextureFormat texFormat) {
	TextureInfo texInfo{};
	texInfo.width = width;
	texInfo.height = height;
	texInfo.target = TextureTarget_TEXTURE_BUFFER;
	texInfo.format = texFormat;
	texInfo.type = TEXTURE_TYPE_NONE;
	texInfo.usage = 0;
	texInfo.multiSample = false;
	texInfo.useMipmaps = false;

	TextureData texData {};
	texData.bufferData = bufferData;
	return createTexture(texInfo, {}, texData);
}

bool Renderer::setupColorBuffer(std::shared_ptr<Texture> &outBuffer, int width, int height, bool force, bool bCubeMap,
                                TextureTarget texTarget, TextureFormat texFormat, TextureType texType, const TextureData &texData) const {
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
		texInfo.type = texType;
		texInfo.usage = TextureUsage_AttachmentColor | TextureUsage_RendererOutput;
		texInfo.multiSample = bMultisample;
		texInfo.useMipmaps = false;

		SamplerInfo smInfo{};
		smInfo.filterMag = Filter_LINEAR;
		smInfo.filterMin = Filter_LINEAR;

		smInfo.wrapR = Wrap_MIRRORED_REPEAT;
		smInfo.wrapS = Wrap_MIRRORED_REPEAT;
		smInfo.wrapT = Wrap_MIRRORED_REPEAT;

		outBuffer = createTexture(texInfo, smInfo, texData);

		return true;;
	}
	return false;
}

bool Renderer::setupDepthBuffer(std::shared_ptr<Texture> &outBuffer, bool multiSample, bool force) const {
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
		return true;
	}
	return false;
}

void Renderer::setupShadowMapBuffer(std::shared_ptr<Texture> &outBuffer, int width, int height, bool multiSample,
	bool bCubeMap, bool force) const {
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


void Renderer::drawDebugPoint(const glm::vec3 &pos, float persistTime, float pointSize) {
	std::lock_guard<std::mutex> guard(debug_point_task_lock_);
	auto&& point = std::make_shared<FPoint>(pos, persistTime);
	point->pointSize_ = pointSize;;
	debug_point_tasks_[point->getUUID()] = point;
}


void Renderer::drawDebugLine(const glm::vec3 &start, const glm::vec3 &end, float persistTime) {
	std::lock_guard<std::mutex> guard(debug_line_task_lock_);
	auto&& line = std::make_shared<FLine>(start, end, persistTime);
	debug_line_tasks_[line->getUUID()] = line;
}

void Renderer::drawDebugTriangle(const Triangle &inTri, float persistTime) {
	std::lock_guard<std::mutex> guard(debug_triangle_task_lock_);
	auto&& triangle = std::make_shared<Triangle>(inTri);
	triangle->persistTime_ = persistTime;
	debug_triangle_tasks_[triangle->getUUID()] = triangle;
}

void Renderer::handleDebugs() {
	// handle points
	{
		std::lock_guard<std::mutex> guard(debug_point_task_lock_);
		for (auto &[_, point]: debug_point_tasks_) {
			drawDebugPoint_Impl(point);
		}
	}

	// handle lines
	{
		std::lock_guard<std::mutex> guard(debug_line_task_lock_);
		for (auto &[_, line]: debug_line_tasks_) {
			drawDebugLine_Impl(line);
		}
	}

	// handle triangles
	{
		std::lock_guard<std::mutex> guard(debug_triangle_task_lock_);
		for (auto &[_, triangle]: debug_triangle_tasks_) {
			drawDebugTriangle_Impl(triangle);
		}
	}

}

void Renderer::loadGlobalUniforms(const std::shared_ptr<UMesh> &mesh) const {
	const auto pmat = mesh->getMaterial();
	// if (!pmat) return;
	// uniforms will be loaded to shader when Material::use(ShaderPass) is called.
	pmat->setUniformBlock(modelUniformBlock_->name(), modelUniformBlock_);
	pmat->setUniformBlock(lightUniformBlock_->name(), lightUniformBlock_);
	pmat->setUniformBlock(shadowUniformBlock_->name(), shadowUniformBlock_);
	pmat->setUniformBlock(sceneUniformBlock_->name(), sceneUniformBlock_);
	if (auto&& shadowMap = shadowMapUniformSampler_.lock()) {
		pmat->setUniformSampler(shadowMap->name(), shadowMap);
	}
	if (auto&& envMap = envCubeMapUniformSampler_.lock()) {
		pmat->setUniformSampler(envMap->name(), envMap);
	}
	if (auto&& cubeShadow = shadowMapCubeUniformSampler_.lock()) {
		pmat->setUniformSampler(cubeShadow->name(), cubeShadow);
	}
	if (ssaoKernelUniformSampler_) {
		pmat->setUniformSampler(ssaoKernelUniformSampler_->name(), ssaoKernelUniformSampler_);
	}
	if (noiseUniformSampler_) {
		pmat->setUniformSampler(noiseUniformSampler_->name(), noiseUniformSampler_);
	}
	if (noiseUniformSampler_) {
		pmat->setUniformSampler(noiseUniformSampler_->name(), noiseUniformSampler_);
	}
	if (auto&& ssao = abientOcclusionUniformSampler_.lock()) {
		pmat->setUniformSampler(ssao->name(), ssao);
	}
}
;
void Renderer::loadGlobalUniforms(const Shader &program) const {
	program.setUniformBlock(modelUniformBlock_->name(), modelUniformBlock_);
	program.setUniformBlock(lightUniformBlock_->name(), lightUniformBlock_);
	program.setUniformBlock(shadowUniformBlock_->name(), shadowUniformBlock_);
	program.setUniformBlock(sceneUniformBlock_->name(), sceneUniformBlock_);
	if (auto&& envMap = envCubeMapUniformSampler_.lock()) {
		program.setUniformSampler(envMap->name(), envMap);
	}
	if (auto&& shadowMap = shadowMapUniformSampler_.lock()) {
		program.setUniformSampler(shadowMap->name(), shadowMap);
	}
	if (auto&& cubeShadow = shadowMapCubeUniformSampler_.lock()) {
		program.setUniformSampler(cubeShadow->name(), cubeShadow);
	}
	if (ssaoKernelUniformSampler_) {
		program.setUniformSampler(ssaoKernelUniformSampler_->name(), ssaoKernelUniformSampler_);
	}
	if (noiseUniformSampler_) {
		program.setUniformSampler(noiseUniformSampler_->name(), noiseUniformSampler_);
	}
	if (auto&& ssao = abientOcclusionUniformSampler_.lock()) {
		program.setUniformSampler(ssao->name(), ssao);
	}
}

void Renderer::updateMainUniformBlock(const std::shared_ptr<UMesh> &mesh, const std::shared_ptr<FCamera> &camera, const std::shared_ptr<ULight> &shadowLight) {
	auto&& config = Config::getInstance();

	// Model Uniform Block
	ModelUniformBlock modelBlock{};
	modelBlock.uModel = glm::mat4(1.f);
	if (mesh) {
		modelBlock.uModel = mesh->getWorldMatrix();
		modelBlock.uNormalToWorld = glm::transpose(glm::inverse(mesh->getWorldMatrix()));
		if (auto&& material = mesh->getMaterial()) {
			if (material->hasEmission()) {
				modelBlock.uUsePureEmission = true;
			}
		}
	}

	if (camera) {
		modelBlock.uProjection = camera->GetProjectionMatrix();
		modelBlock.uView = camera->GetViewMatrix();
		modelBlock.uViewPos = camera->position();;
		modelBlock.uNearPlaneCamera = camera->getNearPlane();
		modelBlock.uFarPlaneCamera = camera->getFarPlane();
	}
	// shadow mapping
	if ((mesh && mesh->castShadow() || !mesh) && config.bShadowMap && shadowLight) {
		auto&& shadowCamera = shadowLight->getLightCamera();
		auto&& shadowMapSampler = shadowLight->getShadowMap(*this)->getUniformSampler(*this);
		if (shadowLight->isPointLight()) {
			modelBlock.uUseShadowMap = false;
			modelBlock.uUseShadowMapCube = true;
			updateShadowCubeUniformBlock(shadowLight);
			shadowMapCubeUniformSampler_ = shadowMapSampler;
		}
		else {
			modelBlock.uUseShadowMap = true;
			modelBlock.uUseShadowMapCube = false;
			// model项将在shader中指定，方便gbuffer!!!!
			modelBlock.uShadowMapVP = shadowCamera->GetProjectionMatrix() * shadowCamera->GetViewMatrix() * glm::mat4(1.f);
			// set ShadowMap
			shadowMapUniformSampler_ = shadowMapSampler;
		}
	}
	else {
		modelBlock.uUseShadowMap = false;;
		modelBlock.uUseShadowMapCube = false;
	}
	// eviroment mapping
	auto&& scene = renderingScene_.lock();
	if (config.bSkybox && scene && scene->skybox_ && scene->skybox_->getMesh()) {
		envCubeMapUniformSampler_ = scene->skybox_->getMesh()->tryGetSkyboxSampler(*this);
		modelBlock.uUseEnvmap = true;
	}
	else {
		modelBlock.uUseEnvmap = false;
	}
	modelUniformBlock_->setData(&modelBlock, sizeof(ModelUniformBlock));

	// Scene Uniform Block
	SceneUniformBlock sceneBlock{};
	sceneBlock.uScreenWidth = width();
	sceneBlock.uScreenHeight = height();
	sceneBlock.uUseSSAO = config.bUseSSAO;
	sceneUniformBlock_->setData(&sceneBlock, sizeof(SceneUniformBlock));
};;

void Renderer::updateShadowCubeUniformBlock(const std::shared_ptr<ULight> &shadowLight) const {
	auto&& config = Config::getInstance();
	if (!config.bShadowMap) return;

	ShadowCubeUniformBlock block {};
	if (shadowLight) {
		auto&& lightCamera = shadowLight->getLightCamera();
		if (shadowLight->isPointLight()) {
			auto&& proj = lightCamera->GetProjectionMatrix();
			block.uShadowVPs[0] = proj * lightCamera->GetViewMatrix({1.f, 0.f, 0.f}, {0.f, -1.f, 0.f});		// positive x
			block.uShadowVPs[1] = proj * lightCamera->GetViewMatrix({-1.f, 0.f, 0.f}, {0.f, -1.f, 0.f});	// negative x
			block.uShadowVPs[2] = proj * lightCamera->GetViewMatrix({0.f, 1.f, 0.f}, {0.f, 0.f, 1.f});		// positive y
			block.uShadowVPs[3] = proj * lightCamera->GetViewMatrix({0.f, -1.f, 0.f}, {0.f, 0.f, -1.f});	// negative y
			block.uShadowVPs[4] = proj * lightCamera->GetViewMatrix({0.f, 0.f, 1.f}, {0.f, -1.f, 0.f});		// positive z
			block.uShadowVPs[5] = proj * lightCamera->GetViewMatrix({0.f, 0.f, -1.f}, {0.f, -1.f, 0.f});	// negative z
		}
		// far plane
		block.uFarPlaneCubeShadow = lightCamera->getFarPlane();
	}
	shadowUniformBlock_->setData(&block, sizeof(ShadowCubeUniformBlock));
}

void Renderer::updateLightUniformBlock(const std::shared_ptr<ULight>& light) const {
	// 现在可以确定内存布局没问题了，只能是c++端的问题，light初始化有问题
	LightDataUniformBlock tmp;
	if (light) {
		tmp = light->serialize();
	}
	else {
		tmp.uLightType = LightType_NoLight;;
	}
	lightUniformBlock_->setData(&tmp, sizeof(LightDataUniformBlock));
}

void Renderer::setViewPort(int x, int y, int width, int height) {
	viewer_x_ = x;
	viewer_y_ = y;
	viewer_width_ = width;
	viewer_height_ = height;
	setRenderViewPort(x, y, width, height);
}

void Renderer::restoreViewPort() {
	setRenderViewPort(viewer_x_, viewer_y_, viewer_width_, viewer_height_);
}

Renderer::~Renderer()
{
	// std::cout << "Base renderer here!" << std::endl;
}

std::shared_ptr<FCamera> Renderer::getCamera() const {
	return mainCamera_;
}

std::shared_ptr<FCamera> Renderer::getViewCamera() const {
	return viewCamera_;
}

void Renderer::setCamera(const std::shared_ptr<FCamera> &camera) {
	mainCamera_ = camera;
}

void Renderer::setBackToViewCamera() {
	setCamera(viewCamera_);
}

void Renderer::setRenderingScene(const std::shared_ptr<FScene> &scene) {
	renderingScene_ = scene;
}

int Renderer::width() const {
	return width_;
}

int Renderer::height() const {
	return height_;
}

void Renderer::remove_all_debug_primitives_safe() {
	{
		std::lock_guard<std::mutex> guard(debug_point_task_lock_);
		debug_point_tasks_.clear();
	}
	{
		std::lock_guard<std::mutex> guard(debug_line_task_lock_);
		debug_point_tasks_.clear();
	}
	{
		std::lock_guard<std::mutex> guard(debug_triangle_task_lock_);
		debug_point_tasks_.clear();
	}
}

void Renderer::remove_point_debug_task_safe(int uuid) {
	std::lock_guard<std::mutex> guard(debug_point_task_lock_);
	debug_point_tasks_.erase(uuid);
}

void Renderer::remove_line_debug_task_safe(int uuid) {
	std::lock_guard<std::mutex> guard(debug_line_task_lock_);
	debug_line_tasks_.erase(uuid);
}

void Renderer::remove_triangle_debug_task_safe(int uuid) {
	std::lock_guard<std::mutex> guard(debug_triangle_task_lock_);
	debug_triangle_tasks_.erase(uuid);
}


