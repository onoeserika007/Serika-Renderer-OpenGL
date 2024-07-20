#include "RenderPass/RenderPass.h"
#include "Texture.h"
#include "Renderer.h"

void RenderPass::setupColorBuffer(std::shared_ptr<Texture>& colorBuffer, bool multiSample, bool force) {
	int w = renderer_.width(), h = renderer_.height();
	if (w != width_ || h != height_) {
		force = true;
		width_ = w;
		height_ = h;
	}

	if (!colorBuffer || colorBuffer->multiSample() != multiSample || force) {
		TextureInfo texInfo{};
		texInfo.width = width_;
		texInfo.height = height_;
		texInfo.target = TextureTarget::TextureTarget_2D;
		texInfo.format = TextureFormat::TextureFormat_RGBA8;
		texInfo.usage = TextureUsage_AttachmentColor | TextureUsage_RendererOutput;
		texInfo.multiSample = multiSample;
		texInfo.useMipmaps = false;

		SamplerInfo smInfo{};
		smInfo.filterMag = Filter_LINEAR;
		smInfo.filterMin = Filter_LINEAR;

		colorBuffer = renderer_.createTexture(texInfo, smInfo);

		// now loading is handled in constructor, manual loading is nolonger needed.
		// load to pipeline
		//colorBuffer->setupPipeline(renderer_);
	}
}

void RenderPass::setupDepthBuffer(std::shared_ptr<Texture>& depthBuffer, bool multiSample, bool force) {
	int w = renderer_.width(), h = renderer_.height();
	if (w != width_ || h != height_) {
		force = true;
		width_ = w;
		height_ = h;
	}

	if (!depthBuffer || depthBuffer->multiSample() != multiSample || force) {
		TextureInfo texInfo{};
		texInfo.width = width_;
		texInfo.height = height_;
		texInfo.target = TextureTarget::TextureTarget_2D;
		texInfo.format = TextureFormat::TextureFormat_FLOAT32;
		texInfo.usage = TextureUsage::TextureUsage_AttachmentDepth;
		texInfo.multiSample = multiSample;
		texInfo.useMipmaps = false;

		SamplerInfo smInfo{};
		smInfo.filterMag = Filter_NEAREST;
		smInfo.filterMin = Filter_NEAREST;

		depthBuffer = renderer_.createTexture(texInfo, smInfo);
	}
}

void RenderPass::setupShadowMapBuffer(std::shared_ptr<Texture> &depthBuffer, int width, int height, bool multiSample,
	bool force) {
	if (depthBuffer) {
		const TextureInfo& texInfo = depthBuffer->getTextureInfo();
		force = force || texInfo.width != width || texInfo.height != height;
		force = force || texInfo.target != TextureTarget_2D || texInfo.format != TextureFormat_FLOAT32;
	}

	if (!depthBuffer || depthBuffer->multiSample() != multiSample || force) {
		TextureInfo texInfo{};
		texInfo.width = width;
		texInfo.height = height;
		texInfo.target = TextureTarget::TextureTarget_2D;
		texInfo.format = TextureFormat::TextureFormat_FLOAT32;
		texInfo.usage = TextureUsage::TextureUsage_AttachmentColor | TextureUsage::TextureUsage_Sampler;
		texInfo.multiSample = multiSample;
		texInfo.useMipmaps = false;

		SamplerInfo smInfo{};
		smInfo.filterMag = Filter_NEAREST;
		smInfo.filterMin = Filter_NEAREST;
		smInfo.wrapS = Wrap_CLAMP_TO_EDGE;
		smInfo.wrapT = Wrap_CLAMP_TO_EDGE;

		depthBuffer = renderer_.createTexture(texInfo, smInfo);
	}
}

