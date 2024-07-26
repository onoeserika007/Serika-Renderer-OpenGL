#include "Renderer.h"

#include <Light.h>
#include <../include/Material/Shader.h>

#include "FCamera.h"
#include "FScene.h"
#include "../include/Geometry/Object.h"

#include "../include/Material/Uniform.h"
#include "Base/Globals.h"
#include "Geometry/UMesh.h"

Renderer::Renderer(const std::shared_ptr<FCamera> &camera) {
	// 在基类的构造函数里调用虚函数是无效的！会调用基类的实现，但是基类是纯虚函数，没有实现，所以会发生链接错误
	// modelUniformBlock_ = createUniformBlock("Model", sizeof(ModelUniformBlock));
	mainCamera_ = camera;
	viewCamera_ = camera;
}

void Renderer::loadGlobalUniforms(const std::shared_ptr<UMesh> &mesh) {
	const auto pmat = mesh->getMaterial();
	// uniforms will be loaded to shader when Material::use(ShaderPass) is called.
	pmat->setUniformBlock(modelUniformBlock_->name(), modelUniformBlock_);
	pmat->setUniformBlock(lightUniformBlock_->name(), lightUniformBlock_);
	pmat->setUniformBlock(shadowUniformBlock_->name(), shadowUniformBlock_);
	if (auto&& shadowMap = shadowMapUniformSampler_.lock()) {
		pmat->setUniformSampler(shadowMap->name(), shadowMap);
	}
	if (auto&& envMap = environmentMappingCubeSampler_.lock()) {
		pmat->setUniformSampler(envMap->name(), envMap);
	}
	if (auto&& cubeShadow = shadowMapCubeUniformSampler_.lock()) {
		pmat->setUniformSampler(cubeShadow->name(), cubeShadow);
	}
}

void Renderer::loadGlobalUniforms(Shader &program) {
	program.setUniformBlock(modelUniformBlock_->name(), modelUniformBlock_);
	program.setUniformBlock(lightUniformBlock_->name(), lightUniformBlock_);
	program.setUniformBlock(shadowUniformBlock_->name(), shadowUniformBlock_);
	if (auto&& shadowMap = shadowMapUniformSampler_.lock()) {
		program.setUniformSampler(shadowMap->name(), shadowMap);
	}
	if (auto&& envMap = environmentMappingCubeSampler_.lock()) {
		program.setUniformSampler(envMap->name(), envMap);
	}
	if (auto&& cubeShadow = shadowMapCubeUniformSampler_.lock()) {
		program.setUniformSampler(cubeShadow->name(), cubeShadow);
	}
}

void Renderer::updateModelUniformBlock(const std::shared_ptr<UMesh> &mesh, const std::shared_ptr<FCamera> &camera, const std::shared_ptr<ULight> &shadowLight) const {
	auto&& config = Config::getInstance();

	ModelUniformBlock tmp{};
	tmp.uModel = glm::mat4(1.f);
	if (mesh) {
		tmp.uModel = mesh->getWorldMatrix();
		tmp.uNormalToWorld = mesh->getNormalToWorld();
	}

	if (camera) {
		tmp.uProjection = camera->GetProjectionMatrix();
		tmp.uView = camera->GetViewMatrix();
		tmp.uViewPos = camera->position();;
	}
	// shadow mapping
	if ((mesh && mesh->castShadow() || !mesh) && config.bShadowMap && shadowLight) {
		auto&& shadowCamera = shadowLight->getLightCamera();
		auto&& shadowMapSampler = shadowLight->getShadowMap(*this)->getUniformSampler(*this);
		if (shadowLight->isPointLight()) {
			tmp.uUseShadowMap = false;
			tmp.uUseShadowMapCube = true;
			updateShadowCubeUniformBlock(shadowLight);
			shadowMapCubeUniformSampler_ = shadowMapSampler;
		}
		else {
			tmp.uUseShadowMap = true;
			tmp.uUseShadowMapCube = false;
			tmp.uShadowMapMVP = shadowCamera->GetProjectionMatrix() * shadowCamera->GetViewMatrix() * tmp.uModel;
			// set ShadowMap
			shadowMapUniformSampler_ = shadowMapSampler;
		}
	}
	else {
		tmp.uUseShadowMap = false;
		tmp.uUseShadowMapCube = false;
	}
	// eviroment mapping
	auto&& scene = renderingScene_.lock();
	if (config.bSkybox && scene && scene->skybox_) {
		environmentMappingCubeSampler_ = scene->skybox_->tryGetSkyboxSampler(*this);
		tmp.uUseEnvmap = true;
	}
	else {
		tmp.uUseEnvmap = false;
	}
	modelUniformBlock_->setData(&tmp, sizeof(ModelUniformBlock));
};;

void Renderer::updateShadowCubeUniformBlock(const std::shared_ptr<ULight> &shadowLight) const {
	auto&& config = Config::getInstance();
	if (!config.bShadowMap) return;

	ShadowCubeUniformBlock block {};
	if (shadowLight && shadowLight->isPointLight()) {
		auto&& lightCamera = shadowLight->getLightCamera();
		auto&& proj = lightCamera->GetProjectionMatrix();
		block.uFarPlane = lightCamera->getFarPlane();
		block.uShadowVPs[0] = proj * lightCamera->GetViewMatrix({1.f, 0.f, 0.f}, {0.f, -1.f, 0.f});		// positive x
		block.uShadowVPs[1] = proj * lightCamera->GetViewMatrix({-1.f, 0.f, 0.f}, {0.f, -1.f, 0.f});	// negative x
		block.uShadowVPs[2] = proj * lightCamera->GetViewMatrix({0.f, 1.f, 0.f}, {0.f, 0.f, 1.f});		// positive y
		block.uShadowVPs[3] = proj * lightCamera->GetViewMatrix({0.f, -1.f, 0.f}, {0.f, 0.f, -1.f});	// negative y
		block.uShadowVPs[4] = proj * lightCamera->GetViewMatrix({0.f, 0.f, 1.f}, {0.f, -1.f, 0.f});		// positive z
		block.uShadowVPs[5] = proj * lightCamera->GetViewMatrix({0.f, 0.f, -1.f}, {0.f, -1.f, 0.f});		// negative z
	}
	shadowUniformBlock_->setData(&block, sizeof(ShadowCubeUniformBlock));
}

void Renderer::updateLightUniformBlock(const std::shared_ptr<ULight>& light) const {
	// 现在可以确定内存布局没问题了，只能是c++端的问题，light初始化有问题
	LightDataUniformBlock tmp;
	if (light) {
		tmp = light->serialize();
	}
	else {
		tmp.uLightType = LightType_NoLight;
	}
	lightUniformBlock_->setData(&tmp, sizeof(LightDataUniformBlock));
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
	// std::cout << "Base renderer here!" << std::endl;
}

std::shared_ptr<FCamera> Renderer::getCamera() const {
	return mainCamera_;
}

std::shared_ptr<FCamera> Renderer::getViewCamera() const {
	return viewCamera_;
}

void Renderer::setCamera(const std::shared_ptr<FCamera> &camera) {
	mainCamera_ = camera;
}

void Renderer::setBackToViewCamera() {
	setCamera(viewCamera_);
}

void Renderer::setRenderingScene(const std::shared_ptr<FScene> &scene) {
	renderingScene_ = scene;
}

int Renderer::width() const {
	return width_;
}

int Renderer::height() const {
	return height_;
}
