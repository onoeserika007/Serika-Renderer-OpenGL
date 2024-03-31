#include "FrameBuffer.h"
#include <glad/glad.h>
#include "Utils/OpenGLUtils.h"
#include "Utils/Logger.h"
#include "OpenGL/EnumsOpenGL.h"

class FrameBufferOpenGL : public FrameBuffer {
public:
    explicit FrameBufferOpenGL(bool offscreen) : FrameBuffer(offscreen) {
        GL_CHECK(glGenFramebuffers(1, &fbo_));
    }

    ~FrameBufferOpenGL() {
        GL_CHECK(glDeleteFramebuffers(1, &fbo_));
    }

    int getId() const override {
        return (int)fbo_;
    }

    bool isValid() override {
        if (!fbo_) {
            return false;
        }

        GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, fbo_));
        GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        if (status != GL_FRAMEBUFFER_COMPLETE) {
            LOGE("glCheckFramebufferStatus: %x", status);
            return false;
        }

        return true;
    }

    void setColorAttachment(std::shared_ptr<Texture>& color, int level, Renderer& renderer) override {
        if (color == colorAttachment_.tex && level == colorAttachment_.level) {
            return;
        }

        FrameBuffer::setColorAttachment(color, level, renderer);
        GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, fbo_));
        GL_CHECK(glFramebufferTexture2D(GL_FRAMEBUFFER,
            GL_COLOR_ATTACHMENT0,
            color->multiSample() ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D,
            color->getId(),
            level));
    }

    void setColorAttachment(std::shared_ptr<Texture>& color, CubeMapFace face, int level, Renderer& renderer) override {
        if (color == colorAttachment_.tex && face == colorAttachment_.layer && level == colorAttachment_.level) {
            return;
        }

        FrameBuffer::setColorAttachment(color, face, level, renderer);
        GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, fbo_));
        GL_CHECK(glFramebufferTexture2D(GL_FRAMEBUFFER,
            GL_COLOR_ATTACHMENT0,
            OpenGL::cvtCubeFace(face),
            color->getId(),
            level));
    }

    void setDepthAttachment(std::shared_ptr<Texture>& depth, Renderer& renderer) override {
        if (depth == depthAttachment_.tex) {
            return;
        }

        FrameBuffer::setDepthAttachment(depth, renderer);
        GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, fbo_));
        GL_CHECK(glFramebufferTexture2D(GL_FRAMEBUFFER,
            GL_DEPTH_ATTACHMENT,
            depth->multiSample() ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D,
            depth->getId(),
            0));
    }

    void bind() const override{
        GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, fbo_));
    }

private:
    GLuint fbo_ = 0;
};