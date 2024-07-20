//
// Created by Inory on 2024/7/18.
//

#ifndef RENDERPASSLIGHT_H
#define RENDERPASSLIGHT_H
#include <memory>
#include "RenderPass/RenderPass.h"


class Texture;
class FrameBuffer;
class UniformSampler;
class UObject;

class RenderPassLight: public RenderPass{
public:
    RenderPassLight(Renderer& renderer);
    virtual void render(Scene & scene) override;
    virtual void setupBuffers() override;
    virtual void init() override;
    std::shared_ptr<UniformSampler> getTexColorSampler();
    std::shared_ptr<FrameBuffer> getFramebufferMain();
    void setGBuffer(std::shared_ptr<FrameBuffer> gBuffer) { gBuffer_ = gBuffer; }
private:
    std::shared_ptr<FrameBuffer> fboMain_ = nullptr;
    std::shared_ptr<FrameBuffer> gBuffer_ = nullptr;
    std::shared_ptr<Texture> texColorMain_ = nullptr;
    std::shared_ptr<Texture> texDepthMain_ = nullptr;
};



#endif //RENDERPASSLIGHT_H
