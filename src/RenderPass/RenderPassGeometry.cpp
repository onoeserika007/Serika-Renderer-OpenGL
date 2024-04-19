#include "RenderPass/RenderPassGeometry.h"
#include "Renderer.h"
#include "FrameBuffer.h"

RenderPassGeometry::RenderPassGeometry(Renderer& renderer): RenderPass(renderer)
{
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

void RenderPassGeometry::setupBuffers()
{
	for (auto& colorAttachment : gbufferTextures_) {
		setupColorBuffer(colorAttachment, false, true);
	}
	setupDepthBuffer(depthTexture_, false, true);
}

