#pragma once
#include <memory>
#include "Camera.h"

class Renderer;
class Texture;
class FrameBuffer;

// 错误地将struct前向声明为class也会引起链接错误
struct Config;
class Object;
class RenderPassPlain;
class Scene;
class Model;

class Viewer {
private:
protected:
	Config& config_;
	Camera& cameraMain_;
	std::shared_ptr<Camera> cameraDepth_ = nullptr;

	// scene

	int width_ = 0;
	int height_ = 0;
	int outTexId_ = 0;

	std::shared_ptr<Renderer> renderer_ = nullptr;

	glm::vec4 clearColor{ 0.2f, 0.3f, 0.3f, 1.0f };

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
	std::shared_ptr<RenderPassPlain> plainPass_ = nullptr;

public:
	Viewer(Camera& camera, Config& config);
	void init(int width, int height, int outTexId);
	void setViewPort(int x, int y, int width, int height);
	void cleanup();

	std::shared_ptr<Camera> createCamera(CameraType type);
	virtual std::shared_ptr<Renderer> createRenderer() = 0;
	virtual void drawScene(std::shared_ptr<Scene> scene);
	virtual void drawModel(std::shared_ptr<Model> model);
};