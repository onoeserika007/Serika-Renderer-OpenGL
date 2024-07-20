#include "ViewerOpenGL.h"
#include "RendererOpenGL.h"

ViewerOpenGL::ViewerOpenGL(const std::shared_ptr<Camera>& camera) : Viewer(camera) {}

std::shared_ptr<Renderer> ViewerOpenGL::createRenderer() {
	auto renderer = std::make_shared<RendererOpenGL>(cameraMain_);
	renderer->init();
	return renderer;
}
