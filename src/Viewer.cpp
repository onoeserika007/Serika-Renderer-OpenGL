#include "Viewer.h"
#include "Camera.h"
#include "Utils/Logger.h"
#include "Texture.h"
#include "Renderer.h"

void Viewer::init(int width, int height, int outTexId)
{
	cleanup();

	width_ = width;
	height_ = height;
	outTexId_ = outTexId;

	// depth camera
	if (!cameraDepth_) {
		cameraDepth_ = std::make_shared<OrthographicCamera>();
	}

	// renderer
	if (!renderer_) {
		renderer_ = createRenderer();
	}
	if (!renderer_) {
		LOGE("Viewer::create failed: createRenderer error");
	}

	// create resources
	shadowPlaceholder_ = Texture::createTexture2DDefault(1, 1, TextureFormat_FLOAT32, TextureUsage_Sampler);
}

void Viewer::cleanup()
{
	if (renderer_) {
		renderer_->waitIdle();
	}
	// main fbo 
	fboMain_ = nullptr;
	texColorMain_ = nullptr;
	texDepthMain_ = nullptr;

	// shadow map
	fboShadow_ = nullptr;
	texDepthShadow_ = nullptr;
	shadowPlaceholder_ = nullptr;
}

std::shared_ptr<Camera> Viewer::createCamera(CameraType type)
{
	if (type == CameraType::PERSPECTIVE) {
		return std::make_shared<PerspectiveCamera>();
	}
	else if (type == CameraType::ORTHOGRAPHIC) {
		return std::make_shared<OrthographicCamera>();
	}
	return nullptr;
}

std::shared_ptr<Renderer> Viewer::createRenderer()
{
	return nullptr;
}

void Viewer::drawObject(Object& obj)
{

}

Viewer::Viewer(Camera& camera, Config& config) : cameraMain_(camera), config_(config)
{
}

