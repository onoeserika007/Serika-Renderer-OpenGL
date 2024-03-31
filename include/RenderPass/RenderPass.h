#pragma once
#include <iostream>
#include "Material.h"

class FrameBuffer;
class Renderer;
class Shader;
class Object;
class Model;

// 区分通用渲染过程和renderpass wides的渲染过程，前者放在renderer里，后面的setup放在renderpass里（暂定）
class RenderPass {
private:
protected:
	std::shared_ptr<FrameBuffer> renderTarget_;
	std::shared_ptr<Shader> shaderProgram_;
	Renderer& renderer_;
	ShaderPass shaderPass_ = Shader_MainRender_Pass;
public:
	RenderPass(Renderer& renderer) : renderer_(renderer) {}
	virtual void render(Object& obj) = 0;
	void render(Model& model);
	void setRenderTarget(std::shared_ptr<FrameBuffer> target) {
		renderTarget_ = target;
	}
};