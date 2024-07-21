#include "RenderPass/RenderPassForwardShading.h"

#include <../../include/Geometry/Model.h>
#include <Scene.h>
#include "Light.h"

#include "FrameBuffer.h"
#include "Renderer.h"
#include "Uniform.h"
#include "Base/Config.h"

RenderPassForwardShading::RenderPassForwardShading(Renderer& renderer): RenderPass(renderer)
{
}

void RenderPassForwardShading::render(Scene & scene)
{
	// auto&& config = Config::getInstance();
	// renderer_.setRenderViewPort(0, 0, config.Resolution_ShadowMap, config.Resolution_ShadowMap);

	// 更新新Buffer后还要记得重新绑定到fbo
	// 为何这里的texture没有被回收？？ 懂了！！Frambuffer中的Attachment还保存有一份引用
	setupBuffers();
	fboMain_->bind();
	RenderStates& renderStates = renderer_.renderStates;

	BlendParameters blendParams;
	blendParams.SetBlendFactor(BlendFactor_ONE, BlendFactor_ONE);
	blendParams.SetBlendFunc(BlendFunc_ADD);
	renderStates.blendParams = blendParams;
	auto setupToScreen = [&renderStates, this]() {
		renderStates.blend = true;
		renderStates.depthTest = false;
		renderStates.depthMask = false;
		renderStates.cullFace = false;
		renderer_.updateRenderStates(renderStates);
	};

	auto setupComputeLight = [&renderStates, this]() {
		renderStates.blend = false;
		renderStates.depthTest = true;
		renderStates.depthMask = true;
		renderStates.cullFace = true;
		renderer_.updateRenderStates(renderStates);
	};

	// setAttachment后会绑定到0 fbo，所以记得绑定回来

	// 目前仅支持单光源

	for (const auto& light: scene.getLights()) {
		renderer_.updateLightUniformBlock(light);

		// 关闭混合
		setupComputeLight();
		// 渲染所有mesh时关闭混合，保证深度测试
		for (const auto& model : scene.getModels()) {
			renderer_.draw(*model, shaderPass_, nullptr);
		}

		// 混合光照结果时打开混合
		setupToScreen();
		auto outTex = texColorMain_->getUniformSampler(renderer_);
		renderer_.dump(*outTex, true, true, fboMain_, 1);
	}

	// 画光源时关闭混合
	setupComputeLight();
	for (const auto& light: scene.getLights()) {
		renderer_.updateLightUniformBlock(nullptr);
		renderer_.draw(*light, shaderPass_, nullptr);
	}

	// 混合光照结果时打开混合
	setupToScreen();
	auto outTex = texColorMain_->getUniformSampler(renderer_);
	renderer_.dump(*outTex, true, true, fboMain_, 1);

}

void RenderPassForwardShading::setupBuffers()
{

	setupColorBuffer(texColorMain_, false, false);
	setupColorBuffer(texBlendResult_, false, false);
	setupDepthBuffer(texDepthMain_, false, false);

	// create fbo
	if (!fboMain_) {
		fboMain_ = renderer_.createFrameBuffer(true);
	}

	fboMain_->setColorAttachment(texColorMain_, 0, 0);
	fboMain_->setColorAttachment(texBlendResult_, 0, 1);
	fboMain_->setDepthAttachment(texDepthMain_);
	fboMain_->isValid();
}

void RenderPassForwardShading::init()
{
	// init and resize depth Buffer
	setupBuffers();
}

std::shared_ptr<UniformSampler> RenderPassForwardShading::getTexColorSampler()
{
	return texBlendResult_->getUniformSampler(renderer_);
}

std::shared_ptr<FrameBuffer> RenderPassForwardShading::getFramebufferMain()
{
	return fboMain_;
}
