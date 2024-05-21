#include "ViewerOpenGL.h"
#include "RendererOpenGL.h"

ViewerOpenGL::ViewerOpenGL(Camera& camera, Config& config) : Viewer(camera, config) {}

std::shared_ptr<Renderer> ViewerOpenGL::createRenderer() {
	auto renderer = std::make_shared<RendererOpenGL>(cameraMain_);
	renderer->init();
	return renderer;
}
