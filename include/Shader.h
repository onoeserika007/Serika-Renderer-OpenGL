#pragma once 
#include <string>
#include <unordered_map>
#include <memory>
#include "Uniform.h"
#include "Material.h"

class Material;
class Shader {
public:
    virtual int getId() const = 0;

    virtual void addDefine(const std::string& def) = 0;
    virtual unsigned getSamplerBinding() const = 0;
    virtual unsigned getUniformBlockBinding() const = 0;
    virtual void setupPipeline(Material& material) = 0;
    virtual int getAttributeLocation(const std::string& name) const = 0;;
    virtual std::shared_ptr<Shader> clone() const = 0;

    // This is the initial call between uniform and shader.
    // and this will definitely not reset binding counter!!
    // counter reset should be managed by yourself.

    // setters
    void setResources(const std::shared_ptr<ShaderResources> &resources);
    void bindResources(const std::shared_ptr<ShaderResources> &resources);
    void bindHoldingResources() const;
    void setUniformSampler(const std::string& name, const std::shared_ptr<UniformSampler>& uniform);
    void setUniformBlock(const std::string& name, const std::shared_ptr<UniformBlock> &uniform);

    // getters
    bool ready();
    void setReady(bool flag);
    EShadingMode shadingMode();

    void setShadingMode(EShadingMode mode);

protected:
    mutable std::unordered_map<int, int> uniformLocations_;
    bool pipelineSetup_ = false;
    EShadingMode shadingMode_;

    virtual void use() const = 0;

private:
    bool bindUniform(Uniform& uniform) const;;

    std::weak_ptr<ShaderResources> shaderResources_;
};