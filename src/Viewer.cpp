#include "Viewer.h"

#include <Base/Config.h>

#include "RenderPass/RenderPassGeometry.h"
#include "RenderPass/RenderPassLight.h"

#include "FCamera.h"
#include "Light.h"
#include "Utils/Logger.h"
#include "Renderer.h"
#include "RenderPass/RenderPassForwardShading.h"
#include "../include/Geometry/Model.h"
#include "FScene.h"
#include "OpenGL/RendererOpenGL.h"
#include "RenderPass/RenderPassShadow.h"

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

void Viewer::cleanup()
{
	if (renderer_) {
		renderer_->waitIdle();
	}
}

void Viewer::render(const std::shared_ptr<FScene> &scene) {
	auto&& config = Config::getInstance();
	renderer_->render_mode = config.RenderMode;
	renderer_->setRenderingScene(scene);

	if (config.RenderMode == ERenderMode::RenderMode_ForwardRendering) {
		drawScene_ForwardRendering(scene);
	}
	else if (config.RenderMode == ERenderMode::RenderMode_DefferedRendering) {
		drawScene_DefferedRendering(scene);
	}
	else if (config.RenderMode == ERenderMode::RenderMode_TestRendering_OffScreen) {
		drawScene_TestPipeline(scene);
	}
	else if (config.RenderMode == ERenderMode::RenderMode_TestRendering_OnScreen) {
		drawScene_OnScreen(scene);
	}
	else {
		drawScene_ForwardRendering(scene);
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
	/*
	 * ShadowPass
	 */
	if (Config::getInstance().bShadowMap)
	{
		ClearStates ClearStatsShadowPass;
		ClearStatsShadowPass.clearColor = clearColor;
		ClearStatsShadowPass.colorFlag = true;
		ClearStatsShadowPass.depthFlag = true;

		auto&& renderStates = renderer_->renderStates;
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

		auto&& renderStates = renderer_->renderStates;
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
	renderer_->endRenderPass();
}

void Viewer::drawScene_TestPipeline(const std::shared_ptr<FScene> &scene) const {
	/*
	* ShadowPass
	*/
	{
		ClearStates ClearStatsShadowPass;
		ClearStatsShadowPass.clearColor = clearColor;
		ClearStatsShadowPass.colorFlag = true;
		ClearStatsShadowPass.depthFlag = true;

		auto&& renderStates = renderer_->renderStates;
		renderStates.blend = false;
		renderStates.depthMask = true;
		renderStates.depthTest = true;
		renderer_->updateRenderStates(renderStates);

		renderer_->beginRenderPass(shadowPass_->getFramebufferMain(), ClearStatsShadowPass);
		renderer_->executeRenderPass(shadowPass_, *scene);
		renderer_->endRenderPass();
	}

	// /*
	//  * Forwarding Pass
	//  */
	//
	// {
	// 	ClearStates clearStatesForwardingPass;
	// 	clearStatesForwardingPass.clearColor = BLACK_COLOR;
	// 	clearStatesForwardingPass.colorFlag = true;
	// 	clearStatesForwardingPass.depthFlag = true;
	//
	// 	auto&& renderStates = renderer_->renderStates;
	// 	renderStates.blend = true;
	// 	renderStates.depthMask = true;
	// 	renderStates.depthTest = true;
	// 	renderStates.cullFace = true;
	// 	renderer_->updateRenderStates(renderStates); // 有可能上一帧关闭了DepthMask，所以每个renderPass开始前一定要检查状态
	//
	// 	renderer_->beginRenderPass(plainPass_->getFramebufferMain(), clearStatesForwardingPass);
	// 	renderer_->executeRenderPass(plainPass_, *scene);
	// 	renderer_->endRenderPass();
	// }
	if (!scene->getLights().empty()) {
		ClearStates ClearStatsShadowPass;
		ClearStatsShadowPass.clearColor = clearColor;
		ClearStatsShadowPass.colorFlag = true;
		ClearStatsShadowPass.depthFlag = true;

		auto&& renderStates = renderer_->renderStates;
		renderStates.blend = false;
		renderStates.depthMask = true;
		renderStates.depthTest = true;
		renderer_->updateRenderStates(renderStates);

		// to screen
		renderer_->beginRenderPass(nullptr, ClearStatsShadowPass);
		auto&& srcTex = scene->getLights()[0]->getShadowMap(*renderer_);
		auto&& cubeShadowProgram = renderer_->getToScreenCubeDepthProgram(srcTex);
		for (auto&& model: scene->getModels()) {
			renderer_->draw(model, {}, {}, cubeShadowProgram);
		}
		if (scene->skybox_) {
			renderer_->draw(scene->skybox_, {}, {}, cubeShadowProgram);
		}

		for (auto&& light: scene->getLights()) {
			renderer_->draw(scene->skybox_, ShaderPass::Shader_ForwardShading_Pass, {}, cubeShadowProgram);
		}
	}

	/*
	* ToScreenPass
	*/
	// renderer_->dump(plainPass_->getOutTex(), true, false, nullptr, 0, false, {});
	// renderer_->endRenderPass();
}

void Viewer::drawScene_OnScreen(const std::shared_ptr<FScene> &scene) const {

}

void Viewer::drawScene_DefferedRendering(std::shared_ptr<FScene> scene) {
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

		auto&& renderStates = renderer_->renderStates;
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

		auto&& renderStates = renderer_->renderStates;
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

		auto&& renderStates = renderer_->renderStates;
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
