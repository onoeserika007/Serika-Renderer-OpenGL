#pragma once
#include <string>
#include <unordered_map>
#include <string>
#include <memory>
#include "Buffer.h"
#include "Base/GLMInc.h"

const int MAX_TEXTURE_NUM = 16;

enum TextureType {
    TEXTURE_NONE,
    TEXTURE_DIFFUSE,
    TEXTURE_SPECULAR,
    TEXTURE_AMBIENT,
    TEXTURE_EMISSIVE,
    TEXTURE_HEIGHT,
    TEXTURE_NORMALS,
    TEXTURE_SHININESS,
    TEXTURE_OPACITY,
    TEXTURE_DISPLACEMENT,
    TEXTURE_LIGHTMAP,
    TEXTURE_REFLECTION,
    TEXTURE_BASE_COLOR,
    TEXTURE_NORMAL_CAMERA,
    TEXTURE_EMISSION_COLOR,
    TEXTURE_METALNESS,
    TEXTURE_DIFFUSE_ROUGHNESS,
    TEXTURE_AMBIENT_OCCLUSION,
    TEXTURE_UNKNOWN,
    TEXTURE_SHEEN,
    TEXTURE_CLEARCOAT,
    TEXTURE_TRANSMISSION,
    TEXTURE_SHADOW,
};

enum CubeMapFace {
    TEXTURE_CUBE_MAP_POSITIVE_X = 0,
    TEXTURE_CUBE_MAP_NEGATIVE_X = 1,
    TEXTURE_CUBE_MAP_POSITIVE_Y = 2,
    TEXTURE_CUBE_MAP_NEGATIVE_Y = 3,
    TEXTURE_CUBE_MAP_POSITIVE_Z = 4,
    TEXTURE_CUBE_MAP_NEGATIVE_Z = 5,
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

enum CubeMapFace {
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

    WrapMode wrapS = Wrap_CLAMP_TO_EDGE;
    WrapMode wrapT = Wrap_CLAMP_TO_EDGE;
    WrapMode wrapR = Wrap_CLAMP_TO_EDGE;

    BorderColor borderColor = Border_BLACK;
};

enum TextureTarget{
    TextureTarget_2D,
    TextureTarget_CUBE,
};

enum TextureFormat {
    TextureFormat_RGBA8 = 0,      // RGBA8888
    TextureFormat_FLOAT32 = 1,    // Float32
};

enum TextureUsage {
    TextureUsage_Sampler = 1 << 0,
    TextureUsage_UploadData = 1 << 1,
    TextureUsage_AttachmentColor = 1 << 2,
    TextureUsage_AttachmentDepth = 1 << 3,
    TextureUsage_RendererOutput = 1 << 4,
};

struct TextureData {
    std::vector<std::shared_ptr<Buffer<RGBA>>> dataArray;
    std::string path;
};

struct TextureInfo {
    int width = 0; 
    int height = 0;
    uint32_t type = TEXTURE_NONE;
    uint32_t target = TextureTarget_2D;
    uint32_t format = TextureFormat_RGBA8;
    uint32_t usage = TextureUsage_Sampler;
    bool useMipmaps = false;
    bool multiSample = false;
};

class Renderer;
class Shader;
class UniformSampler;
class Texture {

	unsigned textureId_;
    std::string name_;
    bool pipelineReady_ = false;

    // texture description
    TextureInfo textureInfo_;
    SamplerInfo samplerInfo_;
    std::shared_ptr<TextureData> prawData;

public:
    static std::shared_ptr<Texture> createTexture2DDefault(int width, int height, TextureFormat format, TextureUsage usage);
    static const char* materialTexTypeStr(TextureType usage);
    static const char* samplerDefine(TextureType usage);
    static const char* samplerName(TextureType usage);
    Texture();
    Texture(TextureType type);
    Texture(const TextureInfo& info);
    Texture(const TextureInfo& texInfo, const SamplerInfo& smInfo);
    void loadTextureData(const std::string& picture);
    void loadTextureData(TextureData data);
    void setName(const std::string& name);
    void setTextureInfo(const TextureInfo& info);
    const TextureInfo& getTextureInfo();
    void setSamplerInfo(const SamplerInfo& info);
    const SamplerInfo& getSamplerInfo();
    std::string getName();
	TextureType getType();
    std::shared_ptr<TextureData> getpRawData();
    int width();
    int height();
    bool useMipmaps();
    bool multiSample();
    bool ready();

    unsigned getId();
    void setId(unsigned id);
    void setMipmaps(bool flag);
    void setMultiSample(bool flag);
    void setupPipeline(Renderer& renderer);
    void clearPipeline(Renderer& renderer);
};


// 如果将静态成员变量的初始化放在头文件中，每个包含该头文件的源文件都会有一份独立的静态成员变量，导致重定义错误。
// 将初始化放在一个源文件中可以确保只有一个初始化，而其他源文件只是包含了声明，不会导致重复定义。