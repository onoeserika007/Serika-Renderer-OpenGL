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
    virtual bool isValid() const = 0;
    virtual void bind() const = 0;
    virtual void bindForReading() const = 0;
    virtual void bindForWriting() const = 0;
    virtual void disableForColorWriting() const = 0;
    virtual void diableForColorReading() const = 0;
    virtual void setReadBuffer(int colorAttachmentType) = 0;
    virtual void setWriteBuffer(int colorAttachmentType, bool bClear) = 0;
    virtual void clearDepthBuffer() = 0;

    virtual void setColorAttachment(std::shared_ptr<Texture>& color, int level, int pos);

    virtual void setColorAttachment(std::shared_ptr<Texture>& color, CubeMapFace face, int level, int pos);

    virtual void setDepthAttachment(std::shared_ptr<Texture>& depth);;

    inline const FrameBufferAttachment& getColorAttachment(int pos);

    const std::unordered_map<int, FrameBufferAttachment>& getColorAttachments() const;

    inline const FrameBufferAttachment& getDepthAttachment() const;

    inline bool isColorReady() const;

    inline bool isDepthReady() const;

    inline bool isMultiSample() const;

    inline bool isOffscreen() const;

    inline void setOffscreen(bool offscreen);

protected:
    bool offscreen_ = false;
    bool colorReady_ = false;
    bool depthReady_ = false;

    // TODO MTR
    std::unordered_map<int, FrameBufferAttachment> colorAttachments_{}; // layout->colorAttachment
    FrameBufferAttachment depthAttachment_{};
};