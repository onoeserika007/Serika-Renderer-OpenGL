#include "OpenGL/UniformOpenGL.h"
#include "Material/Texture.h"
#include "Material/Shader.h"
#include "Utils/OpenGLUtils.h"
#include "OpenGL/EnumsOpenGL.h"

int UniformSamplerOpenGL::getLocation(const Shader &program) {
	return glGetUniformLocation(program.getId(), name_.c_str());
}

// the location is from getLocation func above
void UniformSamplerOpenGL::bindProgram(const Shader &program, int location) const {
	// call this will reset binding counter, do not use it.
	// program.use();
	// reseting should be ensured before call bindProgram
	auto binding = program.getSamplerBinding();
	GL_CHECK(glActiveTexture(GL_TEXTURE0 + binding));
	GL_CHECK(glBindTexture(texTarget_, textureId_));
	GL_CHECK(glUniform1i(location, binding));
}

void UniformSamplerOpenGL::setTexture(const Texture &tex) {
	auto texInfo = tex.getTextureInfo();
	texTarget_ = OpenGL::cvtTextureTarget(static_cast<TextureTarget>(texInfo.target));
	textureId_ = tex.getId();
}

UniformBlockOpenGL::UniformBlockOpenGL(const std::string& name, int size) : UniformBlock(name, size) {
	GL_CHECK(glGenBuffers(1, &ubo_));
	GL_CHECK(glBindBuffer(GL_UNIFORM_BUFFER, ubo_));
	GL_CHECK(glBufferData(GL_UNIFORM_BUFFER, size, nullptr, GL_STATIC_DRAW));
}

UniformBlockOpenGL::~UniformBlockOpenGL() {
	// static局部变量 有个生命周期的问题，当viewer析构掉后，static仍然会持有uniform，这就造成了问题
	// std::cout << "UniformOpenGL here!" << std::endl;
	GL_CHECK(glDeleteBuffers(1, &ubo_));
}

// location是从shader中计算出来的，这和block binding无关
int UniformBlockOpenGL::getLocation(const Shader &program) {
	return glGetUniformBlockIndex(program.getId(), name_.c_str());
}

// binding Point只用作通信用，它是0还是100无关紧要，当数据传输到gpu后它的使命就完成了
void UniformBlockOpenGL::bindProgram(const Shader &program, int location) const {
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
