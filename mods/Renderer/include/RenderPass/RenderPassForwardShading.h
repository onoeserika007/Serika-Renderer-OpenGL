#ifndef SERIKA_RENDERER_INCLUDE_RENDERPASS_RENDERPASSFORWARDSHADING_H
#define SERIKA_RENDERER_INCLUDE_RENDERPASS_RENDERPASSFORWARDSHADING_H
#include "RenderPass/RenderPass.h"
#include "Renderer.h"
#include <memory>
class Renderer;
class UniformSampler;

class RenderPassForwardShading : public RenderPass {
public:
	explicit RenderPassForwardShading(const std::shared_ptr<Renderer>& renderer);
	~RenderPassForwardShading();

	//virtual void resetBufferSize() override;
	virtual void render(FScene & scene) override;
	virtual void setupBuffers() override;
	virtual void init() override;

	std::shared_ptr<Texture> getOutTex();
	std::shared_ptr<FrameBuffer> getFramebufferMain() override;

private:
	std::shared_ptr<FrameBuffer> fboMain_ = nullptr;
	std::shared_ptr<Texture> texColorMain_ = nullptr;
	std::shared_ptr<Texture> texBlendResult_ = nullptr;
	std::shared_ptr<Texture> texDepthMain_ = nullptr;
};

#endif // SERIKA_RENDERER_INCLUDE_RENDERPASS_RENDERPASSFORWARDSHADING_H
