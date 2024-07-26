#pragma once
#include "RenderPass/RenderPass.h"
#include "Renderer.h"
#include <memory>
class Renderer;
class UniformSampler;

class RenderPassShadow: public RenderPass {
public:
	RenderPassShadow(Renderer& renderer);

	virtual void render(FScene & scene) override;
	virtual void setupBuffers() override;
	virtual void init() override;
	virtual std::shared_ptr<FrameBuffer> getFramebufferMain() override;
private:
	std::shared_ptr<FrameBuffer> fboShadow_;
	std::shared_ptr<Texture> texColorMain_ = nullptr;
	std::shared_ptr<Texture> texDepthMain_ = nullptr;
};