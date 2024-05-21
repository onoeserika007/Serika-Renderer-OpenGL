#include "Viewer.h"
#include "Camera.h"
#include "Utils/Logger.h"
#include "Texture.h"
#include "Renderer.h"
#include "RenderPass/RenderPassPlain.h"
#include "Model.h"
#include "Scene.h"
#include "Utils/OpenGLUtils.h"

Viewer::Viewer(Camera& camera, Config& config) : cameraMain_(camera), config_(config) {
}

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

	setViewPort(0, 0, width_, height_);

	//// create resources
	//shadowPlaceholder_ = Texture::createTexture2DDefault(1, 1, TextureFormat_FLOAT32, TextureUsage_Sampler);

	// passes
	plainPass_ = std::make_shared<RenderPassPlain>(*renderer_);
	plainPass_->init();
}

void Viewer::setViewPort(int x, int y, int width, int height)
{
	width_ = width;
	height_ = height;
	if (renderer_) {
		renderer_->setViewPort(x, y, width, height);
	}
}

void Viewer::cleanup()
{
	if (renderer_) {
		renderer_->waitIdle();
	}
	// main fbo 
	//fboMain_ = nullptr;
	//texColorMain_ = nullptr;
	//texDepthMain_ = nullptr;

	// shadow map
	//fboShadow_ = nullptr;
	//texDepthShadow_ = nullptr;
	//shadowPlaceholder_ = nullptr;
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


void Viewer::drawScene(std::shared_ptr<Scene> scene)
{
	/*
	* PlainPass
	*/
	ClearStates clearStatesPlainPass;
	clearStatesPlainPass.clearColor = clearColor;
	clearStatesPlainPass.colorFlag = true;
	clearStatesPlainPass.depthFlag = true;

	renderer_->beginRenderPass(plainPass_->getFramebufferMain(), clearStatesPlainPass);
	for (auto& model : scene->getModels()) {
		for (auto& mesh : model->getMeshes()) {
			plainPass_->render(*mesh);
		}
	}
	renderer_->endRenderPass();
	
	/*
	* ToScreenPass
	*/
	ClearStates clearStatesToScreenPass;
	clearStatesToScreenPass.clearColor = clearColor;
	clearStatesToScreenPass.colorFlag = true;
	clearStatesToScreenPass.depthFlag = true;

	renderer_->beginRenderPass(nullptr, clearStatesPlainPass);
	auto outTex = plainPass_->getTexColorSampler();
	renderer_->renderToScreen(*outTex, width_, height_);
	renderer_->endRenderPass();
}

void Viewer::drawModel(std::shared_ptr<Model> model)
{
	auto& meshes = model->getMeshes();
	for (auto& mesh : meshes) {
		plainPass_->render(*mesh);
	}
}

