#include "RenderPass/RenderPassShadow.h"
#include "Renderer.h"
#include "OpenGL/FrameBufferOpenGL.h"
#include "Light.h"
#include "FScene.h"
#include "Base/Config.h"
#include "Geometry/Model.h"

RenderPassShadow::RenderPassShadow(Renderer& renderer) : RenderPass(renderer) {
	shaderPass_ = ShaderPass::Shader_Shadow_Pass;
}

void RenderPassShadow::render(FScene &scene) {
	auto&& config = Config::getInstance();
	renderer_.setRenderViewPort(0, 0, config.Resolution_ShadowMap, config.Resolution_ShadowMap);
	setupBuffers();

	auto setupForShadowDrawing = [this]() {
		fboShadow_->bind(); // bind back to fbo, since setupBuffers() set attachment will disvalidate fbo
		fboShadow_->disableForColorWriting();
		fboShadow_->disableForColorReading();

		RenderStates& renderStates = renderer_.renderStates;
		renderStates.blend = false;
		renderStates.depthTest = true;;
		renderStates.depthMask = true;
		renderStates.cullFace = true;
		renderer_.updateRenderStates(renderStates);

		fboShadow_->clearDepthBuffer(); // 一定要在写入深度缓存之前clear，否则所有的深度测试都无法通过
	};

	auto switchCullFace = [this](const bool bCullBack) {
		RenderStates& render_states = renderer_.renderStates;
		render_states.faceToCull = bCullBack? CullMode_BACK: CullMode_FRONT;
		renderer_.updateRenderStates(render_states);
	};

	// enable front cull
	// switchCullFace(false);
	for(const auto& light: scene.getLights()) {
		// when change depth attachment, texture contents lost. Reason not clear. So I updated to opengl 4.3.
		auto&& shadowMapBuffer = light->getShadowMap(renderer_);

		renderer_.updateLightUniformBlock(light); // 之前单个shadowmap是用camera位置更新做的，所以没更新光信息
		// directional light
		if (!light->isPointLight()) {
			renderer_.setCamera(light->getLightCamera());
			fboShadow_->setDepthAttachment(shadowMapBuffer);
			setupForShadowDrawing();
			for (const auto& model: scene.getModels()) {
				// renderer_.setCullFaceEnabled(model->useCull());
				renderer_.draw(model, ShaderPass::Shader_Shadow_Pass, nullptr);
			}
			// texDepthMain_->copyDataTo(*shadowMapBuffer);
			renderer_.setBackToViewCamera();
		}
		// point light
		else {
			fboShadow_->setDepthAttachment(shadowMapBuffer);
			setupForShadowDrawing();
			// update shadowMatrices before draw shadow cube
			renderer_.updateShadowCubeUniformBlock(light);
			for (const auto& model: scene.getModels()) {
				renderer_.draw(model, ShaderPass::Shader_Shadow_Cube_Pass, nullptr);
			}
			// color attachment's layers count must be the same with depth, couple couple desu
			fboShadow_->setDepthAttachment(texDepthMain_); // set back depth buffer
			fboShadow_->setColorAttachment(texColorMain_, 0, 0);
		}

	}
	// set back to normal
	// switchCullFace(true);
	renderer_.restoreViewPort();
}

void RenderPassShadow::setupBuffers() {
	// renderer_.setupColorBuffer(texColorMain_, renderer_.width(), renderer_.height(), false, false);
	renderer_.setupDepthBuffer(texDepthMain_, false, false);
	// renderer_.setupShadowMapBuffer(texDepthMain_, renderer_.width(), renderer_.height(), false, false);

	// create fbo
	if (!fboShadow_) {
		fboShadow_ = renderer_.createFrameBuffer(true);
	}

	// fboShadow_->setColorAttachment(texColorMain_, 0, 0);
	fboShadow_->disableForColorReading();
	fboShadow_->disableForColorWriting();
	fboShadow_->setDepthAttachment(texDepthMain_);
}

void RenderPassShadow::init() {
	// init and resize depth Buffer
	setupBuffers();
}

std::shared_ptr<FrameBuffer> RenderPassShadow::getFramebufferMain() {
	return fboShadow_;
}


