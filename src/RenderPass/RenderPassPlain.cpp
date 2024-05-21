#include "RenderPass/RenderPassPlain.h"
#include "FrameBuffer.h"
#include "Renderer.h"
#include "Uniform.h"
#include "Base/RenderStates.h"

RenderPassPlain::RenderPassPlain(Renderer& renderer): RenderPass(renderer)
{
}

void RenderPassPlain::render(Object& obj)
{
	setupBuffers();
	renderer_.drawObject(obj, shaderPass_);
}

void RenderPassPlain::setupBuffers()
{
	setupColorBuffer(texColorMain_, false, false);
	setupDepthBuffer(texDepthMain_, false, false);
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
	fboMain_->isValid();
}

std::shared_ptr<UniformSampler> RenderPassPlain::getTexColorSampler()
{
	if (!texColorSampler) {
		auto texInfo = texColorMain_->getTextureInfo();
		texColorSampler = renderer_.createUniformSampler(texInfo);
	}

	// update colorbuffer in case it rebuilt.
	texColorSampler->setTexture(texColorMain_);
	return texColorSampler;
}

std::shared_ptr<FrameBuffer> RenderPassPlain::getFramebufferMain()
{
	return fboMain_;
}
