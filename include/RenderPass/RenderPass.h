#pragma once
#include "Material/FMaterial.h"

class FScene;
class FrameBuffer;
class Renderer;
class Shader;
class UObject;
class Texture;

// 区分通用渲染过程和renderpass wides的渲染过程，前者放在renderer里，后面的setup放在renderpass里（暂定）
class RenderPass {
private:
protected:
	std::shared_ptr<Renderer> renderer_;
	ShaderPass shaderPass_ = ShaderPass::Shader_ForwardShading_Pass;
public:
	explicit RenderPass(const std::shared_ptr<Renderer>& renderer) : renderer_(renderer) {}
	virtual ~RenderPass() = default;
	virtual void render(FScene & scene) = 0;
	virtual void setupBuffers() = 0;
	virtual void init() = 0;
	virtual std::shared_ptr<FrameBuffer> getFramebufferMain() = 0;
};