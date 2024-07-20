#include "RenderPass/RenderPassPlain.h"

#include <Model.h>
#include <Scene.h>
#include "ULight.h"

#include "FrameBuffer.h"
#include "Renderer.h"
#include "Uniform.h"
#include "Base/Config.h"

RenderPassPlain::RenderPassPlain(Renderer& renderer): RenderPass(renderer)
{
}

void RenderPassPlain::render(Scene & scene)
{
	// auto&& config = Config::getInstance();
	// renderer_.setRenderViewPort(0, 0, config.Resolution_ShadowMap, config.Resolution_ShadowMap);

	// 更新新Buffer后还要记得重新绑定到fbo
	// 为何这里的texture没有被回收？？ 懂了！！Frambuffer中的Attachment还保存有一份引用
	setupBuffers();
	fboMain_->bind();
	// setAttachment后会绑定到0 fbo，所以记得绑定回来

	for (const auto& model : scene.getModels()) {
		for (const auto& mesh : model->getMeshes()) {
			for (const auto& light: scene.getLights()) {
				// setupShadowMapBuffer(tempDepthBuffer_, config.Resolution_ShadowMap, config.Resolution_ShadowMap, false, false);
				// fboMain_->setDepthAttachment(tempDepthBuffer_);
				// fboMain_->bind();

				renderer_.updateLightUniformBlock(*mesh, light);
				renderer_.draw(*mesh, shaderPass_, nullptr);
			}
		}
	}
;
	for (const auto& light: scene.getLights()) {
		renderer_.updateLightUniformBlock(*light, nullptr);
		renderer_.draw(*light, shaderPass_, nullptr);
	}

	// fboMain_->setDepthAttachment(texDepthMain_);
	// renderer_.restoreViewPort();
}

void RenderPassPlain::setupBuffers()
{

	setupColorBuffer(texColorMain_, false, false);
	setupDepthBuffer(texDepthMain_, false, false);

	// create fbo
	if (!fboMain_) {
		fboMain_ = renderer_.createFrameBuffer(true);
	}

	fboMain_->setColorAttachment(texColorMain_, 0, 0);
	fboMain_->setDepthAttachment(texDepthMain_);
	fboMain_->isValid();
}

void RenderPassPlain::init()
{
	// init and resize depth Buffer
	setupBuffers();
}

std::shared_ptr<UniformSampler> RenderPassPlain::getTexColorSampler()
{
	if (!texColorSampler) {
		auto texInfo = texColorMain_->getTextureInfo();
		texColorSampler = renderer_.createUniformSampler(texInfo);
	}

	// update colorbuffer in case it rebuilt.
	texColorSampler->setTexture(*texColorMain_);
	return texColorSampler;
}

std::shared_ptr<FrameBuffer> RenderPassPlain::getFramebufferMain()
{
	return fboMain_;
}
