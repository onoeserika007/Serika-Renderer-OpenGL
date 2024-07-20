#include "Viewer.h"

#include <Base/Config.h>

#include "RenderPass/RenderPassGeometry.h"
#include "RenderPass/RenderPassLight.h"

#include "Camera.h"
#include "Light.h"
#include "Utils/Logger.h"
#include "Texture.h"
#include "Renderer.h"
#include "RenderPass/RenderPassPlain.h"
#include "../include/Geometry/Model.h"
#include "Scene.h"
#include "RenderPass/RenderPassShadow.h"
#include "Utils/OpenGLUtils.h"

Viewer::Viewer(const std::shared_ptr<Camera>& camera) : cameraMain_(camera) {
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
	plainPass_ = std::make_shared<RenderPassPlain>(*renderer_);
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

void Viewer::render(std::shared_ptr<Scene> scene) {
	auto&& config = Config::getInstance();
	if (config.RenderMode == ERenderMode::RenderMode_ForwardRendering) {
		drawScene(scene);
	}
	else if (config.RenderMode == ERenderMode::RenderMode_DefferedRendering) {
		drawScene_DefferedRendering(scene);
	}
	else if (config.RenderMode == ERenderMode::RenderMode_TestRendering) {
		drawScene_ShadowMapTest(scene);
	}
}


std::shared_ptr<Camera> Viewer::createCamera(CameraType type)
{
	if (type == CameraType::PERSPECTIVE) {
		return std::make_shared<PerspectiveCamera>();
	}
	else if (type == CameraType::ORTHOGRAPHIC) {
		return std::make_shared<OrthographicCamera>();
	}
	return nullptr;
}


void Viewer::drawScene(std::shared_ptr<Scene> scene)
{
	/*
	* PlainPass
	*/
	ClearStates clearStatesPlainPass;
	clearStatesPlainPass.clearColor = clearColor;
	clearStatesPlainPass.colorFlag = true;
	clearStatesPlainPass.depthFlag = true;

	auto&& renderStates = renderer_->getRenderStates();
	renderStates.blend = true;
	renderStates.depthMask = true;
	renderStates.depthTest = true;
	renderer_->updateRenderStates(renderStates);

	renderer_->beginRenderPass(plainPass_->getFramebufferMain(), clearStatesPlainPass);
	renderer_->executeRenderPass(plainPass_, *scene);
	renderer_->endRenderPass();

	/*
	* ToScreenPass
	*/
	ClearStates clearStatesToScreenPass;
	clearStatesToScreenPass.clearColor = clearColor;
	clearStatesToScreenPass.colorFlag = true;
	clearStatesToScreenPass.depthFlag = true;

	// auto depthText = plainPass_->tempDepthBuffer_->getUniformSampler(*renderer_);
	auto outTex = plainPass_->getTexColorSampler();
	renderer_->beginRenderPass(nullptr, clearStatesToScreenPass);
	renderer_->renderToScreen(*outTex, width_, height_, true);
	renderer_->endRenderPass();
}

void Viewer::drawScene_DefferedRendering(std::shared_ptr<Scene> scene) {
	/*
	 * GeometryPass
	 */
	if (geometryPass_) {
		ClearStates clearStateGeometryPass;
		clearStateGeometryPass.clearColor = clearColor;
		clearStateGeometryPass.colorFlag = true;
		clearStateGeometryPass.depthFlag = true;

		auto&& renderStates = renderer_->getRenderStates();
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
		ClearStates clearStatesLightPass;
		clearStatesLightPass.clearColor = clearColor;
		clearStatesLightPass.colorFlag = true;
		clearStatesLightPass.depthFlag = false;

		BlendParameters blendParams;
		blendParams.SetBlendFactor(BlendFactor_ONE, BlendFactor_ONE);
		blendParams.SetBlendFunc(BlendFunc_ADD);

		auto&& renderStates = renderer_->getRenderStates();
		renderStates.blend = true;
		renderStates.blendParams = blendParams;
		renderStates.depthMask = true;
		renderStates.depthTest = true;
		renderer_->updateRenderStates(renderStates);

		lightPass_->setGBuffer(geometryPass_->getFramebufferMain());
		renderer_->beginRenderPass(lightPass_->getFramebufferMain(), clearStatesLightPass);
		renderer_->executeRenderPass(lightPass_, *scene);
		renderer_->endRenderPass();

		/*
		* ToScreenPass
		*/
		ClearStates clearStatesToScreenPass;
		clearStatesToScreenPass.clearColor = clearColor;
		clearStatesToScreenPass.colorFlag = true;
		clearStatesToScreenPass.depthFlag = true;

		auto outTex = lightPass_->getTexColorSampler();
		renderer_->beginRenderPass(nullptr, clearStatesToScreenPass);
		renderer_->renderToScreen(*outTex, width_, height_, true);
		renderer_->endRenderPass();
	}
}

void Viewer::drawScene_ShadowMapTest(std::shared_ptr<Scene> scene) {
	/*
	* ShadowPass
	*/
	ClearStates clearStatesPlainPass;
	clearStatesPlainPass.clearColor = clearColor;
	clearStatesPlainPass.colorFlag = true;
	clearStatesPlainPass.depthFlag = true;

	auto&& renderStates = renderer_->getRenderStates();
	renderStates.blend = false;
	renderStates.depthMask = true;
	renderStates.depthTest = true;
	renderer_->updateRenderStates(renderStates);

	renderer_->beginRenderPass(shadowPass_->getFramebufferMain(), clearStatesPlainPass);
	renderer_->executeRenderPass(shadowPass_, *scene);
	renderer_->endRenderPass();


	if (!scene->getLights().empty()) {
		/*
		* ToScreenPass
		*/
		ClearStates clearStatesToScreenPass;
		clearStatesToScreenPass.clearColor = clearColor;
		clearStatesToScreenPass.colorFlag = true;
		clearStatesToScreenPass.depthFlag = true;

		auto shadowMap = scene->getLights()[0]->getShadowMap(*shadowPass_);
		auto outTex = shadowMap->getUniformSampler(*renderer_);
		renderer_->beginRenderPass(nullptr, clearStatesToScreenPass);
		renderer_->renderToScreen(*outTex, width_, height_, true);
		renderer_->endRenderPass();
	}


}

void Viewer::drawModel(std::shared_ptr<UModel> model)
{

}

