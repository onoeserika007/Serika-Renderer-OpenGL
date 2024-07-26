#include "RenderPass/RenderPassLight.h"

#include <glad/glad.h>

#include "Light.h"
#include "../../include/Material/Texture.h"
#include "Renderer.h"
#include "FScene.h"
#include "FrameBuffer.h"
#include "RenderPass/RenderPassGeometry.h"

RenderPassLight::RenderPassLight(Renderer &renderer): RenderPass(renderer) {
    shaderPass_ = ShaderPass::Shader_Light_Pass;
}

void RenderPassLight::render(FScene & scene) {
    if (auto&& geometryPass = geometryPass_.lock()) {

        RenderStates& renderStates = renderer_.renderStates;

        BlendParameters blendParams;
        blendParams.SetBlendFactor(BlendFactor_ONE, BlendFactor_ONE);
        blendParams.SetBlendFunc(BlendFunc_ADD);
        renderStates.blendParams = blendParams;

        auto enableBlending = [&renderStates, this](const bool bEnabled) {
            renderStates.blend = bEnabled;
            renderStates.depthTest = !bEnabled;
            renderStates.depthMask = !bEnabled;
            renderStates.cullFace = !bEnabled;
            renderer_.updateRenderStates(renderStates);
        };

        // deffred shading
        for (auto&& light: scene.getLights()) {
            renderer_.updateLightUniformBlock(light);
            renderer_.updateModelUniformBlock({}, renderer_.getViewCamera(), light);
            enableBlending(true);
            renderer_.dump(renderer_.getDefferedShadingProgram(geometryPass->getGBuffers()), true, fboMain_, 0);
        }

        // draw Lights
        // first copy depth buffer from geometryPass_
        auto&& depthBuffer = geometryPass->getFramebufferMain()->getDepthAttachment().tex;
        depthBuffer->copyDataTo(*texDepthMain_);
        enableBlending(false); // disable blending
        for (auto&& light: scene.getLights()) {
            renderer_.updateLightUniformBlock(nullptr);
            renderer_.draw(light, ShaderPass::Shader_ForwardShading_Pass, nullptr);
        }
        if (scene.skybox_) {
            renderer_.draw(scene.skybox_, ShaderPass::Shader_ForwardShading_Pass, nullptr);
        }
    }
}

void RenderPassLight::setupBuffers() {
    renderer_.setupColorBuffer(texColorMain_, renderer_.width(), renderer_.height(), false);
    renderer_.setupDepthBuffer(texDepthMain_, false, false);
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

std::shared_ptr<Texture> RenderPassLight::getOutTex() {
    return texColorMain_;
}

std::shared_ptr<FrameBuffer> RenderPassLight::getFramebufferMain() {
    return fboMain_;
}

void RenderPassLight::renderGBuffersToScreen(const std::shared_ptr<FrameBuffer> &gBuffer) {
    setupBuffers();
    assert(gBuffer, "Please check GeometryBuffer not null in Light RenderPass!");
    // fboMain_->bindForWriting();
    // glBindFramebuffer(GL_FRAMEBUFFER, 0);
    gBuffer->bindForReading();
    const int windowWidth = renderer_.width();
    const int windowHeight = renderer_.height();
    const GLsizei HalfWidth = (GLsizei)(windowWidth / 2.0f);
    const GLsizei HalfHeight = (GLsizei)(windowHeight / 2.0f);

    // copy textureBuffers to lightPass
    gBuffer->setReadBuffer(RenderPassGeometry::GBUFFER_TEXTURE_TYPE_POSITION);
    glBlitFramebuffer(0, 0, windowWidth, windowHeight,
                    0, 0, HalfWidth, HalfHeight, GL_COLOR_BUFFER_BIT, GL_LINEAR);
    gBuffer->setReadBuffer(RenderPassGeometry::GBUFFER_TEXTURE_TYPE_DIFFUSE);
    glBlitFramebuffer(0, 0, windowWidth, windowHeight,
                    0, HalfHeight, HalfWidth, windowHeight, GL_COLOR_BUFFER_BIT, GL_LINEAR);
    gBuffer->setReadBuffer(RenderPassGeometry::GBUFFER_TEXTURE_TYPE_NORMAL);
    glBlitFramebuffer(0, 0, windowWidth, windowHeight,
                    HalfWidth, HalfHeight, windowWidth, windowHeight, GL_COLOR_BUFFER_BIT, GL_LINEAR);
    gBuffer->setReadBuffer(RenderPassGeometry::GBUFFER_TEXTURE_TYPE_SPECULAR);
    glBlitFramebuffer(0, 0, windowWidth, windowHeight,
                    HalfWidth, 0, windowWidth, HalfHeight, GL_COLOR_BUFFER_BIT, GL_LINEAR);
}
