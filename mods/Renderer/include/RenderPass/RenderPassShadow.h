#ifndef SERIKA_RENDERER_INCLUDE_RENDERPASS_RENDERPASSSHADOW_H
#define SERIKA_RENDERER_INCLUDE_RENDERPASS_RENDERPASSSHADOW_H
#include "RenderPass/RenderPass.h"
#include "Renderer.h"
#include <memory>
class Renderer;
class UniformSampler;

class RenderPassShadow: public RenderPass {
public:
	explicit RenderPassShadow(const std::shared_ptr<Renderer>& renderer);
	virtual ~RenderPassShadow();

	virtual void render(FScene & scene) override;
	virtual void setupBuffers() override;
	virtual void init() override;
	virtual std::shared_ptr<FrameBuffer> getFramebufferMain() override;
private:
	std::shared_ptr<FrameBuffer> fboShadow_ = nullptr;
	std::shared_ptr<Texture> texColorMain_ = nullptr;
	std::shared_ptr<Texture> texDepthMain_ = nullptr;
};

#endif // SERIKA_RENDERER_INCLUDE_RENDERPASS_RENDERPASSSHADOW_H
