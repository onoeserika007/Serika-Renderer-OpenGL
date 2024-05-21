#pragma once

#include "Viewer.h"

class Renderer;

class ViewerOpenGL : public Viewer {
private:

public:
	ViewerOpenGL(Camera& camera, Config& config);
	virtual std::shared_ptr<Renderer> createRenderer() override;
};