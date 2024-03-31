#include "RenderPass/RenderPassShadow.h"
#include "Renderer.h"

void RenderPassShadow::resetBufferSize()
{
	if (renderer_.width() != width_ || renderer_.height() != height_) {
		setupColorBuffer(texDepthShadow_, false, true);
	}
}
