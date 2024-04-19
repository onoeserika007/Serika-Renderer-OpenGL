#include "RenderPass/RenderPass.h"
#include "Renderer.h"
#include <memory>
class Renderer;
class UniformSampler;

class RenderPassToScreen : public RenderPass {
public:
	RenderPassToScreen(Renderer& renderer);

	//virtual void resetBufferSize() override;
private:
	std::shared_ptr<FrameBuffer> fboShadow_;
	std::shared_ptr<Texture> texDepthShadow_ = nullptr;
	std::shared_ptr<UniformSampler> depthShadowSampler_ = nullptr;
};