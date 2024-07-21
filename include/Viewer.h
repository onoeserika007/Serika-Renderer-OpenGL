#pragma once
#include <memory>
#include "Base/Config.h"

#include "Camera.h"

class RenderPassShadow;
class RenderPassLight;
class RenderPassGeometry;
class Renderer;
class Texture;
class FrameBuffer;

// 错误地将struct前向声明为class也会引起链接错误
// struct Config;
class UObject;
class RenderPassForwardShading;
class Scene;
class UModel;

class Viewer {
public:
	Viewer(const std::shared_ptr<Camera>& camera);
	virtual ~Viewer();
	void init(int width, int height, int outTexId);
	void setViewPort(int x, int y, int width, int height);
	void cleanup();
	void render(std::shared_ptr<Scene> scene);
	void drawScene(std::shared_ptr<Scene> scene);
	void drawScene_DefferedRendering(std::shared_ptr<Scene> scene);
	void drawScene_ShadowMapTest(std::shared_ptr<Scene> scene);
	void drawModel(std::shared_ptr<UModel> model);

	std::shared_ptr<Camera> createCamera(CameraType type);
	virtual std::shared_ptr<Renderer> createRenderer() = 0;

public:

protected:
	std::shared_ptr<Camera> cameraMain_;
	std::shared_ptr<Camera> cameraDepth_ = nullptr;

	// scene
	int width_ = 0;
	int height_ = 0;
	int outTexId_ = 0;

	std::shared_ptr<Renderer> renderer_ = nullptr;

	glm::vec4 clearColor{ 0.2f, 0.3f, 0.3f, 1.0f };
	const glm::vec4 BLACK_COLOR {0.f, 0.f, 0.f, 1.f};

	// 现在暂定不对renderpass进行循环渲染，而是手动装配管线，即手动接入和输出renderpass的结果

	// main fbo
	//std::shared_ptr<FrameBuffer> fboMain_ = nullptr;
	//std::shared_ptr<Texture> texColorMain_ = nullptr;
	//std::shared_ptr<Texture> texDepthMain_ = nullptr;

	// shadow map
	//std::shared_ptr<FrameBuffer> fboShadow_ = nullptr;
	//std::shared_ptr<Texture> texDepthShadow_ = nullptr;
	//std::shared_ptr<Texture> shadowPlaceholder_ = nullptr;

	// renderpasses
	std::shared_ptr<RenderPassForwardShading> plainPass_ = nullptr;
	std::shared_ptr<RenderPassGeometry> geometryPass_ = nullptr;
	std::shared_ptr<RenderPassLight> lightPass_ = nullptr;
	std::shared_ptr<RenderPassShadow> shadowPass_ = nullptr;
private:

};