#pragma once
#include <memory>
#include "Texture.h"
#include <unordered_map>

class Renderer;

struct FrameBufferAttachment {
    std::shared_ptr<Texture> tex = nullptr;
    uint32_t layer = 0; // for cube map
    uint32_t level = 0;
};

class FrameBuffer {
public:
    explicit FrameBuffer(bool offscreen) : offscreen_(offscreen) {}

    virtual int getId() const = 0;
    virtual bool isValid() = 0;
    virtual void bind() const = 0;

    virtual void setColorAttachment(std::shared_ptr<Texture>& color, int level, int pos = 0) {
        auto& colorAttachment = colorAttachments_[pos];
        color->setupPipeline();
        colorAttachment.tex = color;
        colorAttachment.layer = 0;
        colorAttachment.level = level;
        colorReady_ = true;
    };

    virtual void setColorAttachment(std::shared_ptr<Texture>& color, CubeMapFace face, int level, int pos = 0) {
        auto& colorAttachment = colorAttachments_[pos];
        color->setupPipeline();
        colorAttachment.tex = color;
        colorAttachment.layer = 0;
        colorAttachment.level = level;
        colorReady_ = true;
    };

    virtual void setDepthAttachment(std::shared_ptr<Texture>& depth) {
        depth->setupPipeline();
        depthAttachment_.tex = depth;
        depthAttachment_.layer = 0;
        depthAttachment_.level = 0;
        depthReady_ = true;
    };

    inline const FrameBufferAttachment& getColorAttachment(int pos)  {
        // 查询会改变map，所以这个函数不能声明为const
        return colorAttachments_[pos];
    }

    const std::unordered_map<int, FrameBufferAttachment>& getColorAttachments() const {
        return colorAttachments_;
    }

    inline const FrameBufferAttachment& getDepthAttachment() const {
        return depthAttachment_;
    }

    inline bool isColorReady() const {
        return colorReady_;
    }

    inline bool isDepthReady() const {
        return depthReady_;
    }

    inline bool isMultiSample() const {
        if (colorReady_) {
            
        }
        if (depthReady_) {
            return getDepthAttachment().tex->multiSample();
        }

        return false;
    }

    inline bool isOffscreen() const {
        return offscreen_;
    }

    inline void setOffscreen(bool offscreen) {
        offscreen_ = offscreen;
    }

protected:
    bool offscreen_ = false;
    bool colorReady_ = false;
    bool depthReady_ = false;

    // TODO MTR
    std::unordered_map<int, FrameBufferAttachment> colorAttachments_{}; // layout->colorAttachment
    FrameBufferAttachment depthAttachment_{};
};