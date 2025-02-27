#pragma once
#include <string>
#include <memory>
#include <unordered_map>
#include "Utils/UUID.h"
#include "Texture.h"
#include "FrameBuffer.h"
#include "Utils/SRKLogger.h"

enum UniformBlockType {
    UniformBlock_Scene,
    UniformBlock_Model,
    UniformBlock_Material,
    UniformBlock_QuadFilter,
    UniformBlock_IBLPrefilter,
};

class Shader;

class Uniform {
public:
    explicit Uniform(std::string name) : name_(std::move(name)) {}

    inline int getHash() const {
        return uuid_.get();
    }

    virtual int getLocation(const Shader &program) = 0;
    virtual void bindProgram(const Shader &program, int location) const = 0;

    virtual ~Uniform() = default;

    std::string name() {
        return name_;
    }

    void setName(const std::string& name) {
        name_ = name;
    }

protected:
    std::string name_;
private:
    Serika::UUID<Uniform> uuid_;
};

class UniformBlock : public Uniform {
public:
    UniformBlock(const std::string& name, int size) : Uniform(name), blockSize(size) {}

    virtual void setSubData(void* data, int len, int offset) = 0;
    virtual void setData(void* data, int len) = 0;

    virtual ~UniformBlock();

protected:
    int blockSize;
};

class UniformSampler : public Uniform {
public:
    //UniformSampler(const std::string& name, TextureType type, TextureFormat format)
    //    : Uniform(name), type(type), format(format) {}
    UniformSampler(const std::string& name, TextureTarget target, TextureFormat format)
        : Uniform(name), target_(target), format_(format) {}

    UniformSampler(const TextureInfo& info)
        : Uniform(Texture::samplerName(static_cast<TextureType>(info.type))),
        target_(static_cast<TextureTarget>(info.target)), 
        format_(static_cast<TextureFormat>(info.format)) {}

    UniformSampler(const FrameBufferAttachment& attachment) 
        : UniformSampler(attachment.tex? attachment.tex->getTextureInfo(): TextureInfo())
    {
        if (!attachment.tex) {
            LOGE("Uniform sampler initialized from attachment with no texure!");
        }
    }

    virtual void setTexture(const Texture &tex) = 0;

protected:
    TextureTarget target_;
    TextureFormat format_;
};

class ShaderResources {
public:
    std::unordered_map<std::string, std::shared_ptr<UniformBlock>> blocks;
    std::unordered_map<std::string, std::shared_ptr<UniformSampler>> samplers;
};