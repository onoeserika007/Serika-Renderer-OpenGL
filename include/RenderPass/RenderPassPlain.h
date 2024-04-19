#include "RenderPass/RenderPass.h"
#include "Renderer.h"
#include <memory>
class Renderer;
class UniformSampler;

class RenderPassPlain : public RenderPass {
public:
	RenderPassPlain(Renderer& renderer);

	//virtual void resetBufferSize() override;
	virtual void render(Object& obj) = 0;
	virtual void setupBuffers() = 0;
	virtual void init() = 0;
private:
	std::shared_ptr<FrameBuffer> fboMain_ = nullptr;
	std::shared_ptr<Texture> texColorMain_ = nullptr;
	std::shared_ptr<Texture> texDepthMain_ = nullptr;
	std::shared_ptr<UniformSampler> texColorSampler = nullptr;
};