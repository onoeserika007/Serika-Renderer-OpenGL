#pragma once
#include <string>
#include <Utils/UUID.h>
#include <memory>
#include <unordered_map>
#include "Texture.h"

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

    virtual int getLocation(Shader& program) = 0;
    virtual void bindProgram(Shader& program, int location) = 0;

    virtual ~Uniform() = default;

    std::string name() {
        return name_;
    }

protected:
    std::string name_;
private:
    UUID<Uniform> uuid_;
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
    virtual void setTexture(const std::shared_ptr<Texture>& tex) = 0;

protected:
    TextureTarget target_;
    TextureFormat format_;
};

class ShaderResources {
public:
    std::unordered_map<int, std::shared_ptr<UniformBlock>> blocks;
    std::unordered_map<int, std::shared_ptr<UniformSampler>> samplers;
};