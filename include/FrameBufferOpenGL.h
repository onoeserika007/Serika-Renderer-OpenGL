#include "FrameBuffer.h"
#include <glad/glad.h>
#include "Utils/OpenGLUtils.h"
#include "Utils/Logger.h"
#include "OpenGL/EnumsOpenGL.h"
#include <vector>


class FrameBufferOpenGL : public FrameBuffer {
public:
    explicit FrameBufferOpenGL(bool offscreen);

    ~FrameBufferOpenGL();

    virtual int getId() const override;
    virtual bool isValid() const override;
    virtual void bind() const override;
    virtual void bindForReading() const override;
    virtual void bindForWriting() const override;
    virtual void disableForColorWriting() const override;
    virtual void diableForColorReading() const override;
    virtual void setReadBuffer(int colorAttachmentType) override;
    virtual void setWriteBuffer(int colorAttachmentType, bool bClear) override;
    virtual void clearDepthBuffer() override;

    void setColorAttachment(std::shared_ptr<Texture>& color, int level, int pos) override;

    void setColorAttachment(std::shared_ptr<Texture>& color, CubeMapFace face, int level, int pos) override;

    void setDepthAttachment(std::shared_ptr<Texture>& depth) override;

private:
    GLuint fbo_ = 0;
};