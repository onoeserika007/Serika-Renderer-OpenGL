#pragma once

#include "../Viewer.h"

class Renderer;

class ViewerOpenGL : public Viewer {
private:

public:
	explicit ViewerOpenGL(const std::shared_ptr<Camera>& camera);
	virtual std::shared_ptr<Renderer> createRenderer() override;
};