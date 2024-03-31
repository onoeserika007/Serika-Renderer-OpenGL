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

    virtual void use() = 0;

    virtual unsigned getSamplerBinding() = 0;

    virtual unsigned getUniformBlockBinding() = 0;

    virtual void setupPipeline(Material& material) = 0;

    //virtual void addDefines(const std::set<std::string>& defs) {
    //    for (auto& str : defs) {
    //        addDefine(str);
    //    }
    //}

    //virtual void bindResources(ShaderResources& resources) {
    //    for (auto& kv : resources.blocks) {
    //        bindUniform(*kv.second);
    //    }

    //    for (auto& kv : resources.samplers) {
    //        bindUniform(*kv.second);
    //    }
    //}

    virtual bool bindUniform(Uniform& uniform) {
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
    };

    virtual int getAttributeLocation(const std::string& name) const = 0;;

    virtual std::shared_ptr<Shader> clone() const = 0;

    bool ready() {
        return pipelineSetup_;
    }

    void setReady(bool flag) {
        pipelineSetup_ = flag;
    }

    ShadingMode shadingMode() {
        return shadingMode_;
    }

    void setShadingMode(ShadingMode mode) {
        shadingMode_ = mode;
    }

protected:
    std::unordered_map<int, int> uniformLocations_;
    bool pipelineSetup_;
    ShadingMode shadingMode_;
};