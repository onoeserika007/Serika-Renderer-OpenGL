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

	for(const auto& light: scene.getLights()) {
		// when change depth attachment, texture contents lost. Reason not clear. So I updated to opengl 4.3.
		auto&& shadowMapBuffer = light->getShadowMap(*this);
		renderer_.setCamera(light->getLightCamera());

		for (const auto& model: scene.getModels()) {
			renderer_.draw(*model, shaderPass_, nullptr);
		}
		texDepthMain_->copyDataTo(*shadowMapBuffer);
	}
	renderer_.restoreViewPort();
	renderer_.setBackToViewCamera();
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


