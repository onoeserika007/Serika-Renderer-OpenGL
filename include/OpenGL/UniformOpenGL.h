#include "../Uniform.h"
#include "../../ThirdParty/glad/include/glad/glad.h"

class Texture;
class Shader;
class UniformSamplerOpenGL : public UniformSampler {
private:
	unsigned textureId_ = 0;
public:
	UniformSamplerOpenGL(const std::string& name, TextureTarget target, TextureFormat format) 
        : UniformSampler(name, target, format) {}
    UniformSamplerOpenGL(const TextureInfo& info): UniformSampler(info) {}
	int getLocation(const Shader &program) override;
	virtual void bindProgram(const Shader &program, int location) const override;
	virtual void setTexture(const Texture &tex) override;
};

class UniformBlockOpenGL : public UniformBlock {
public:
    UniformBlockOpenGL(const std::string& name, int size);

    virtual ~UniformBlockOpenGL();

    int getLocation(const Shader &program) override;

    void bindProgram(const Shader &program, int location) const override;

    void setSubData(void* data, int len, int offset) override;

    void setData(void* data, int len) override;

private:
    GLuint ubo_ = 0;
};