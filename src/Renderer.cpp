#include "Renderer.h"

#include <ULight.h>
#include <Shader.h>

#include "Camera.h"
#include "../include/Geometry/UObject.h"

#include "Uniform.h"
#include "Base/GLMInc.h"
#include "Geometry/UMesh.h"

Renderer::Renderer(Camera& camera) : camera_(camera) {
	// 在基类的构造函数里调用虚函数是无效的！会调用基类的实现，但是基类是纯虚函数，没有实现，所以会发生链接错误
	// modelUniformBlock_ = createUniformBlock("Model", sizeof(ModelUniformBlock));
}

void Renderer::updateModelUniformBlock(UMesh & mesh, Camera &camera, const std::shared_ptr<Camera> &shadowCamera) const {
	ModelUniformBlock tmp{};
	tmp.uModel = mesh.getModelMatrix();
	tmp.uNormalToWorld = mesh.getNormalToWorld();
	tmp.uProjection = camera.GetProjectionMatrix();
	tmp.uView = camera.GetViewMatrix();
	tmp.uViewPos = camera.position();
	if (shadowCamera) {
		tmp.uShadowMapMVP = shadowCamera->GetProjectionMatrix() * shadowCamera->GetViewMatrix() * tmp.uModel;
	}

	modelUniformBlock_->setData(&tmp, sizeof(ModelUniformBlock));

	const auto pmat = mesh.getpMaterial();
	pmat->setUniform(modelUniformBlock_->name(), modelUniformBlock_);
}

void Renderer::updateLightUniformBlock(Shader &shader, const std::shared_ptr<ULight>& light) const {
	LightDataUniformBlock tmp;
	if (light) {
		tmp = light->serialize();
	}
	else {
		tmp.uLightType = LightType_NoLight;
	}
	lightUniformBlock_->setData(&tmp, sizeof(LightDataUniformBlock));
	shader.use();
	shader.bindUniform(*lightUniformBlock_);
}

void Renderer::updateLightUniformBlock(UMesh &mesh, const std::shared_ptr<ULight>& light) const {
	// 现在可以确定内存布局没问题了，只能是c++端的问题，light初始化有问题
	LightDataUniformBlock tmp;
	if (light) {
		tmp = light->serialize();
	}
	else {
		tmp.uLightType = LightType_NoLight;
	}
	lightUniformBlock_->setData(&tmp, sizeof(LightDataUniformBlock));

	const auto pmat = mesh.getpMaterial();
	pmat->setUniform(lightUniformBlock_->name(), lightUniformBlock_); // uniforms will be loaded to shader when Material::use(ShaderPass) is called.
}

void Renderer::setViewPort(int x, int y, int width, int height) {
	viewer_x_ = x;
	viewer_y_ = y;
	viewer_width_ = width;
	viewer_height_ = height;
	setRenderViewPort(x, y, width, height);
}

void Renderer::restoreViewPort() {
	setRenderViewPort(viewer_x_, viewer_y_, viewer_width_, viewer_height_);
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
