// learnOpenGL-aTriangle.h: 标准系统包含文件的包含文件
// 或项目特定的包含文件。

#pragma once
// #define GLM_FORCE_ALIGNED
#define GLM_FORCE_INLINE

#include <glm.hpp>
#include <glm/gtc/type_ptr.hpp>

using RGBA = glm::u8vec4;

#define DEBUG
// #undef DEBUG

constexpr double M_PI = 3.14159265358979323846f;
constexpr float M_EPSILON = 1e-6;

#define NO_DISCARD [[nodiscard]]

struct ModelUniformBlock{
	alignas(64)	glm::mat4 uModel;
	alignas(64)	glm::mat4 uView;
	alignas(64)	glm::mat4 uProjection;
	alignas(64)	glm::mat4 uNormalToWorld;
	alignas(64)	glm::mat4 uShadowMapMVP;
	alignas(16)	glm::vec3 uViewPos;
	alignas(4)	glm::int32 uUseShadowMap;
	alignas(4)	glm::int32 uUseShadowMapCube;
	alignas(4)	glm::int32 uUseEnvmap;
};

struct ShadowCubeUniformBlock {
	glm::mat4	uShadowVPs[6];
	alignas(4)	glm::float32 uFarPlane;
};

enum WrapMode {
	Wrap_REPEAT,
	Wrap_MIRRORED_REPEAT,
	Wrap_CLAMP_TO_EDGE,
	Wrap_CLAMP_TO_BORDER,
};

enum FilterMode {
	Filter_NEAREST,
	Filter_LINEAR,
	Filter_NEAREST_MIPMAP_NEAREST,
	Filter_LINEAR_MIPMAP_NEAREST,
	Filter_NEAREST_MIPMAP_LINEAR,
	Filter_LINEAR_MIPMAP_LINEAR,
};

enum EShadingModel {
	Shading_Unknown = 0,
	Shading_BaseColor,
	Shading_BlinnPhong,
	Shading_PBR,
	Shading_Skybox,
	Shading_IBL_Irradiance,
	Shading_IBL_Prefilter,
	Shading_FXAA,
};

enum class ShaderPass: uint8_t {
	Shader_Shadow_Pass,
	Shader_Shadow_Cube_Pass,
	Shader_ForwardShading_Pass,
	Shader_Geometry_Pass,
	Shader_Light_Pass,
};

struct MaterialInfo {
	EShadingModel shading_model;
	glm::vec3 emission;
	float ior;
	glm::vec3 Kd;
	glm::vec3 Ks;
	float specularExponent;

	bool hasEmission() const { return glm::length(emission) > 0.f; }
};


// TODO: 在此处引用程序需要的其他标头。
