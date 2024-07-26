//
// Created by Inory on 2024/7/18.
//

#ifndef RENDERPASSLIGHT_H
#define RENDERPASSLIGHT_H
#include <memory>
#include "RenderPass/RenderPass.h"


class RenderPassGeometry;
class Texture;
class FrameBuffer;
class UniformSampler;
class UObject;

class RenderPassLight: public RenderPass{
public:
    RenderPassLight(Renderer& renderer);
    virtual void render(FScene & scene) override;
    virtual void setupBuffers() override;
    virtual void init() override;

    std::shared_ptr<Texture> getOutTex();
    std::shared_ptr<FrameBuffer> getFramebufferMain();
    void renderGBuffersToScreen(const std::shared_ptr<FrameBuffer>& gBuffer);
    void injectGeometryPass(const std::shared_ptr<RenderPassGeometry>& geometryPass) { geometryPass_ = geometryPass; }
private:
    std::shared_ptr<FrameBuffer> fboMain_ = nullptr;
    std::shared_ptr<Texture> texColorMain_ = nullptr;
    std::shared_ptr<Texture> texDepthMain_ = nullptr;
    std::weak_ptr<RenderPassGeometry> geometryPass_ {};
};



#endif //RENDERPASSLIGHT_H
