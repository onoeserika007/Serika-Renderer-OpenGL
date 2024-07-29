#pragma once
// #define GLM_FORCE_ALIGNED
#define GLM_FORCE_INLINE

#include <glm.hpp>
#include <glm/gtc/type_ptr.hpp>

using RGBA = glm::u8vec4;

#define DEBUG
// #undef DEBUG

constexpr float M_PI = 3.14159265358979323846f;
constexpr float M_EPSILON = 1e-6;
constexpr float FLOAT_MAX = std::numeric_limits<float>::max();
// constexpr float FLOAT_MIN = std::numeric_limits<float>::min(); // 对这个理解错误了，这不是最小的值，这是最小的norm值
constexpr float FLOAT_MIN = -std::numeric_limits<float>::max();

constexpr glm::vec3 WHITE_COLOR {0.725f, 0.71f, 0.68f};
constexpr glm::vec3 BLACK_COLOR {0.001f, 0.001f, 0.001f};
constexpr glm::vec3 RED_COLOR {0.63f, 0.065f, 0.05f};
constexpr glm::vec3 GREEN_COLOR {0.14f, 0.45f, 0.091f};

constexpr glm::vec3 M_X_POSITIVE_UNIT {1.f, 0.f, 0.f};
constexpr glm::vec3 M_Y_POSITIVE_UNIT {0.f, 1.f, 0.f};
constexpr glm::vec3 M_Z_POSITIVE_UNIT {0.f, 0.f, 1.f};

#define NO_DISCARD [[nodiscard]]

struct ModelUniformBlock{
	alignas(64)	glm::mat4 uModel {1.f};
	alignas(64)	glm::mat4 uView {1.f};
	alignas(64)	glm::mat4 uProjection {1.f};
	alignas(64)	glm::mat4 uNormalToWorld {1.f};
	alignas(64)	glm::mat4 uShadowMapVP {1.f};
	alignas(16)	glm::vec3 uViewPos {1.f};
	alignas(4)	glm::int32 uUseShadowMap = false;
	alignas(4)	glm::int32 uUseShadowMapCube = false;
	alignas(4)	glm::int32 uUseEnvmap = false;
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
	Shading_Lambertian,
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
