#include "Texture.h"

class TextureOpenGL:public Texture {
public:
    //TextureOpenGL();
    TextureOpenGL(const TextureInfo& texInfo, const SamplerInfo& smInfo);
    TextureOpenGL(const TextureInfo& texInfo, const SamplerInfo& smInfo, const TextureData& texData);

    virtual ~TextureOpenGL();
};

class TextureOpenGL2D : public TextureOpenGL {
public:
    //TextureOpenGL2D();
    TextureOpenGL2D(const TextureInfo& texInfo, const SamplerInfo& smInfo);
    TextureOpenGL2D(const TextureInfo& texInfo, const SamplerInfo& smInfo, const TextureData& texData);

    void setupPipeline();

    virtual ~TextureOpenGL2D();
};

class TextureOpenGLCube : public TextureOpenGL {
public:
    //TextureOpenGLCube();
    TextureOpenGLCube(const TextureInfo& texInfo, const SamplerInfo& smInfo);
    TextureOpenGLCube(const TextureInfo& texInfo, const SamplerInfo& smInfo, const TextureData& texData);

    virtual ~TextureOpenGLCube();
};