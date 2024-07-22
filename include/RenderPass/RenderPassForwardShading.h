#pragma once
#include "RenderPass/RenderPass.h"
#include "Renderer.h"
#include <memory>
class Renderer;
class UniformSampler;

class RenderPassForwardShading : public RenderPass {
public:
	RenderPassForwardShading(Renderer& renderer);

	//virtual void resetBufferSize() override;
	virtual void render(Scene & scene) override;
	virtual void setupBuffers() override;
	virtual void init() override;

	std::shared_ptr<Texture> getOutTex();
	std::shared_ptr<FrameBuffer> getFramebufferMain() override;

	std::shared_ptr<Texture> tempDepthBuffer_ = nullptr;
private:
	std::shared_ptr<FrameBuffer> fboMain_ = nullptr;
	std::shared_ptr<Texture> texColorMain_ = nullptr;
	std::shared_ptr<Texture> texBlendResult_ = nullptr;
	std::shared_ptr<Texture> texDepthMain_ = nullptr;
};