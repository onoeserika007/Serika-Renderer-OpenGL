#include "Viewer.h"

#include <Base/Config.h>
#include <omp.h>

#include "RenderPass/RenderPassGeometry.h"
#include "RenderPass/RenderPassLight.h"

#include "FCamera.h"
#include "ULight.h"
#include "Utils/Logger.h"
#include "Renderer.h"
#include "RenderPass/RenderPassForwardShading.h"
#include "FScene.h"
#include "OpenGL/RendererOpenGL.h"
#include "RenderPass/RenderPassShadow.h"
#include "Utils/ImageUtils.h"

Viewer::Viewer(const std::shared_ptr<FCamera>& camera) : cameraMain_(camera) {
}

Viewer::~Viewer() {
	// 程序退出时出现卡顿是因为出现了shared_ptr的相互引用！！
	// lightPass_引用geometryPass_时应该使用weak_ptr
	cameraDepth_ = nullptr;
	renderer_ = nullptr;
	plainPass_ = nullptr;
	geometryPass_ = nullptr;
	lightPass_ = nullptr;
	// 好像也不是这个问题，将~Viewer声明为虚函数就解决了？
}

void Viewer::init(int width, int height, int outTexId)
{
	cleanup();

	width_ = width;
	height_ = height;
	outTexId_ = outTexId;

	// depth camera
	if (!cameraDepth_) {
		cameraDepth_ = std::make_shared<OrthographicCamera>();
	}

	// renderer
	if (!renderer_) {
		renderer_ = createRenderer();
	}
	if (!renderer_) {
		LOGE("Viewer::create failed: createRenderer error");
	}

	setViewPort(0, 0, width_, height_);

	// passes
	plainPass_ = std::make_shared<RenderPassForwardShading>(*renderer_);
	plainPass_->init();

	geometryPass_ = std::make_shared<RenderPassGeometry>(*renderer_);
	geometryPass_->init();

	lightPass_ = std::make_shared<RenderPassLight>(*renderer_);
	lightPass_->init();

	shadowPass_ = std::make_shared<RenderPassShadow>(*renderer_);
	shadowPass_->init();
}

void Viewer::setViewPort(int x, int y, int width, int height)
{
	width_ = width;
	height_ = height;
	if (renderer_) {
		renderer_->setViewPort(x, y, width, height);
	}
}

void Viewer::setScene(const std::shared_ptr<FScene> &scene) { scene_ = scene; }

void Viewer::cleanup()
{
	if (renderer_) {
		renderer_->waitIdle();
	}
}

void Viewer::DrawFrame() {
	if (scene_) {
		// pre setup
		auto&& config = Config::getInstance();
		renderer_->render_mode_ = config.RenderMode;
		renderer_->setRenderingScene(scene_);
		scene_->packMeshesFromScene();

		// pipeline
		if (config.RenderMode == ERenderMode::RenderMode_ForwardRendering) {
			drawScene_ForwardRendering(scene_);
		}
		else if (config.RenderMode == ERenderMode::RenderMode_DefferedRendering) {
			drawScene_DefferedRendering(scene_);
		}
		else if (config.RenderMode == ERenderMode::RenderMode_TestRendering_OffScreen) {
			drawScene_TestPipeline(scene_);
		}
		else if (config.RenderMode == ERenderMode::RenderMode_TestRendering_OnScreen) {
			drawScene_OnScreen(scene_);
		}
		else if (config.RenderMode == ERenderMode::RenderMode_PathTracing) {
			drawScene_PathTracing_CPU(scene_);
		}
		else {
			drawScene_ForwardRendering(scene_);
		}

		// post drawing
		// 最后画debug bbox
		// if (scene_) {
		// 	auto&& renderStates = renderer_->renderStates_;
		// 	renderStates.blend = false;
		// 	renderStates.depthMask = false;
		// 	renderStates.depthTest = false;
		// 	renderStates.cullFace = false;
		// 	renderer_->updateRenderStates(renderStates);
		//
		// 	// auto&& BVH2 = scene->bvh_accel_;
		// 	auto&& BVH = scene_->getBVH();
		// 	auto&& root = BVH->getRoot();
		// 	renderer_->drawDebugBBoxes(root, 0, {});
		// }
		renderer_->drawWorldAxis({});
		renderer_->handleDebugs();
		// renderer_->endRenderPass();
	}
}


std::shared_ptr<FCamera> Viewer::createCamera(CameraType type)
{
	if (type == CameraType::PERSPECTIVE) {
		return std::make_shared<PerspectiveCamera>();
	}
	else if (type == CameraType::ORTHOGRAPHIC) {
		return std::make_shared<OrthographicCamera>();
	}
	return nullptr;
}

std::shared_ptr<Renderer> Viewer::createRenderer() {
	auto&& config = Config::getInstance();
	std::shared_ptr<Renderer> renderer;
	switch (config.RendererType) {
		case RendererType_SOFT: {
			break;
		}
		case RendererType_OPENGL: {
			renderer = std::make_shared<RendererOpenGL>(cameraMain_);
			renderer->init();
			break;
		}
		case RendererType_Vulkan: {
			break;
		}
		default: break;
	}
	return renderer;
}


void Viewer::drawScene_ForwardRendering(const std::shared_ptr<FScene> &scene) const {

	// clear framebuffer 0
	/*
	 * ShadowPass
	 */
	if (Config::getInstance().bShadowMap)
	{
		ClearStates ClearStatsShadowPass;
		ClearStatsShadowPass.clearColor = clearColor;
		ClearStatsShadowPass.colorFlag = true;
		ClearStatsShadowPass.depthFlag = true;

		auto&& renderStates = renderer_->renderStates_;
		renderStates.blend = false;
		renderStates.depthMask = true;
		renderStates.depthTest = true;
		renderer_->updateRenderStates(renderStates);

		renderer_->beginRenderPass(shadowPass_->getFramebufferMain(), ClearStatsShadowPass);
		renderer_->executeRenderPass(shadowPass_, *scene);
		renderer_->endRenderPass();
	}

	/*
	 * Forwarding Pass
	 */

	{
		ClearStates clearStatesForwardingPass;
		clearStatesForwardingPass.clearColor = BLACK_COLOR;
		clearStatesForwardingPass.colorFlag = true;
		clearStatesForwardingPass.depthFlag = true;

		auto&& renderStates = renderer_->renderStates_;
		renderStates.blend = true;
		renderStates.depthMask = true;
		renderStates.depthTest = true;
		renderStates.cullFace = true;
		renderer_->updateRenderStates(renderStates); // 有可能上一帧关闭了DepthMask，所以每个renderPass开始前一定要检查状态

		renderer_->beginRenderPass(plainPass_->getFramebufferMain(), clearStatesForwardingPass);
		renderer_->executeRenderPass(plainPass_, *scene);
		renderer_->endRenderPass();
	}

	/*
	* ToScreenPass
	*/
	auto&& toScreenProgram = renderer_->getToScreenColorProgram(plainPass_->getOutTex());
	renderer_->dump(toScreenProgram, false, nullptr, 0);
}

void Viewer::drawScene_TestPipeline(const std::shared_ptr<FScene> &scene) const {
}

void Viewer::drawScene_OnScreen(const std::shared_ptr<FScene> &scene) const {
}

void Viewer::drawScene_PathTracing_CPU(const std::shared_ptr<FScene> &scene) const {

	int width = renderer_->width(), height = renderer_->height();
	auto&& framebuffer = Buffer<glm::vec4>::makeBuffer(width, height, glm::vec4(0.f));
	int spp = 128;

	std::cout << "SPP: " << spp << "\n";
	uint32_t cpuNum= std::thread::hardware_concurrency();

	std::cout<<"Cpu Num :" <<cpuNum<<std::endl;

	omp_set_num_threads(cpuNum);
	int hxw= framebuffer->size();

	// std::atomic<float> progress = 0.f;
	glm::mat4 cameraToWorld = glm::transpose(cameraMain_->GetViewMatrix());

	#pragma omp parallel for
	for (int p = 0; p < hxw; ++p) {
		int i= p % height;
		int j= p / height;

		// generate primary ray direction
		auto&& ray = screenToWorldRay(i, j, height, width, cameraMain_, false);

		// glm::vec3 eye = cameraMain_->position();
		// glm::mat4 cameraToWorld = glm::transpose(cameraMain_->GetViewMatrix());
		//
		// float tanHalfFOV = tan(glm::radians(cameraMain_->getFOV() * 0.5));
		// float aspect = cameraMain_->getAspect();
		//
		// float x_offset = MathUtils::get_random_float(0.5f - M_EPSILON, 0.5f + M_EPSILON); // do msaa
		// float y_offset = MathUtils::get_random_float(0.5f - M_EPSILON, 0.5f + M_EPSILON);
		// float x = (2 * (i + x_offset) / (float)width - 1) *
		// 		aspect * tanHalfFOV;
		// float y = (1 - 2 * (j + y_offset) / (float)height) * tanHalfFOV;
		//
		// glm::vec3 dir = glm::normalize(glm::vec3(-x, y, 1));
		// Ray ray {cameraMain_->position(), dir};

		glm::vec4& pixel = framebuffer->getPixelRef(i, j);
		for (int k = 0; k < spp; k++){
			pixel += glm::vec4(scene->castRay(ray, 0) * 255.f, 255.f) / float(spp);
		}

		// progress = progress + i * j / float(hxw);
	}

	for (int i = 0; i < width; i++) {
		for (int j = 0; j < height; j++) {
			auto&& pixel = framebuffer->getPixelRef(i, j);
			pixel = glm::clamp(pixel, glm::vec4(0.f, 0.f, 0.f, 255.f), glm::vec4(255.f, 255.f, 255.f, 255.f));
		}
	}

	auto&& outImg = Buffer<RGBA>::makeBuffer();
	outImg->copyFrom(*framebuffer);
	ImageUtils::writeImage("./raycasting_output.png", outImg->width(), outImg->height(), 4,  outImg->rawData(), 4 * outImg->width(), false);
	exit(0);
}

Ray Viewer::screenToWorldRay(int mouseX, int mouseY, int screenWidth, int screenHeight, const std::shared_ptr<FCamera> &camera, bool bUseDisturb, float
                             disturbRadius) const {

	glm::vec3 eye = cameraMain_->position();
	glm::mat4 cameraToWorld = glm::transpose(cameraMain_->GetViewMatrix());

	float tanHalfFOV = tan(glm::radians(cameraMain_->getFOV() * 0.5));
	float aspect = cameraMain_->getAspect();

	float x_offset, y_offset;
	if (bUseDisturb) {
		x_offset = MathUtils::get_random_float(0.5f - disturbRadius, 0.5f + disturbRadius);
		y_offset = MathUtils::get_random_float(0.5f - disturbRadius, 0.5f + disturbRadius);
	}
	else {
		x_offset = y_offset = 0.5f;
	}

	float x = (2 * (mouseX + x_offset) / (float)screenWidth - 1) * aspect * tanHalfFOV * cameraMain_->getNearPlane(); // mapping o-width to 0-2, then -1-1
	float y = (1 - 2 * (mouseY + y_offset) / (float)screenHeight) * tanHalfFOV * cameraMain_->getNearPlane();
	glm::vec3 localFocus {x, y, -cameraMain_->getNearPlane()};

	glm::vec3 dir = glm::normalize(glm::vec3(cameraToWorld * glm::vec4(localFocus, 0.f)));
	return {eye, dir};
}

void Viewer::drawCursorHitDebugLine(int mouseX, int mouseY, int screenWidth, int screenHeight) {
	auto&& ray = screenToWorldRay(mouseX, mouseY, screenWidth, screenHeight, cameraMain_);
	renderer_->drawDebugLine(ray.origin, ray.origin + ray.direction * 10.f, 5.f);
}

void Viewer::drawUnderCursorTraceDebugTriangle(int mouseX, int mouseY, int screenWidth, int screenHeight) {
	auto&& ray = screenToWorldRay(mouseX, mouseY, screenWidth, screenHeight, cameraMain_);
	auto inters = scene_->intersect(ray);
	if (inters.bHit) {
		auto && prim = inters.primitive.lock();
		if (auto&& tri = std::dynamic_pointer_cast<Triangle>(prim)) {
			renderer_->drawDebugTriangle(*tri, 5.f);
		}
		else {
			LOGD("Hit but not hitting a Triangle, please retry!");
		}
	}
	else {
		LOGD("Did not hit any object, please retry!");
	}
}

void Viewer::drawScene_DefferedRendering(const std::shared_ptr<FScene>& scene) {
	/**
	 * Beginning
	 */
	{
		ClearStates clearStates;
		clearStates.clearColor = BLACK_COLOR;
		clearStates.colorFlag = true;
		clearStates.depthFlag = true;

		renderer_->beginRenderPass(nullptr, clearStates);
		renderer_->endRenderPass();
	}

	/**
	 * Shadow Pass
	 */
	{
		ClearStates ClearStatsShadowPass;
		ClearStatsShadowPass.clearColor = clearColor;
		ClearStatsShadowPass.colorFlag = true;
		ClearStatsShadowPass.depthFlag = true;

		auto&& renderStates = renderer_->renderStates_;
		renderStates.blend = false;
		renderStates.depthMask = true;
		renderStates.depthTest = true;
		renderer_->updateRenderStates(renderStates);

		renderer_->beginRenderPass(shadowPass_->getFramebufferMain(), ClearStatsShadowPass);
		renderer_->executeRenderPass(shadowPass_, *scene);
		renderer_->endRenderPass();
	}

	/*
	 * GeometryPass
	 */
	if (geometryPass_) {
		ClearStates clearStateGeometryPass;
		clearStateGeometryPass.clearColor = BLACK_COLOR;
		clearStateGeometryPass.colorFlag = true;
		clearStateGeometryPass.depthFlag = true;

		auto&& renderStates = renderer_->renderStates_;
		renderStates.blend = false;
		renderStates.depthMask = true;
		renderStates.depthTest = true;
		renderer_->updateRenderStates(renderStates);

		renderer_->beginRenderPass(geometryPass_->getFramebufferMain(), clearStateGeometryPass);
		renderer_->executeRenderPass(geometryPass_, *scene); // please make sure adding public to inheritance explicitly to use "is-a" inheritance.
		renderer_->endRenderPass();
	}

	/**
	 * Light Pass
	 */
	if (lightPass_ && geometryPass_) {

		// inject geometry pass
		lightPass_->injectGeometryPass(geometryPass_);

		ClearStates clearStatesLightPass;
		clearStatesLightPass.clearColor = BLACK_COLOR; // 防止blend的时候blend进背景色
		clearStatesLightPass.colorFlag = true;
		clearStatesLightPass.depthFlag = false;

		BlendParameters blendParams;
		blendParams.SetBlendFactor(BlendFactor_ONE, BlendFactor_ONE);
		blendParams.SetBlendFunc(BlendFunc_ADD);

		auto&& renderStates = renderer_->renderStates_;
		renderStates.blend = true;
		renderStates.blendParams = blendParams;
		renderStates.depthMask = true;
		renderStates.depthTest = true;
		renderer_->updateRenderStates(renderStates);

		renderer_->beginRenderPass(lightPass_->getFramebufferMain(), clearStatesLightPass);
		// lightPass_->renderGBuffersToScreen(geometryPass_->getFramebufferMain());
		renderer_->executeRenderPass(lightPass_, *scene);
		renderer_->endRenderPass();

		/*
		* ToScreenPass
		*/
		ClearStates clearStatesToScreenPass;
		clearStatesToScreenPass.clearColor = clearColor;
		clearStatesToScreenPass.colorFlag = true;
		clearStatesToScreenPass.depthFlag = true;

		renderer_->beginRenderPass(nullptr, clearStatesToScreenPass);
		renderer_->dump(renderer_->getToScreenColorProgram(lightPass_->getOutTex()), false, nullptr, 1);
		renderer_->endRenderPass();
	}
}
