#include "RenderPass/RenderPassGeometry.h"

#include <../../include/Geometry/Model.h>
#include <Scene.h>

#include "Renderer.h"
#include "FrameBuffer.h"

const char* RenderPassGeometry::GBUFFER_NAMES[] = {
	"gPosition",
	"gDiffuse",
	"gNormal",
	"gSpecular"
};

RenderPassGeometry::RenderPassGeometry(Renderer& renderer): RenderPass(renderer)
{
	shaderPass_ = ShaderPass::Shader_Geometry_Pass;
}

void RenderPassGeometry::init()
{
	gbufferTextures_.resize(GBUFFER_NUM_TEXTURES);
	// init and resize depth Buffer
	setupBuffers();

	// create fbo
	if (!fboGbuffer_) {
		fboGbuffer_ = renderer_.createFrameBuffer(true);
	}

	fboGbuffer_->setDepthAttachment(depthTexture_);
	for (int i = 0; i < GBUFFER_NUM_TEXTURES; i++) {
		auto& colorTex = gbufferTextures_[i];
		fboGbuffer_->setColorAttachment(colorTex, 0, i);
	}
}

std::shared_ptr<FrameBuffer> RenderPassGeometry::getFramebufferMain() {
	return fboGbuffer_;
}

const std::vector<std::shared_ptr<Texture>> & RenderPassGeometry::getGBuffers() const {
	return gbufferTextures_;
}

void RenderPassGeometry::setupBuffers()
{
	for (auto& colorAttachment : gbufferTextures_) {
		renderer_.setupGBuffer(colorAttachment, false, false);
	}
	renderer_.setupDepthBuffer(depthTexture_, false, true);
}

void RenderPassGeometry::render(Scene & scene) {
	setupBuffers();
	for (auto& model : scene.getModels()) {
		renderer_.draw(*model, shaderPass_, nullptr);
	}
}

