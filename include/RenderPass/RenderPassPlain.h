#pragma once
#include "RenderPass/RenderPass.h"
#include "Renderer.h"
#include <memory>
class Renderer;
class UniformSampler;

class RenderPassPlain : public RenderPass {
public:
	RenderPassPlain(Renderer& renderer);

	//virtual void resetBufferSize() override;
	virtual void render(Object& obj);
	virtual void setupBuffers();
	virtual void init();
	std::shared_ptr<UniformSampler> getTexColorSampler();
	std::shared_ptr<FrameBuffer> getFramebufferMain();
private:
	std::shared_ptr<FrameBuffer> fboMain_ = nullptr;
	std::shared_ptr<Texture> texColorMain_ = nullptr;
	std::shared_ptr<Texture> texDepthMain_ = nullptr;
	std::shared_ptr<UniformSampler> texColorSampler = nullptr;
};