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
	// renderer_.setRenderViewPort(0, 0, config.Resolution_ShadowMap, config.Resolution_ShadowMap);
	setupBuffers();

	for(const auto& light: scene.getLights()) {
		auto&& shadowMapBuffer = light->getShadowMap(*this);
		fboShadow_->setDepthAttachment(shadowMapBuffer);
		fboShadow_->isValid();
		fboShadow_->bind();
		for (const auto& model: scene.getModels()) {
			renderer_.draw(*model, shaderPass_, nullptr);
		}
	}
	fboShadow_->setDepthAttachment(texDepthMain_);
	// renderer_.restoreViewPort();
}

void RenderPassShadow::setupBuffers() {
	setupColorBuffer(texColorMain_, false, false);
	setupDepthBuffer(texDepthMain_, false, false);
}

void RenderPassShadow::init() {
	// init and resize depth Buffer
	setupBuffers();

	// create fbo
	if (!fboShadow_) {
		fboShadow_ = renderer_.createFrameBuffer(true);
	}

	fboShadow_->setColorAttachment(texColorMain_, 0, 0);
	fboShadow_->setDepthAttachment(texDepthMain_);
}

std::shared_ptr<FrameBuffer> RenderPassShadow::getFramebufferMain() {
	return fboShadow_;
}


