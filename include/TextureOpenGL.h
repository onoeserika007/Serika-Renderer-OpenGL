#include "Texture.h"

class TextureOpenGL:public Texture {
public:
    TextureOpenGL();
    TextureOpenGL(const TextureInfo& texInfo, const SamplerInfo& smInfo);

    virtual void setupPipeline() override;
    virtual void clearPipeline() override;
    virtual ~TextureOpenGL();
};

class TextureOpenGL2D : public TextureOpenGL {
public:
    TextureOpenGL2D();
    TextureOpenGL2D(const TextureInfo& texInfo, const SamplerInfo& smInfo);

    virtual void setupPipeline() override;
    virtual void clearPipeline() override;
    virtual ~TextureOpenGL2D();
};

class TextureOpenGLCube : public TextureOpenGL {
public:
    TextureOpenGLCube();
    TextureOpenGLCube(const TextureInfo& texInfo, const SamplerInfo& smInfo);

    virtual void setupPipeline() override;
    virtual void clearPipeline() override;
    virtual ~TextureOpenGLCube();
};