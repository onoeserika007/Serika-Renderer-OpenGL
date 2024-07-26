#pragma once
#include <memory>
#include "Material/Texture.h"
#include <unordered_map>

class Renderer;

struct FrameBufferAttachment {
    std::shared_ptr<Texture> tex = nullptr;
    uint32_t layer = 0; // for cube map
    uint32_t level = 0;
};

class FrameBuffer {
public:
    virtual ~FrameBuffer() = default;

    explicit FrameBuffer(const bool offscreen) : offscreen_(offscreen) {}

    virtual int getId() const = 0;
    virtual bool isValid() const = 0;
    virtual void bind() const = 0;
    virtual void bindForReading() const = 0;
    virtual void bindForWriting() const = 0;
    virtual void disableForColorWriting() const = 0;
    virtual void disableForColorReading() const = 0;
    virtual void unbindAllColorAttachments() const = 0;
    virtual void setReadBuffer(int colorAttachmentType) = 0;
    virtual void setWriteBuffer(int colorAttachmentType, bool bClear) = 0;
    virtual void clearDepthBuffer() = 0;

    virtual void setColorAttachment(std::shared_ptr<Texture>& color, int level, int pos);
    virtual void setColorAttachment(std::shared_ptr<Texture>& color, ECubeMapFace face, int level, int pos);
    virtual void setDepthAttachment(std::shared_ptr<Texture>& depth);;

    inline const FrameBufferAttachment& getColorAttachment(int pos) const;

    const std::unordered_map<int, FrameBufferAttachment>& getColorAttachments() const;

    const FrameBufferAttachment& getDepthAttachment() const;

    bool isColorReady() const;

    bool isDepthReady() const;

    bool isMultiSample() const;

    bool isOffscreen() const;

    inline void setOffscreen(bool offscreen);

protected:
    bool offscreen_ = false;
    bool colorReady_ = false;
    bool depthReady_ = false;

    // TODO MTR
    mutable std::unordered_map<int, FrameBufferAttachment> colorAttachments_{}; // layout->colorAttachment
    FrameBufferAttachment depthAttachment_{};
};