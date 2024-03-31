#include "Viewer.h"
#include "RendererOpenGL.h"

class ViewerOpenGL : public Viewer {
private:
	virtual std::shared_ptr<Renderer> createRenderer() override {
		auto renderer = std::make_shared<RendererOpenGL>();
		renderer->init();
		return renderer;
	}
public:
	ViewerOpenGL(Camera& camera, Config& config): Viewer(camera, config) {}


};