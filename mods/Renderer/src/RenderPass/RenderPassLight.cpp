#include "RenderPass/RenderPassLight.h"

#include <glad/glad.h>

#include "ULight.h"
#include "Material/Texture.h"
#include "Renderer.h"
#include "FScene.h"
#include "FrameBuffer.h"
#include "RenderPass/RenderPassGeometry.h"

RenderPassLight::RenderPassLight(const std::shared_ptr<Renderer>& renderer): RenderPass(renderer) {
    shaderPass_ = ShaderPass::Shader_Light_Pass;
}

void RenderPassLight::render(FScene & scene) {
    auto&& config = Config::getInstance();
    setupBuffers();
    fboMain_->bind();;;
    if (auto&& geometryPass = geometryPass_.lock()) {

        RenderStates& renderStates = renderer_->renderStates_;

        BlendParameters blendParams;
        blendParams.SetBlendFactor(BlendFactor_ONE, BlendFactor_ONE);
        blendParams.SetBlendFunc(BlendFunc_ADD);
        renderStates.blendParams = blendParams;

        auto enableBlending = [&renderStates, this](const bool bEnabled) {
            renderStates.blend = bEnabled;
            renderStates.depthTest = !bEnabled;
            renderStates.depthMask = !bEnabled;
            renderStates.cullFace = !bEnabled;
            renderer_->updateRenderStates(renderStates);
        };

        // deffred shading
        for (auto&& light: scene.getLights()) {
            renderer_->updateLightUniformBlock(light);
            renderer_->updateMainUniformBlock({}, renderer_->getViewCamera(), light);
            renderer_->updateShadowCubeUniformBlock(light);
            enableBlending(true);
            switch (config.ShadingModelForDeferredRendering) {
                case EShadingModel::Shading_BaseColor: {
                    renderer_->dump(
                    renderer_->getDefferedShadingProgram(geometryPass->getGBuffers(),
                    Shading_BaseColor), true, fboMain_, 0);
                    break;
                }
                case EShadingModel::Shading_BlinnPhong: {
                    renderer_->dump(
                    renderer_->getDefferedShadingProgram(geometryPass->getGBuffers(),
                    Shading_BlinnPhong), true, fboMain_, 0);
                    break;
                }
                case EShadingModel::Shading_PBR: {
                    renderer_->dump(
                        renderer_->getDefferedShadingProgram(geometryPass->getGBuffers(),
                        Shading_PBR), true, fboMain_, 0);
                    break;
                }
                default: {
                renderer_->dump(
                    renderer_->getDefferedShadingProgram(geometryPass->getGBuffers(),
                    Shading_BlinnPhong), true, fboMain_, 0);
                    break;
                }
            }
        }

        // draw Lights
        // first copy depth buffer from geometryPass_
        auto&& depthBuffer = geometryPass->getFramebufferMain()->getDepthAttachment().tex;
        depthBuffer->copyDataTo(*texDepthMain_);
        enableBlending(false); // disable blending
        for (auto&& light: scene.getPackedLightMeshes()) {
            renderer_->updateLightUniformBlock(nullptr);
            renderer_->drawMesh(light, ShaderPass::Shader_ForwardShading_Pass, nullptr);
        }
        if (scene.skybox_ && scene.skybox_->getMesh()) {
            renderer_->drawMesh(scene.skybox_->getMesh(), ShaderPass::Shader_ForwardShading_Pass, nullptr);
        }
    }
}

void RenderPassLight::setupBuffers() {
    // create fbo
    if (!fboMain_) {
        fboMain_ = renderer_->createFrameBuffer(true);
    }
    if (renderer_->setupColorBuffer(texColorMain_, renderer_->width(), renderer_->height(), false)) {
        fboMain_->setColorAttachment(texColorMain_, 0, 0);
    }

    renderer_->setupColorBuffer(texColorMain_, renderer_->width(), renderer_->height(), false);
    if (renderer_->setupDepthBuffer(texDepthMain_, false, false)) {
        fboMain_->setDepthAttachment(texDepthMain_);
    }
}

void RenderPassLight::init() {
    // init and resize depth Buffer
    setupBuffers();

}

std::shared_ptr<Texture> RenderPassLight::getOutTex() {
    return texColorMain_;
}

std::shared_ptr<FrameBuffer> RenderPassLight::getFramebufferMain() {
    return fboMain_;;
}

void RenderPassLight::renderGBuffersToScreen() {
    setupBuffers();
    fboMain_->bind();;
    if (auto&& geometryPass = geometryPass_.lock()) {
        auto&& gFbo_ = geometryPass->getFramebufferMain();
        assert(gFbo_);
        // fboMain_->bindForWriting();
        // glBindFramebuffer(GL_FRAMEBUFFER, 0);
        gFbo_->bindForReading();
        const int windowWidth = renderer_->width();
        const int windowHeight = renderer_->height();;
        const GLsizei HalfWidth = (GLsizei)(windowWidth / 2.0f);
        const GLsizei HalfHeight = (GLsizei)(windowHeight / 2.0f);

        // copy textureBuffers to lightPass
        gFbo_->setReadBuffer(RenderPassGeometry::GBUFFER_TEXTURE_TYPE_POSITION_DEPTH);
        glBlitFramebuffer(0, 0, windowWidth, windowHeight,
                        0, 0, HalfWidth, HalfHeight, GL_COLOR_BUFFER_BIT, GL_LINEAR);
        gFbo_->setReadBuffer(RenderPassGeometry::GBUFFER_TEXTURE_TYPE_DIFFUSE);
        glBlitFramebuffer(0, 0, windowWidth, windowHeight,
                        0, HalfHeight, HalfWidth, windowHeight, GL_COLOR_BUFFER_BIT, GL_LINEAR);
        gFbo_->setReadBuffer(RenderPassGeometry::GBUFFER_TEXTURE_TYPE_NORMAL_SPECULAR);
        glBlitFramebuffer(0, 0, windowWidth, windowHeight,
                        HalfWidth, HalfHeight, windowWidth, windowHeight, GL_COLOR_BUFFER_BIT, GL_LINEAR);
        gFbo_->setReadBuffer(RenderPassGeometry::GBUFFER_TEXTURE_TYPE_AO_METAL_ROUGHNESS);
        glBlitFramebuffer(0, 0, windowWidth, windowHeight,
                        HalfWidth, 0, windowWidth, HalfHeight, GL_COLOR_BUFFER_BIT, GL_LINEAR);
    }
}
