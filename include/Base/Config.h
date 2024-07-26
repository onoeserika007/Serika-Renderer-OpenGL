#pragma once
#include "Material/FMaterial.h"

namespace json11 {
	class Json;
}

enum ERendererType {
	RendererType_SOFT,
	RendererType_OPENGL,
	RendererType_Vulkan,
};

enum class ERenderMode: int {
	RenderMode_ForwardRendering,
	RenderMode_DefferedRendering,
	RenderMode_TestRendering_OffScreen,
	RenderMode_TestRendering_OnScreen
};

// 修改这里前要删掉config，不然会崩溃
struct Config {
	// viewer
	int WindowWidth = 1920;
	int WindowHeight = 1080;
	bool bShadowMap = false;
	bool bSkybox = false;;
	ERendererType RendererType = ERendererType::RendererType_OPENGL;
	ERenderMode RenderMode = ERenderMode::RenderMode_ForwardRendering;

	// geometry
	bool bUseBVH = false;

	// Default camera values
	float CameraYaw = -90.0f;
	float CameraPitch = 0.0f;
	float CameraSpeed = 2.5f;
	float MouseSensitivity = 0.1f;
	float CameraZoom = 90.0f;

	// shadow map
	int Resolution_ShadowMap = 1024;

	float CameraNear = 0.1f;
	float CameraFar = 50.f;
	// perspective camera
	float CameraAspect = 1.f;
	float CameraFOV = 45.f;
	// Ortho camera
	float CaptureRadius_ShadowMap = 20.f;


	void serialize(const std::string& path);
	void deserialize(const std::string& path);
	static Config& getInstance();
	json11::Json to_json() const;
	Config& from_json(const json11::Json & j);
private:
	Config() = default;
	Config(const Config&) = default;
	Config& operator=(const Config&) = default;

};