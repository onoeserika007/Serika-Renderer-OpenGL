#pragma once
#include "FrameBuffer.h"
#include <glad/glad.h>
#include "Utils/SRKLogger.h"
#include "EnumsOpenGL.h"


class FrameBufferOpenGL final : public FrameBuffer {
public:
    explicit FrameBufferOpenGL(bool offscreen);

    ~FrameBufferOpenGL() override;

    virtual int getId() const override;
    virtual bool isValid() const override;
    virtual void bind() const override;
    virtual void bindForReading() const override;
    virtual void bindForWriting() const override;
    virtual void disableForColorWriting() const override;
    virtual void disableForColorReading() const override;
    virtual void unbindAllColorAttachments() const override;
    virtual void setReadBuffer(int colorAttachmentType) override;
    virtual void setWriteBuffer(int colorAttachmentType, bool bClear) override;
    virtual void clearDepthBuffer() override;

    void setColorAttachment(std::shared_ptr<Texture>& color, int level, int pos) override;

    void setColorAttachment(std::shared_ptr<Texture>& color, ECubeMapFace face, int level, int pos) override;

    void setDepthAttachment(std::shared_ptr<Texture>& depth) override;

private:
    GLuint fbo_ = 0;
};