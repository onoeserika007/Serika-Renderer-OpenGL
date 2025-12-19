#ifndef SERIKA_RENDERER_INCLUDE_OPENGL_TEXTUREOPENGL_H
#define SERIKA_RENDERER_INCLUDE_OPENGL_TEXTUREOPENGL_H
#include "Material/Texture.h"
#include <glad/glad.h>

class TextureOpenGL:public Texture {
public:
    //TextureOpenGL();
    TextureOpenGL(const TextureInfo& texInfo, const SamplerInfo& smInfo, const TextureData& texData);

    virtual ~TextureOpenGL();
    virtual std::shared_ptr<UniformSampler> getUniformSampler(const Renderer &renderer) const override;
};

class TextureOpenGL2D : public TextureOpenGL {
public:
    //TextureOpenGL2D();
    TextureOpenGL2D(const TextureInfo& texInfo, const SamplerInfo& smInfo, const TextureData& texData);

    void setupPipeline();
    virtual void copyDataTo(Texture& other);

    virtual ~TextureOpenGL2D();
};

class TextureOpenGLCube : public TextureOpenGL {
public:

    TextureOpenGLCube(const TextureInfo& texInfo, const SamplerInfo& smInfo, const TextureData& texData);

    void setupPipeline();

    ~TextureOpenGLCube() override;
};

class TextureOpenGLBuffer: public TextureOpenGL {
public:
    TextureOpenGLBuffer(const TextureInfo& texInfo, const SamplerInfo& smInfo, const TextureData& texData);

    void setupPipeline();

    ~TextureOpenGLBuffer() override;

private:
    GLuint TBO;
};

#endif // SERIKA_RENDERER_INCLUDE_OPENGL_TEXTUREOPENGL_H
