#include "Renderer.h"
#include "Uniform.h"
#include "Base/GLMInc.h"

Renderer::Renderer(Camera& camera) : camera_(camera) {
	// 在基类的构造函数里调用虚函数是无效的！会调用基类的实现，但是基类是纯虚函数，没有实现，所以会发生链接错误
	// modelUniformBlock_ = createUniformBlock("Model", sizeof(ModelUniformBlock));
}

Renderer::~Renderer()
{
	std::cout << "Base renderer here!" << std::endl;
}

Camera& Renderer::getCamera()
{
	return camera_;
}

int Renderer::width()
{
	return width_;
}

int Renderer::height()
{
	return height_;
}
