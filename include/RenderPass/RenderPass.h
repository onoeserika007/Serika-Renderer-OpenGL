#pragma once
#include <iostream>
#include "Material.h"

class FrameBuffer;
class Renderer;
class Shader;
class Object;
class Texture;
class Model;

// 区分通用渲染过程和renderpass wides的渲染过程，前者放在renderer里，后面的setup放在renderpass里（暂定）
class RenderPass {
private:
protected:
	int width_ = 0;
	int height_ = 0;
	std::shared_ptr<Shader> shaderProgram_;
	Renderer& renderer_;
	ShaderPass shaderPass_ = Shader_MainRender_Pass;
public:
	RenderPass(Renderer& renderer) : renderer_(renderer) {}
	virtual void render(Object& obj) = 0;
	virtual void resetBufferSize() = 0;
	void render(Model& model);
	void setupColorBuffer(std::shared_ptr<Texture>& colorBuffer, bool multiSample, bool force = false);
	void setupDepthBuffer(std::shared_ptr<Texture>& depthBuffer, bool multiSample, bool force = false);
};