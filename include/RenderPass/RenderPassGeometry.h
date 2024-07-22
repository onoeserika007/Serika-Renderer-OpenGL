#pragma once
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
        GBUFFER_TEXTURE_TYPE_SPECULAR,
        GBUFFER_NUM_TEXTURES
    };

    static const char* GBUFFER_NAMES[GBUFFER_NUM_TEXTURES];

    virtual void render(Scene & scene) override;
    virtual void setupBuffers() override;
    virtual void init() override;

    std::shared_ptr<FrameBuffer> getFramebufferMain() override;

    const std::vector<std::shared_ptr<Texture>>& getGBuffers() const;

private:
    std::shared_ptr<FrameBuffer> fboGbuffer_;
    std::vector<std::shared_ptr<Texture>> gbufferTextures_;
    std::shared_ptr<Texture> depthTexture_;
	std::shared_ptr<UniformSampler> depthShadowSampler_ = nullptr;
};