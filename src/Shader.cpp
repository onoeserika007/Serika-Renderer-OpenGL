#include "Shader.h"

void Shader::setResources(const std::shared_ptr<ShaderResources> &resources) {
    shaderResources_ = resources;
}

void Shader::bindResources(const std::shared_ptr<ShaderResources> &resources) {
    setResources(resources);
    bindHoldingResources();
}

void Shader::bindHoldingResources() const {
    // useProgram, this is the only call
    use();
    if (auto&& resource = shaderResources_.lock()) {
        for (auto& [_, sampler] : resource->blocks) {
            bindUniform(*sampler);
        }

        for (auto& [_, sampler] : resource->samplers) {
            bindUniform(*sampler);
        }
    }
}

void Shader::setUniformSampler(const std::string &name, const std::shared_ptr<UniformSampler> &uniform) {
    if (auto&& res = shaderResources_.lock()) {
        res->samplers[name] = uniform;
    }
}

void Shader::setUniformBlock(const std::string &name, const std::shared_ptr<UniformBlock> &uniform) {
    if (auto&& res = shaderResources_.lock()) {
        res->blocks[name] = uniform;
    }
}

bool Shader::bindUniform(Uniform &uniform) const {
    int hash = uniform.getHash();
    int location = -1;
    if (uniformLocations_.find(hash) == uniformLocations_.end()) {
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

bool Shader::ready() {
    return pipelineSetup_;
}

void Shader::setReady(bool flag) {
    pipelineSetup_ = flag;
}

EShadingMode Shader::shadingMode() {
    return shadingMode_;
}

void Shader::setShadingMode(EShadingMode mode) {
    shadingMode_ = mode;
}
