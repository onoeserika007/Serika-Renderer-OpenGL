#include "RenderPass/RenderPass.h"
#include "Renderer.h"
#include <memory>
class Renderer;
class UniformSampler;

class RenderPassShadow: public RenderPass {
private:
	std::shared_ptr<FrameBuffer> fboShadow_;
	std::shared_ptr<Texture> texDepthShadow_ = nullptr;
	std::shared_ptr<UniformSampler> depthShadowSampler_ = nullptr;
public:
	RenderPassShadow(Renderer& renderer): RenderPass(renderer) {

		// init and resize depth Buffer
		resetBufferSize();

		// create fbo
		if (!fboShadow_) {
			fboShadow_ = renderer_.createFrameBuffer(true);
		}
	}

	virtual void resetBufferSize() override;

	std::shared_ptr<UniformSampler> getTargetSampler();
};