// learnOpenGL-aTriangle.h: 标准系统包含文件的包含文件
// 或项目特定的包含文件。

#pragma once
#define GLM_FORCE_ALIGNED_GENTYPES
#define GLM_FORCE_ALIGNED
#define GLM_FORCE_INLINE
#define GLM_FORCE_AVX2
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_aligned.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <stdint.h>

using RGBA = glm::u8vec4;

#define DEBUG

struct ModelUniformBlock{
	glm::mat4 uModel;
	glm::mat4 uView;
	glm::mat4 uProjection;
	glm::mat4 uNormalToWorld;
	glm::mat4 uShadowMapMVP;
	glm::vec3 uViewPos;
};

// TODO: 在此处引用程序需要的其他标头。
