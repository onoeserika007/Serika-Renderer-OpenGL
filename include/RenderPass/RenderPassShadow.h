#pragma once
#include "RenderPass/RenderPass.h"
#include "Renderer.h"
#include <memory>
class Renderer;
class UniformSampler;

class RenderPassShadow: public RenderPass {
public:
	RenderPassShadow(Renderer& renderer);

	//virtual void resetBufferSize() override;
private:
	std::shared_ptr<FrameBuffer> fboShadow_;
	std::shared_ptr<Texture> texDepthShadow_ = nullptr;
	std::shared_ptr<UniformSampler> depthShadowSampler_ = nullptr;
};