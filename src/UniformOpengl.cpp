#include "UniformOpenGL.h"
#include "Texture.h"
#include "Shader.h"
#include "Utils/OpenGLUtils.h"

int UniformSamplerOpenGL::getLocation(Shader& program) {
	return glGetUniformLocation(program.getId(), name_.c_str());
}

void UniformSamplerOpenGL::bindProgram(Shader& program, int location) {
	program.use();
	auto binding = program.getSamplerBinding();
	GL_CHECK(glActiveTexture(GL_TEXTURE0 + binding));
	GL_CHECK(glBindTexture(GL_TEXTURE_2D, textureId_));
	GL_CHECK(glUniform1i(location, binding));
}

void UniformSamplerOpenGL::setTexture(const std::shared_ptr<Texture>& tex) {
	textureId_ = tex->getId();
}

UniformBlockOpenGL::UniformBlockOpenGL(const std::string& name, int size) : UniformBlock(name, size) {
	GL_CHECK(glGenBuffers(1, &ubo_));
	GL_CHECK(glBindBuffer(GL_UNIFORM_BUFFER, ubo_));
	GL_CHECK(glBufferData(GL_UNIFORM_BUFFER, size, nullptr, GL_STATIC_DRAW));
}

UniformBlockOpenGL::~UniformBlockOpenGL() {
	std::cout << "UniformOpenGL here!" << std::endl;
	GL_CHECK(glDeleteBuffers(1, &ubo_));
}

int UniformBlockOpenGL::getLocation(Shader& program) {
	return glGetUniformBlockIndex(program.getId(), name_.c_str());
}

void UniformBlockOpenGL::bindProgram(Shader& program, int location) {
	if (location < 0) {
		return;
	}
	int binding = program.getUniformBlockBinding();

	GL_CHECK(glUniformBlockBinding(program.getId(), location, binding)); // 这一行对shader添加binding信息
	GL_CHECK(glBindBufferBase(GL_UNIFORM_BUFFER, binding, ubo_)); // 这一行建立UniformBuffer到bindingPoint的映射，两者在一次定义，所以肯定能对应上，只是每次渲染都要重新绑定
}

void UniformBlockOpenGL::setSubData(void* data, int len, int offset) {
	GL_CHECK(glBindBuffer(GL_UNIFORM_BUFFER, ubo_));
	GL_CHECK(glBufferSubData(GL_UNIFORM_BUFFER, offset, len, data));
}

void UniformBlockOpenGL::setData(void* data, int len) {
	GL_CHECK(glBindBuffer(GL_UNIFORM_BUFFER, ubo_));
	GL_CHECK(glBufferData(GL_UNIFORM_BUFFER, len, data, GL_STATIC_DRAW));
}
