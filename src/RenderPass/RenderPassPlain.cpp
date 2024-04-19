#include "RenderPass/RenderPassPlain.h"
#include "FrameBuffer.h"
#include "Renderer.h"

RenderPassPlain::RenderPassPlain(Renderer& renderer): RenderPass(renderer)
{
}

void RenderPassPlain::render(Object& obj)
{
	setupBuffers();
	fboMain_->bind();
	renderer_.drawObject(obj, shaderPass_);
}

void RenderPassPlain::setupBuffers()
{
	setupColorBuffer(texColorMain_, false, true);
	setupDepthBuffer(texDepthMain_, false, true);
}

void RenderPassPlain::init()
{
	// init and resize depth Buffer
	setupBuffers();

	// create fbo
	if (!fboMain_) {
		fboMain_ = renderer_.createFrameBuffer(true);
	}

	fboMain_->setDepthAttachment(texDepthMain_);
	fboMain_->setColorAttachment(texColorMain_, 0);
}
