#include "RenderPass/RenderPass.h"
#include <vector>

class UniformSampler;

class RenderPassGeometry : public RenderPass{
public:
    RenderPassGeometry(Renderer& renderer);

    enum GBUFFER_TEXTURE_TYPE {
        GBUFFER_TEXTURE_TYPE_POSITION,
        GBUFFER_TEXTURE_TYPE_DIFFUSE,
        GBUFFER_TEXTURE_TYPE_NORMAL,
        GBUFFER_TEXTURE_TYPE_TEXCOORD,
        GBUFFER_NUM_TEXTURES
    };

    virtual void init() override;
    virtual void setupBuffers() override;

private:
    std::shared_ptr<FrameBuffer> fboGbuffer_;;
    std::vector<std::shared_ptr<Texture>> gbufferTextures_;
    std::shared_ptr<Texture> depthTexture_;
	std::shared_ptr<UniformSampler> depthShadowSampler_ = nullptr;
};