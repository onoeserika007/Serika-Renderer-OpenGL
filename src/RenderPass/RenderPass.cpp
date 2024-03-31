#include "RenderPass/RenderPass.h"
#include "Texture.h"
#include "Renderer.h"

void RenderPass::render(Model& model)
{
	
}

inline void RenderPass::setupColorBuffer(std::shared_ptr<Texture>& colorBuffer, bool multiSample, bool force) {
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
		colorBuffer->setupPipeline(renderer_);
	}
}

inline void RenderPass::setupDepthBuffer(std::shared_ptr<Texture>& depthBuffer, bool multiSample, bool force) {
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
		depthBuffer->setupPipeline(renderer_);
	}
}
