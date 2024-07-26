#pragma once
#include <memory>
#include "Base/Config.h"

#include "FCamera.h"

class RenderPassShadow;
class RenderPassLight;
class RenderPassGeometry;
class Renderer;
class Texture;
class FrameBuffer;

// 错误地将struct前向声明为class也会引起链接错误
// struct configs;
class UObject;
class RenderPassForwardShading;
class FScene;
class UModel;

class Viewer {
public:
	Viewer(const std::shared_ptr<FCamera>& camera);
	virtual ~Viewer();
	void init(int width, int height, int outTexId);
	void setViewPort(int x, int y, int width, int height);
	void cleanup();
	void render(const std::shared_ptr<FScene> &scene);
	void drawScene_ForwardRendering(const std::shared_ptr<FScene> &scene) const;
	void drawScene_DefferedRendering(std::shared_ptr<FScene> scene);
	void drawScene_TestPipeline(const std::shared_ptr<FScene> &scene) const;
	void drawScene_OnScreen(const std::shared_ptr<FScene>& scene) const;

	void waitIdle();

	std::shared_ptr<FCamera> createCamera(CameraType type);
	std::shared_ptr<Renderer> createRenderer();

public:

protected:
	std::shared_ptr<FCamera> cameraMain_;
	std::shared_ptr<FCamera> cameraDepth_ = nullptr;

	// scene
	int width_ = 0;
	int height_ = 0;
	int outTexId_ = 0;

	std::shared_ptr<Renderer> renderer_ = nullptr;

	glm::vec4 clearColor{ 0.2f, 0.3f, 0.3f, 1.0f };
	const glm::vec4 BLACK_COLOR {0.f, 0.f, 0.f, 1.f};

	// renderpasses
	std::shared_ptr<RenderPassForwardShading> plainPass_ = nullptr;
	std::shared_ptr<RenderPassGeometry> geometryPass_ = nullptr;
	std::shared_ptr<RenderPassLight> lightPass_ = nullptr;
	std::shared_ptr<RenderPassShadow> shadowPass_ = nullptr;
private:

};