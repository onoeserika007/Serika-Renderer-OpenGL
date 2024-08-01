#include "../../include/Material/Shader.h"

void Shader::addDefines(const std::vector<std::string> &defines) {
    for (const auto& def : defines) {
        addDefine(def);
    }
}
void Shader::setResources(const std::shared_ptr<ShaderResources> &resources) {
    shaderResources_ = resources;
}

void Shader::bindResources(const std::shared_ptr<ShaderResources> &resources) {
    setResources(resources);
    bindHoldingResources();
};;

void Shader::bindHoldingResources() const {
    // useProgram, this is the only call
    use();
    if (auto&& resource = shaderResources_.lock()) {
        for (auto& [_, blocks] : resource->blocks) {
            if (blocks) {
                bindUniform(*blocks);
            }
        }

        for (auto& [_, sampler] : resource->samplers) {
            if (sampler) {
                bindUniform(*sampler);
            }
        }
    }
}

void Shader::setUniformSampler(const std::string &name, const std::shared_ptr<UniformSampler> &uniform) const {
    if (auto&& res = shaderResources_.lock()) {
        res->samplers[name] = uniform;
    }
}

void Shader::setUniformBlock(const std::string &name, const std::shared_ptr<UniformBlock> &uniform) const {
    if (auto&& res = shaderResources_.lock()) {
        res->blocks[name] = uniform;
    }
}

bool Shader::bindUniform(Uniform &uniform) const {
    int hash = uniform.getHash();
    int location = -1;
    if (!uniformLocations_.contains(hash)) { // if not contains
        location = uniform.getLocation(*this);
        uniformLocations_[hash] = location;
    }
    else {
        location = uniformLocations_[hash];
    }

    if (location < 0) {
        return false;
    }

    uniform.bindProgram(*this, location);
    return true;
}

bool Shader::ready() const {
    return pipelineSetup_;
}

void Shader::setReady(bool flag) {
    pipelineSetup_ = flag;
}

EShadingModel Shader::shadingMode() const {
    return shadingMode_;
}

void Shader::setShadingMode(EShadingModel mode) {
    shadingMode_ = mode;
}
