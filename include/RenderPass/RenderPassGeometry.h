#pragma once
#include "RenderPass/RenderPass.h"
#include <vector>

class UniformSampler;


class RenderPassGeometry : public RenderPass{
public:
    RenderPassGeometry(const std::shared_ptr<Renderer>& renderer);

    enum GBUFFER_TEXTURE_TYPE {
        GBUFFER_TEXTURE_TYPE_POSITION_DEPTH,
        GBUFFER_TEXTURE_TYPE_DIFFUSE,
        GBUFFER_TEXTURE_TYPE_NORMAL_SPECULAR,
        GBUFFER_TEXTURE_TYPE_AO_METAL_ROUGHNESS,
        GBUFFER_NUM_TEXTURES
    };

    // This Name Must be aligned with which in shader!!!
    static const char* GBUFFER_NAMES[GBUFFER_NUM_TEXTURES];

    virtual void render(FScene & scene) override;
    virtual void setupBuffers() override;
    virtual void init() override;

    std::shared_ptr<FrameBuffer> getFramebufferMain() override;

    const std::vector<std::shared_ptr<Texture>>& getGBuffers() const;
    std::shared_ptr<Texture> getSSAOResult() const { return ssaoResult_; }

private:
    std::shared_ptr<FrameBuffer> fboGbuffer_;
    std::shared_ptr<FrameBuffer> ssaoFbo_;
    std::vector<std::shared_ptr<Texture>> gbufferTextures_;
    std::shared_ptr<Texture> ssaoMid_;
    std::shared_ptr<Texture> ssaoResult_;
    std::shared_ptr<Texture> depthTexture_;
};
