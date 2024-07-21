#include "RenderPass/RenderPassShadow.h"
#include "Renderer.h"
#include "FrameBufferOpenGL.h"
#include "Light.h"
#include "../../include/Geometry/Model.h"
#include "Scene.h"
#include "Base/Config.h"

RenderPassShadow::RenderPassShadow(Renderer& renderer) : RenderPass(renderer) {
	shaderPass_ = ShaderPass::Shader_Shadow_Pass;
}

void RenderPassShadow::render(Scene &scene) {
	auto&& config = Config::getInstance();
	renderer_.setRenderViewPort(0, 0, config.Resolution_ShadowMap, config.Resolution_ShadowMap);
	setupBuffers();
	fboShadow_->bind();
	//
	int i = 0;
	int id = 0;
	for(const auto& light: scene.getLights()) {
		auto&& shadowMapBuffer = light->getShadowMap(*this); // when change depth attachment, texture contents lost. Reason not clear.
		// setupDepthBuffer(texTempDepth_, false);
		texTempDepth_ = shadowMapBuffer;
		// if (i == 0) {
		// 	id = shadowMapBuffer->getId();
		// }
		// if (i == 1) {
		// 	fboShadow_->setDepthAttachment(shadowMapBuffer);
		// }
		// else {
		// 	fboShadow_->setDepthAttachment(shadowMapBuffer);
		// }
		for (const auto& model: scene.getModels()) {
			renderer_.draw(*model, shaderPass_, nullptr);
		}
		texDepthMain_->copyDataTo(*shadowMapBuffer);
		i++;
	}
	// fboShadow_->setDepthAttachment(texDepthMain_);
	renderer_.restoreViewPort();

	// for (const auto& light: scene.getLights()) {
	// 	// setupShadowMapBuffer(tempDepthBuffer_, config.Resolution_ShadowMap, config.Resolution_ShadowMap, false, false);
	// 	// fboMain_->setDepthAttachment(tempDepthBuffer_);
	// 	// fboMain_->bind();
	// 	renderer_.updateLightUniformBlock(light);
	// 	for (const auto& model : scene.getModels()) {
	// 		renderer_.draw(*model, shaderPass_, nullptr);
	// 	}
	// }
}

void RenderPassShadow::setupBuffers() {
	setupColorBuffer(texColorMain_, false, false);
	setupDepthBuffer(texDepthMain_, false, false);

	// create fbo
	if (!fboShadow_) {
		fboShadow_ = renderer_.createFrameBuffer(true);
	}

	fboShadow_->setColorAttachment(texColorMain_, 0, 0);
	fboShadow_->setDepthAttachment(texDepthMain_);
}

void RenderPassShadow::init() {
	// init and resize depth Buffer
	setupBuffers();

}

std::shared_ptr<FrameBuffer> RenderPassShadow::getFramebufferMain() {
	return fboShadow_;
}


