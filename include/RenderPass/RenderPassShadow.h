#include "RenderPass/RenderPass.h"
class Renderer;

class RenderPassShadow: public RenderPass {
private:
	std::shared_ptr<Texture> texDepthShadow_ = nullptr;
public:
	RenderPassShadow(Renderer& renderer): RenderPass(renderer) {
	}
};