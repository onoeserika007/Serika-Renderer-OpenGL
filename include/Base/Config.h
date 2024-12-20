#pragma once
#include <mutex>
#include <string>

#include "Globals.h"

namespace json11 {
	class Json;
}

enum ERendererType {
	RendererType_SOFT,
	RendererType_OPENGL,
	RendererType_Vulkan,
};

enum ERenderPipeline {
	RenderMode_ForwardRendering,
	RenderMode_DeferredRendering,
	RenderMode_TestRendering_OffScreen,
	RenderMode_TestRendering_OnScreen,
	RenderMode_PathTracing
};

struct ConfigReadGuard;
struct ConfigWriteGuard;


// 修改这里前要删掉config，不然会崩溃
struct Config {

	static const std::string defaultConfigPath;
	// viewer
	int WindowWidth = 1920;
	int WindowHeight = 1080;
	bool bSkybox = false;;
	ERendererType RendererType = ERendererType::RendererType_OPENGL;
	ERenderPipeline RenderPipeline = ERenderPipeline::RenderMode_ForwardRendering;
	EShadingModel ShadingModelForDeferredRendering = EShadingModel::Shading_BlinnPhong;
	ESceneType SceneType = ESceneType::SceneType_Default;

	// geometry
	bool bUseBVH = false;

	// Default camera values
	float CameraYaw = -90.0f;
	float CameraPitch = 0.0f;
	float CameraSpeed = 2.5f;
	float MouseSensitivity = 0.1f;
	float CameraZoom = 90.0f;

	// shadow map
	bool bShadowMap = false;
	int Resolution_ShadowMap = 1024;
	float CameraNear = 0.1f;
	float CameraFar = 50.f;

	// perspective camera
	float CameraAspect = 1.f;
	float CameraFOV = 45.f;

	// Ortho camera
	float CaptureRadius_ShadowMap = 60.f;

	// Ray Tracing
	int SPP;

	// HDR
	bool bUseHDR = true;
	float Exposure = 1.f;

	// Bloom
	bool bUseBloom = true;

	// SSAO
	bool bUseSSAO = false;

	// Mipmaps
	bool bUseMipmaps = false;

	void serialize(const std::string& path);
	void deserialize(const std::string& path);
	static Config& getInstance();
	NO_DISCARD json11::Json to_json() const;
	Config& from_json(const json11::Json & j);

	Config(const Config&) = delete;
	Config& operator=(const Config&) = delete;
private:
	Config() = default;

	bool loaded_ = false;
	std::mutex load_lock_;
};

struct ConfigReadGuard {

};

struct ConfigWriteGuard {

	explicit ConfigWriteGuard(Config& config): config_((config)) {}
	Config& config_;
};