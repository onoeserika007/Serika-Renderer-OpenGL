// learnOpenGL-aTriangle.h: 标准系统包含文件的包含文件
// 或项目特定的包含文件。

#pragma once
// #define GLM_FORCE_ALIGNED
#define GLM_FORCE_INLINE

#include <glm.hpp>
#include <glm/gtc/type_ptr.hpp>

using RGBA = glm::u8vec4;

#define DEBUG

struct ModelUniformBlock{
	alignas(64) glm::mat4 uModel;
	alignas(64) glm::mat4 uView;
	alignas(64) glm::mat4 uProjection;
	alignas(64) glm::mat4 uNormalToWorld;
	alignas(64) glm::mat4 uShadowMapMVP;
	alignas(16) glm::vec3 uViewPos;
};

// TODO: 在此处引用程序需要的其他标头。
