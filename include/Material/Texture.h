#pragma once
#include <string>
#include <unordered_map>
#include <string>
#include <memory>
#include "Buffer.h"
#include "Base/Globals.h"

const int MAX_TEXTURE_NUM = 16;

enum TextureType {
#define TEXTURETYPE_ENUM(x) x,
#include "Base/TextureTypeEnum.def"

};

enum ECubeMapFace {
    TEXTURE_CUBE_MAP_POSITIVE_X = 0,
    TEXTURE_CUBE_MAP_NEGATIVE_X = 1,
    TEXTURE_CUBE_MAP_POSITIVE_Y = 2,
    TEXTURE_CUBE_MAP_NEGATIVE_Y = 3,
    TEXTURE_CUBE_MAP_POSITIVE_Z = 4,
    TEXTURE_CUBE_MAP_NEGATIVE_Z = 5,
};

enum BorderColor {
    Border_BLACK = 0,
    Border_WHITE,
};

struct SamplerInfo {
    FilterMode filterMin = Filter_LINEAR;
    FilterMode filterMag = Filter_LINEAR;

    WrapMode wrapS = Wrap_REPEAT;
    WrapMode wrapT = Wrap_REPEAT;
    WrapMode wrapR = Wrap_REPEAT;

    BorderColor borderColor = Border_BLACK;
};

enum TextureTarget{
    TextureTarget_TEXTURE_2D,
    TextureTarget_TEXTURE_2D_MULTISAMPLE,
    TextureTarget_TEXTURE_CUBE_MAP,
    TextureTarget_TEXTURE_BUFFER
};

enum TextureFormat {
    TextureFormat_RGBA8,     // RGBA8888
    TextureFormat_RGB8,
    TextureFormat_RGB16F,
    TextureFormat_RGBA16F,
    TextureFormat_RGB32F,
    TextureFormat_RGBA32F,
    TextureFormat_FLOAT32,    // Float32
    TextureFormat_R16F
};

enum TextureUsage {
    TextureUsage_Sampler = 1 << 0,
    TextureUsage_UploadData = 1 << 1,
    TextureUsage_AttachmentColor = 1 << 2,
    TextureUsage_AttachmentDepth = 1 << 3,
    TextureUsage_RendererOutput = 1 << 4,
};

struct TextureData {
    std::vector<std::shared_ptr<Buffer<RGBA>>> unitDataArray;
    std::vector<std::shared_ptr<Buffer<glm::vec3>>> floatDataArray;
    std::vector<float> bufferData;
    std::string path;
    TextureType loadedTextureType = TEXTURE_TYPE_NONE;
};

struct TextureInfo {
    // textureInfo
    int width = 0; 
    int height = 0;
    TextureType type = TEXTURE_TYPE_NONE;
    TextureTarget target = TextureTarget_TEXTURE_2D;
    TextureFormat format = TextureFormat_RGBA8;
    uint32_t usage = TextureUsage_Sampler;
    uint32_t border = 0; // border width
    bool useMipmaps = false;
    bool multiSample = false;
};

class Renderer;
class Shader;
class UniformSampler;
class Texture {
protected:
    unsigned textureId_;
    // name_ is trouble from a texture
    //std::string name_;
    bool pipelineReady_ = false;
    mutable std::shared_ptr<UniformSampler> sampler_;

    // texture description
    TextureInfo textureInfo_;
    SamplerInfo samplerInfo_;
    TextureData textureData_;

public:
    // static std::shared_ptr<Texture> createTexture2DDefault(int width, int height, TextureFormat format, TextureUsage usage);
    static const char* materialTexTypeStr(TextureType usage);
    static const char* samplerDefine(TextureType type);
    static const char* samplerName(TextureType usage);
    virtual ~Texture();

    //Texture();
    Texture(TextureType type);
    Texture(const TextureInfo& info);
    Texture(const TextureInfo& texInfo, const SamplerInfo& smInfo, const TextureData& texData);

    void loadTextureData(const std::string& picture);
    void loadTextureData(TextureData data);

    //void setName(const std::string& name);
    void setTextureInfo(const TextureInfo& info);
    const TextureInfo& getTextureInfo() const;
    void setSamplerInfo(const SamplerInfo& info);
    const SamplerInfo& getSamplerInfo() const;
    //std::string getName();
	TextureType getType() const;
    const TextureData& getTextureData() const;
    int width() const;
    int height() const;
    bool useMipmaps() const;
    bool multiSample() const;
    bool ready() const;
    void setReady(bool flag);

    unsigned getId() const;
    void setId(unsigned id);
    void setMipmaps(bool flag);
    void setMultiSample(bool flag);

    virtual void copyDataTo(Texture& other);
    virtual std::shared_ptr<UniformSampler> getUniformSampler(const Renderer &renderer) const = 0;
};


// 如果将静态成员变量的初始化放在头文件中，每个包含该头文件的源文件都会有一份独立的静态成员变量，导致重定义错误。
// 将初始化放在一个源文件中可以确保只有一个初始化，而其他源文件只是包含了声明，不会导致重复定义。