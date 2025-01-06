#include "Viewer.h"

#include <Base/Config.h>
#include <omp.h>
#include <thread>

#include "app.h"
#include "Base/Globals.h"
#include "RenderPass/RenderPassGeometry.h"
#include "RenderPass/RenderPassLight.h"
#include "RenderPass/RenderPassShadow.h"
#include "RenderPass/RenderPassForwardShading.h"
#include "FCamera.h"
#include "ULight.h"
#include "Utils/SRKLogger.h"
#include "Renderer.h"
#include "FScene.h"
#include "ConfigPanel.h"
#include "OrbitController.h"
#include "GLFW/glfw3.h"
#include "OpenGL/RendererOpenGL.h"
#include "Utils/ImageUtils.h"

void Viewer::init(void *window, int width, int height, int outTexId)
{
	cleanup();

	glfwWindow_ = static_cast<GLFWwindow*>(window);

	auto && config = Config::getInstance();

	if (!configPanel_) {
		configPanel_ = std::make_shared<ConfigPanel>();
		configPanel_->init(window, width, height);
	}

	width_ = width;
	height_ = height;
	outTexId_ = outTexId;

	// depth camera
	if (!cameraDepth_) {
		cameraDepth_ = std::make_shared<OrthographicCamera>();
	}

	if (!cameraMain_) {
		cameraMain_ = std::make_shared<PerspectiveCamera>(config.CameraFOV, static_cast<float>(config.WindowWidth) / config.WindowHeight);
	}

	if (!orbitController_) {
		orbitController_ = std::make_shared<OrbitController>(*cameraMain_);
	}

	// renderer
	if (!renderer_) {
		auto&& ret = createRenderer();
		renderer_ = ret;
	}

	if (!renderer_) {
		LOGE("Viewer::create failed: createRenderer error");
	}

	/** scene */
	reloadScene();

	setViewPort(0, 0, width_, height_);

	// passes
	plainPass_ = std::make_shared<RenderPassForwardShading>(renderer_);
	plainPass_->init();

	geometryPass_ = std::make_shared<RenderPassGeometry>(renderer_);
	geometryPass_->init();

	lightPass_ = nullptr;
	lightPass_ = std::make_shared<RenderPassLight>(renderer_);
	lightPass_->init();

	shadowPass_ = std::make_shared<RenderPassShadow>(renderer_);
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

void Viewer::reloadScene() {
	auto&& config = Config::getInstance();
	if (scene_ && scene_->sceneType_ == config.SceneType) return;

	switch (config.SceneType) {
		case SceneType_Default: {
			scene_ = FScene::generateDeaultScene(cameraMain_);
			scene_->sceneType_ = SceneType_Default;
			break;
		}
		case SceneType_StandfordBunny: {
			scene_ = FScene::generateRaytracingStanfordBunnyScene(cameraMain_);
			scene_->sceneType_ = SceneType_StandfordBunny;
			break;
		}
		case SceneType_PBRTesting: {
			scene_ = FScene::generatePBRScene(cameraMain_);
			scene_->sceneType_ = SceneType_PBRTesting;
			break;
		}
		default: scene_ = FScene::generateDeaultScene(cameraMain_);
		break;
	}
	cameraMain_->lookAt(scene_->getFocus());
}

void Viewer::DrawFrame() {
	cleanup();
	if (scene_) {
		// pre setup
		auto&& config = Config::getInstance();
		if (renderer_->render_mode_ != config.RenderPipeline || scene_->sceneType_ != config.SceneType) {
			// switch render mode, clear all debug primitives
			renderer_->remove_all_debug_primitives_safe();
		}
		renderer_->render_mode_ = config.RenderPipeline;
		reloadScene();
		renderer_->setRenderingScene(scene_);
		scene_->packMeshesFromScene();
		// if (camera_mode_ == CameraMode_OrbitCamera) {
		// 	if (bOrbitCenterDirty) {
		// 		orbitController_->recalculateCenter();
		// 		bOrbitCenterDirty = false;
		// 	}
		// 	orbitController_->update();
		// }
		// else if (camera_mode_ == CameraMode_FPSCamera) {
		// 	bOrbitCenterDirty = true;
		// }

		// pipeline
		if (config.RenderPipeline == ERenderPipeline::RenderMode_ForwardRendering) {
			drawScene_ForwardRendering(scene_);
		}
		else if (config.RenderPipeline == ERenderPipeline::RenderMode_DeferredRendering) {
			drawScene_DefferedRendering(scene_);
		}
		else if (config.RenderPipeline == ERenderPipeline::RenderMode_TestRendering_OffScreen) {
			drawScene_TestPipeline(scene_);
		}
		else if (config.RenderPipeline == ERenderPipeline::RenderMode_TestRendering_OnScreen) {
			drawScene_OnScreen(scene_);
		}
		else if (config.RenderPipeline == ERenderPipeline::RenderMode_PathTracing) {
			TEST_TIME_COST(drawScene_PathTracing_CPU(scene_), Ray_Tracing);
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

		drawPanel();
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

std::shared_ptr<FCamera> Viewer::getViewCamera() const {return cameraMain_; }

std::shared_ptr<Renderer> Viewer::createRenderer() {
	auto&& config = Config::getInstance();
	ERendererType rendererType = config.RendererType;

	switch (rendererType) {
		case RendererType_SOFT: {
			return {};
			break;
		}
		case RendererType_OPENGL: {
			auto&& renderer = std::make_shared<RendererOpenGL>(cameraMain_);
			renderer->init();
			return renderer;
			break;
		}
		case RendererType_Vulkan: {
			return {};
			break;
		}
		default: break;
	}
	return {};
}

void Viewer::toggleShowConfigPanel() { bShowConfigPanel_ = !bShowConfigPanel_; }

void Viewer::drawPanel() const {
	if (bShowConfigPanel_) {
		configPanel_->onDraw();
	}
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
		clearStatesForwardingPass.clearColor = clearColor;
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
	ClearStates clear_states;
	clear_states.colorFlag = true;
	renderer_->beginRenderPass(0, clear_states);

	static int counter = 0;
	if (counter == 0) {
		int NumSamples = 1024;
		// halton
		// for (int i = 0; i < NumSamples; i++) {
		// 	renderer_->drawDebugPoint(glm::vec3(MathUtils::Halton(0, i), MathUtils::Halton(1, i), 1.f), 0.f, 10.f);
		// }

		// Hammersley
		// for (int i = 0; i < NumSamples; i++) {
		// 	renderer_->drawDebugPoint(glm::vec3(MathUtils::Hammersley(0, i, NumSamples), MathUtils::Hammersley(1, i, NumSamples), 1.f), 0.f, 5.f);
		// }

		// Sobol
		// for (int i = 0; i < NumSamples; i++) {
		// 	renderer_->drawDebugPoint(
		// 		glm::vec3(
		// 			MathUtils::Sobol(0, i),
		// 			MathUtils::Sobol(1, i),
		// 			1.f),
		// 			0.f, 5.f);
		// }

		// real random
		// for (int i = 0; i < NumSamples; i++) {
		// 	renderer_->drawDebugPoint(glm::vec3(MathUtils::get_random_float(), MathUtils::get_random_float(), 1.f), 0.f, 5.f);
		// }

		for (int i = 0; i < NumSamples; i++) {
			float pdf;
			glm::vec3 wo;
			MathUtils::UniformHemisphereSampleByVolume(wo, pdf, {}, {0.f, 1.f, 0.f}, true, 0, MathUtils::grayCode(i));
			renderer_->drawDebugPoint(wo, 0.f, 5.f);
		}

		counter++;
	}

	renderer_->endRenderPass();
}

void Viewer::drawScene_OnScreen(const std::shared_ptr<FScene> &scene) const {
}

void Viewer::drawScene_PathTracing_CPU(const std::shared_ptr<FScene> &scene) const {

	int width = renderer_->width(), height = renderer_->height();
	auto&& framebuffer = Buffer<glm::vec4>::makeBuffer(width, height, glm::vec4(0.f));
	auto&& config = Config::getInstance();
	int spp = config.SPP;;

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


		glm::vec4& pixel = framebuffer->getPixelRef(i, j);
		for (int k = 0; k < spp; k++){
			// generate primary ray direction
			auto&& ray = screenToWorldRay(i, j, height, width, cameraMain_, false, 0.5f, p * spp + k);
			pixel += glm::vec4(scene->castRay(ray, 0, p * spp + k, 0.8f) * 255.f, 255.f) / float(spp);
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
                             disturbRadius, int SobolIndex) const {

	glm::vec3 eye = cameraMain_->position();
	glm::mat4 cameraToWorld = glm::inverse(cameraMain_->GetViewMatrix());

	float tanHalfFOV = tan(glm::radians(cameraMain_->getFOV() * 0.5));
	float aspect = cameraMain_->getAspect();

	float x_offset, y_offset;
	if (bUseDisturb) {
		// x_offset = MathUtils::get_random_float(0.5f - disturbRadius, 0.5f + disturbRadius);
		// y_offset = MathUtils::get_random_float(0.5f - disturbRadius, 0.5f + disturbRadius);
		x_offset = 0.5f + (MathUtils::Sobol(0, SobolIndex) * 2.f - 1.f) * disturbRadius;
		y_offset = 0.5f + (MathUtils::Sobol(1, SobolIndex) * 2.f - 1.f) * disturbRadius;
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

void Viewer::listenKeyEvents() {
	auto deltaFrameTime = App::getDeltaTime();
	if (glfwGetKey(glfwWindow_, GLFW_KEY_W) == GLFW_PRESS)
		cameraMain_->ProcessKeyboard(FORWARD, deltaFrameTime);
	if (glfwGetKey(glfwWindow_, GLFW_KEY_S) == GLFW_PRESS)
		cameraMain_->ProcessKeyboard(BACKWARD, deltaFrameTime);
	if (glfwGetKey(glfwWindow_, GLFW_KEY_A) == GLFW_PRESS)
		cameraMain_->ProcessKeyboard(LEFT, deltaFrameTime);
	if (glfwGetKey(glfwWindow_, GLFW_KEY_D) == GLFW_PRESS)
		cameraMain_->ProcessKeyboard(RIGHT, deltaFrameTime);

	if (glfwGetKey(glfwWindow_, GLFW_KEY_SPACE) == GLFW_PRESS) {
		cameraMain_->lookAt({0.f, 0.f, 0.f});
	}
}

void Viewer::setCameraMode(ECameraMode camera_mode) {
	camera_mode_ = camera_mode;
	switch (camera_mode_) {
		case CameraMode_None: {
			glfwSetInputMode(glfwWindow_, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
			break;
		}
		case CameraMode_OrbitCamera: {
			glfwSetInputMode(glfwWindow_, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
			break;
		}
		case CameraMode_FPSCamera: {
			glfwSetInputMode(glfwWindow_, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
			break;
		}
		default: break;
	}
}

void Viewer::updateOrbitZoom(float x, float y) {
	orbitController_->zoomX = x;
	orbitController_->zoomY = y;
}

void Viewer::updateOrbitRotate(float x, float y) {
	orbitController_->rotateX = x;
	orbitController_->rotateY = y;
}

void Viewer::updateOrbitPan(float x, float y) {
	orbitController_->panX = x;
	orbitController_->panY = y;
}

bool Viewer::wantCaptureKeyboard() const { return configPanel_->wantCaptureKeyboard(); }

bool Viewer::wantCaptureMouse() const { return configPanel_->wantCaptureMouse(); }

void Viewer::waitIdle() {
}

void Viewer::drawScene_DefferedRendering(const std::shared_ptr<FScene>& scene) {
	/**
	 * Beginning
	 */
	{
		ClearStates clearStates;
		clearStates.clearColor = clearColor;
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
		clearStateGeometryPass.clearColor = clearColor;
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

	// lightPass_->injectGeometryPass(geometryPass_);
	// lightPass_->renderGBuffersToScreen();

	/*
	* ToScreenPass
	*/
	// ClearStates clearStatesToScreenPass;
	// clearStatesToScreenPass.clearColor = clearColor;
	// clearStatesToScreenPass.colorFlag = true;
	// clearStatesToScreenPass.depthFlag = true;
	//
	// renderer_->beginRenderPass(nullptr, clearStatesToScreenPass);
	// renderer_->dump(renderer_->getToScreenDepthProgram(geometryPass_->getSSAOResult()), false, nullptr, 1);
	// renderer_->endRenderPass();
	/**
	 * Light Pass
	 */
	if (lightPass_ && geometryPass_) {

		// inject geometry pass
		lightPass_->injectGeometryPass(geometryPass_);

		ClearStates clearStatesLightPass;
		clearStatesLightPass.clearColor = clearColor; // 防止blend的时候blend进背景色
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
