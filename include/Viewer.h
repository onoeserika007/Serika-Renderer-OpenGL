#pragma once
#include <memory>
#include "FCamera.h"
#include "ConfigPanel.h"
#include "GLFW/glfw3.h"

class OrbitController;
class ConfigPanel;
class RenderPassLight;
class RenderPassShadow;
class RenderPassGeometry;
class RenderPassForwardShading;
constexpr glm::vec4 clearColor{ 0.0f, 0.0f, 0.0f, 1.0f };

enum ECameraMode {
	CameraMode_None,
	CameraMode_FPSCamera,
	CameraMode_OrbitCamera,
};

// 错误地将struct前向声明为class也会引起链接错误
// struct configs;
class UObject;
class FScene;

class Viewer {
public:
	explicit Viewer() = default;
	__declspec(noinline) void init(void *window, int width, int height, int outTexId);
	void setViewPort(int x, int y, int width, int height);
	void setScene(const std::shared_ptr<FScene>& scene);

	void cleanup();
	void reloadScene();
	void DrawFrame();
	void drawScene_ForwardRendering(const std::shared_ptr<FScene> &scene) const;
	void drawScene_DefferedRendering(const std::shared_ptr<FScene>& scene);
	void drawScene_TestPipeline(const std::shared_ptr<FScene> &scene) const;
	void drawScene_OnScreen(const std::shared_ptr<FScene>& scene) const;

	// ray casting
	void drawScene_PathTracing_CPU(const std::shared_ptr<FScene>& scene) const;

	NO_DISCARD Ray screenToWorldRay(int mouseX, int mouseY, int screenWidth, int screenHeight,
	                     const std::shared_ptr<FCamera> &camera, bool bUseDisturb = false, float disturbRadius = 0, int SobolIndex = 0) const;
	void drawCursorHitDebugLine(int mouseX, int mouseY, int screenWidth, int screenHeight);
	void drawUnderCursorTraceDebugTriangle(int mouseX, int mouseY, int screenWidth, int screenHeight);

	// IOs
	void listenKeyEvents();
	void setCameraMode(ECameraMode camera_mode);

	void updateOrbitZoom(float x, float y);
	void updateOrbitRotate(float x, float y);
	void updateOrbitPan(float x, float y);

	bool wantCaptureKeyboard() const;
	bool wantCaptureMouse() const;

	void waitIdle();

	std::shared_ptr<FCamera> createCamera(CameraType type);
	std::shared_ptr<Renderer> createRenderer();

	NO_DISCARD std::shared_ptr<FCamera> getViewCamera() const;
	NO_DISCARD std::shared_ptr<Renderer> getRenderer() const { return renderer_; }

	// config panel
	void toggleShowConfigPanel();

private:
	void drawPanel() const;

public:

private:

	GLFWwindow* glfwWindow_ = nullptr;

	// config panel
	std::shared_ptr<ConfigPanel> configPanel_ = nullptr;
	bool bShowConfigPanel_ = true;

	// camera
	std::shared_ptr<FCamera> cameraMain_ = nullptr;
	std::shared_ptr<FCamera> cameraDepth_ = nullptr;
	std::shared_ptr<OrbitController> orbitController_ = nullptr;
	ECameraMode camera_mode_ = CameraMode_None;
	bool bOrbitCenterDirty = true;

	// scene
	int width_ = 0;
	int height_ = 0;
	int outTexId_ = 0;
	std::shared_ptr<FScene> scene_ = nullptr;

	std::shared_ptr<Renderer> renderer_{};
	std::shared_ptr<RenderPassForwardShading> plainPass_ = nullptr;
	std::shared_ptr<RenderPassGeometry> geometryPass_ = nullptr;
	std::shared_ptr<RenderPassShadow> shadowPass_ = nullptr;
	std::shared_ptr<RenderPassLight> lightPass_ = nullptr;
};