#include "RenderPass/RenderPassShadow.h"
#include "Renderer.h"
#include "FrameBufferOpenGL.h"

RenderPassShadow::RenderPassShadow(Renderer& renderer) : RenderPass(renderer) {

	// init and resize depth Buffer
	//resetBufferSize();

	// create fbo
	if (!fboShadow_) {
		fboShadow_ = renderer_.createFrameBuffer(true);
	}

	fboShadow_->setDepthAttachment(texDepthShadow_);
}

//void RenderPassShadow::resetBufferSize()
//{
//	if (renderer_.width() != width_ || renderer_.height() != height_) {
//		width_ = renderer_.width();
//		height_ = renderer_.height();
//		setupDepthBuffer(texDepthShadow_, false, true);
//	}
//}
