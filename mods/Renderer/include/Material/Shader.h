#ifndef SERIKA_RENDERER_INCLUDE_MATERIAL_SHADER_H
#define SERIKA_RENDERER_INCLUDE_MATERIAL_SHADER_H
#include <string>
#include <unordered_map>
#include <memory>
#include <glad/glad.h>

#include "Uniform.h"
#include "FMaterial.h"

class FMaterial;
class Shader {
public:
    virtual ~Shader() = default;

    virtual GLuint getId() const = 0;

    virtual void addDefine(const std::string& define) = 0;
    virtual unsigned getSamplerBinding() const = 0;
    virtual unsigned getUniformBlockBinding() const = 0;
    virtual void setupPipeline(FMaterial& material) = 0;
    virtual int getAttributeLocation(const std::string& name) const = 0;;
    virtual std::shared_ptr<Shader> clone() const = 0;

    void addDefines(const std::vector<std::string>& defines);

    // This is the initial call between uniform and shader.
    // and this will definitely not reset binding counter!!
    // counter reset should be managed by yourself.

    // setters
    void setResources(const std::shared_ptr<ShaderResources> &resources);
    void bindResources(const std::shared_ptr<ShaderResources> &resources);
    void bindHoldingResources() const;
    void setUniformSampler(const std::string& name, const std::shared_ptr<UniformSampler>& uniform) const;
    void setUniformBlock(const std::string& name, const std::shared_ptr<UniformBlock> &uniform) const;

    // getters
    bool ready() const;
    void setReady(bool flag);
    EShadingModel shadingMode() const;

    void setShadingMode(EShadingModel mode);

protected:
    mutable std::unordered_map<int, int> uniformLocations_;
    bool pipelineSetup_ = false;
    EShadingModel shadingMode_ = Shading_BaseColor;

    virtual void use() const = 0;

private:
    bool bindUniform(Uniform& uniform) const;;

    std::weak_ptr<ShaderResources> shaderResources_;
};

#endif // SERIKA_RENDERER_INCLUDE_MATERIAL_SHADER_H
