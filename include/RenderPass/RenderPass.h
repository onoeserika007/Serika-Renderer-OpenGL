#pragma once
#include <iostream>
#include "Material.h"

class Scene;
class FrameBuffer;
class Renderer;
class Shader;
class UObject;
class Texture;
class UModel;

// 区分通用渲染过程和renderpass wides的渲染过程，前者放在renderer里，后面的setup放在renderpass里（暂定）
class RenderPass {
private:
protected:
	std::shared_ptr<Shader> shaderProgram_;
	Renderer& renderer_;
	ShaderPass shaderPass_ = ShaderPass::Shader_Plain_Pass;
public:
	RenderPass(Renderer& renderer) : renderer_(renderer) {}
	virtual void render(Scene & scene) = 0;
	virtual void setupBuffers() = 0;
	virtual void init() = 0;
	virtual std::shared_ptr<FrameBuffer> getFramebufferMain() = 0;
	void setupColorBuffer(std::shared_ptr<Texture>& colorBuffer, bool multiSample, bool force = false);
	void setupDepthBuffer(std::shared_ptr<Texture>& depthBuffer, bool multiSample, bool force = false);
	void setupShadowMapBuffer(std::shared_ptr<Texture>& depthBuffer, int width, int height, bool multiSample, bool force = false);
};