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
	auto&& config = Config::getInstance();
	// renderer_.setRenderViewPort(0, 0, config.Resolution_ShadowMap, config.Resolution_ShadowMap);

	// 更新新Buffer后还要记得重新绑定到fbo
	// 为何这里的texture没有被回收？？ 懂了！！Frambuffer中的Attachment还保存有一份引用
	setupBuffers();
	fboMain_->bind();
	RenderStates& renderStates = renderer_.renderStates;

	BlendParameters blendParams;
	blendParams.SetBlendFactor(BlendFactor_ONE, BlendFactor_ONE);
	blendParams.SetBlendFunc(BlendFunc_ADD);
	// blendParams.SetBlendFactor(BlendFactor_CONSTANT_COLOR, BlendFactor_ONE_MINUS_CONSTANT_COLOR);
	// blendParams.SetBlendFunc(BlendFunc_ADD);
	// blendParams.blendColor = glm::vec4(0.5f, 0.5f, 0.5f, 1.f);
	renderStates.blendParams = blendParams;

	auto setupComputeLight = [&renderStates, this]() {
		renderStates.blend = false;
		renderStates.depthTest = true;
		renderStates.depthMask = true;
		renderStates.cullFace = true;
		renderer_.updateRenderStates(renderStates);
	};

	auto setupToScreen = [&renderStates, this]() {
		renderStates.blend = true;
		renderStates.depthTest = false;
		renderStates.depthMask = false;
		renderStates.cullFace = false;
		renderer_.updateRenderStates(renderStates);
	};

	// setAttachment后会绑定到0 fbo，所以记得绑定回来
	for (const auto& light: scene.getLights()) {
		renderer_.updateLightUniformBlock(light);

		// 关闭混合
		setupComputeLight();
		fboMain_->setWriteBuffer(0, true); // to render target
		fboMain_->clearDepthBuffer();
		// 渲染所有mesh时关闭混合，保证深度测试
		for (const auto& model : scene.getModels()) {
			if (config.bShadowMap) {
				renderer_.draw(*model, shaderPass_, light);
			}
			else {
				renderer_.draw(*model, shaderPass_, nullptr);
			}
		}

		// 混合光照结果时打开混合
		setupToScreen();
		renderer_.dump(texColorMain_, true, true, fboMain_, 1, false, {}); // to blend target
	}

	// 画光源时关闭混合
	setupComputeLight();
	fboMain_->setWriteBuffer(1, false); // to blend target
	for (const auto& light: scene.getLights()) {
		renderer_.updateLightUniformBlock(nullptr);
		renderer_.draw(*light, shaderPass_, nullptr);
	}

}

void RenderPassForwardShading::setupBuffers()
{

	renderer_.setupColorBuffer(texColorMain_, false, false);
	renderer_.setupColorBuffer(texBlendResult_, false, false);
	renderer_.setupDepthBuffer(texDepthMain_, false, false);

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

std::shared_ptr<Texture> RenderPassForwardShading::getOutTex()
{
	return texBlendResult_;
}

std::shared_ptr<FrameBuffer> RenderPassForwardShading::getFramebufferMain()
{
	return fboMain_;
}
