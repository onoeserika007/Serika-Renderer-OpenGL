#include "RenderPass/RenderPassGeometry.h"

#include <FScene.h>

#include "Renderer.h"
#include "FrameBuffer.h"

// This Name Must be aligned with which in shader!!!
const char* RenderPassGeometry::GBUFFER_NAMES[] = {
	"gPosition_ViewDepth",
	"gDiffuse",
	"gNormal_Specular",
	"gAo_Metal_Roughness"
};

RenderPassGeometry::RenderPassGeometry(const std::shared_ptr<Renderer>& renderer): RenderPass(renderer)
{
	shaderPass_ = ShaderPass::Shader_Geometry_Pass;
}

void RenderPassGeometry::init()
{
	gbufferTextures_.resize(GBUFFER_NUM_TEXTURES);
	// init and resize depth Buffer
	setupBuffers();

}

void RenderPassGeometry::setupBuffers()
{
	// create fbo
	if (!fboGbuffer_) {
		fboGbuffer_ = renderer_->createFrameBuffer(true);
	}

	if (!ssaoFbo_) {
		ssaoFbo_ = renderer_->createFrameBuffer(true);
	}

	for (int i = 0; i < GBUFFER_NUM_TEXTURES; i++) {
		auto& colorAttachment = gbufferTextures_[i];
		// 位置、法向量信息一定要选择高精度存储！否则后果很严重
		if (renderer_->setupColorBuffer(colorAttachment, renderer_->width(), renderer_->height(), false, false,
			TextureTarget_TEXTURE_2D, TextureFormat_RGBA16F)) {
			fboGbuffer_->setColorAttachment(colorAttachment, 0, i);
		}
	}

	if (renderer_->setupDepthBuffer(depthTexture_, false, true)) {
		fboGbuffer_->setDepthAttachment(depthTexture_);
	}

	if (renderer_->setupColorBuffer(ssaoMid_, renderer_->width(), renderer_->height(), false, false,
		TextureTarget_TEXTURE_2D, TextureFormat_R16F)) {
		ssaoFbo_->setColorAttachment(ssaoMid_, 0, 0);
	}

	if (renderer_->setupColorBuffer(ssaoResult_, renderer_->width(), renderer_->height(), false, false,
		TextureTarget_TEXTURE_2D, TextureFormat_R16F)) {
		ssaoFbo_->setColorAttachment(ssaoResult_, 0, 1);
	}
}

std::shared_ptr<FrameBuffer> RenderPassGeometry::getFramebufferMain() {
	return fboGbuffer_;
}

const std::vector<std::shared_ptr<Texture>> & RenderPassGeometry::getGBuffers() const {
	return gbufferTextures_;
}

void RenderPassGeometry::render(FScene & scene) {
	auto&& config = Config::getInstance();
	setupBuffers();
	fboGbuffer_->bind();
	fboGbuffer_->clearDepthBuffer();
	for (auto& model : scene.getPackedMeshes()) {
		renderer_->drawMesh(model, ShaderPass::Shader_Geometry_Pass, nullptr);
	}

	if (config.bUseSSAO) {
		// SSAO pass
		auto enableDepth = [this](bool bEnabled) {
			RenderStates render_states = renderer_->renderStates_;
			render_states.blend = false;
			render_states.depthMask = bEnabled;
			render_states.depthTest = bEnabled;
			render_states.cullFace = false;
			renderer_->updateRenderStates(render_states);
		};

		enableDepth(false);
		auto&& ssaoProgram = renderer_->getSSAOProgram(gbufferTextures_);
		renderer_->dump(ssaoProgram, false, ssaoFbo_, 0);

		enableDepth(false);
		auto&& ssaoBlurProgram = renderer_->getSSAOBlurProgram(ssaoMid_);
		renderer_->dump(ssaoBlurProgram, false, ssaoFbo_, 1);
		renderer_->setSSAOSampler(ssaoResult_->getUniformSampler(*renderer_));
	}
}

