#include "RenderPass/RenderPassLight.h"

#include <glad/glad.h>

#include "Uniform.h"
#include "Texture.h"
#include "Renderer.h"
#include "RenderPass/RenderPassGeometry.h"

RenderPassLight::RenderPassLight(Renderer &renderer): RenderPass(renderer) {
    shaderPass_ = ShaderPass::Shader_Light_Pass;
}

void RenderPassLight::render(Scene & scene) {
    setupBuffers();
    assert(gBuffer_, "Please check GeometryBuffer not null in Light RenderPass!");
    // fboMain_->bindForWriting();
    // glBindFramebuffer(GL_FRAMEBUFFER, 0);
    gBuffer_->bindForReading();
    const int windowWidth = renderer_.width();
    const int windowHeight = renderer_.height();
    const GLsizei HalfWidth = (GLsizei)(windowWidth / 2.0f);
    const GLsizei HalfHeight = (GLsizei)(windowHeight / 2.0f);

    // copy textureBuffers to lightPass
    gBuffer_->setReadBuffer(RenderPassGeometry::GBUFFER_TEXTURE_TYPE_POSITION);
    glBlitFramebuffer(0, 0, windowWidth, windowHeight,
                    0, 0, HalfWidth, HalfHeight, GL_COLOR_BUFFER_BIT, GL_LINEAR);
    gBuffer_->setReadBuffer(RenderPassGeometry::GBUFFER_TEXTURE_TYPE_DIFFUSE);
    glBlitFramebuffer(0, 0, windowWidth, windowHeight,
                    0, HalfHeight, HalfWidth, windowHeight, GL_COLOR_BUFFER_BIT, GL_LINEAR);
    gBuffer_->setReadBuffer(RenderPassGeometry::GBUFFER_TEXTURE_TYPE_NORMAL);
    glBlitFramebuffer(0, 0, windowWidth, windowHeight,
                    HalfWidth, HalfHeight, windowWidth, windowHeight, GL_COLOR_BUFFER_BIT, GL_LINEAR);
    gBuffer_->setReadBuffer(RenderPassGeometry::GBUFFER_TEXTURE_TYPE_TEXCOORD);
    glBlitFramebuffer(0, 0, windowWidth, windowHeight,
                    HalfWidth, 0, windowWidth, HalfHeight, GL_COLOR_BUFFER_BIT, GL_LINEAR);
}

void RenderPassLight::setupBuffers() {
    setupColorBuffer(texColorMain_, false, false);
    setupDepthBuffer(texDepthMain_, false, false);
}

void RenderPassLight::init() {
    // init and resize depth Buffer
    setupBuffers();

    // create fbo
    if (!fboMain_) {
        fboMain_ = renderer_.createFrameBuffer(true);
    }

    fboMain_->setDepthAttachment(texDepthMain_);
    fboMain_->setColorAttachment(texColorMain_, 0, 0);
}

std::shared_ptr<UniformSampler> RenderPassLight::getTexColorSampler() {
    if (texColorMain_) {
        return texColorMain_->getUniformSampler(renderer_);
    }
    return {};
}

std::shared_ptr<FrameBuffer> RenderPassLight::getFramebufferMain() {
    return fboMain_;
}
