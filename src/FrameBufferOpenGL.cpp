#include "FrameBufferOpenGL.h"
#include "Utils/Logger.h"

FrameBufferOpenGL::FrameBufferOpenGL(bool offscreen) : FrameBuffer(offscreen) {
    GL_CHECK(glGenFramebuffers(1, &fbo_));
}

FrameBufferOpenGL::~FrameBufferOpenGL() {
    GL_CHECK(glDeleteFramebuffers(1, &fbo_));
}

int FrameBufferOpenGL::getId() const {
    return (int)fbo_;
}

bool FrameBufferOpenGL::isValid() const {
    if (!fbo_) {
        LOGE("fbo is zero!");
        return false;
    }

    GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, fbo_));
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        LOGE("glCheckFramebufferStatus: %x", status);
        return false;
    }
    GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, 0));

    return true;
}

void FrameBufferOpenGL::setColorAttachment(std::shared_ptr<Texture>& color, int level, int pos) {
    //if (color == colorAttachment_.tex && level == colorAttachment_.level) {
    //    return;
    //}

    FrameBuffer::setColorAttachment(color, level);
    GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, fbo_));
    GL_CHECK(glFramebufferTexture2D(GL_FRAMEBUFFER,
        GL_COLOR_ATTACHMENT0 + pos,
        color->multiSample() ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D,
        color->getId(),
        level));
    GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, 0));
    //isValid();
}

void FrameBufferOpenGL::setColorAttachment(std::shared_ptr<Texture>& color, CubeMapFace face, int level, int pos) {
    //if (color == colorAttachment_.tex && face == colorAttachment_.layer && level == colorAttachment_.level) {
    //    return;
    //}

    FrameBuffer::setColorAttachment(color, face, level);
    GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, fbo_));
    GL_CHECK(glFramebufferTexture2D(GL_FRAMEBUFFER,
        GL_COLOR_ATTACHMENT0 + pos,
        OpenGL::cvtCubeFace(face),
        color->getId(),
        level));
    GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, 0));
    //isValid();
};

void FrameBufferOpenGL::setDepthAttachment(std::shared_ptr<Texture>& depth) {
    if (depth == depthAttachment_.tex) {
        return;
    }

    FrameBuffer::setDepthAttachment(depth);
    GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, fbo_));
    GL_CHECK(glFramebufferTexture2D(GL_FRAMEBUFFER,
        GL_DEPTH_ATTACHMENT,
        depth->multiSample() ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D,
        depth->getId(),
        0));
    GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, 0));
    //isValid();
}

void FrameBufferOpenGL::bind() const {
    GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, fbo_));
    std::vector<GLenum> DrawBuffers;
    for (auto& [pos, colorAttach] : colorAttachments_) {
        DrawBuffers.push_back(GL_COLOR_ATTACHMENT0 + pos);
    }
    glDrawBuffers(DrawBuffers.size(), DrawBuffers.data());
    GLenum Status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (Status != GL_FRAMEBUFFER_COMPLETE) {
        LOGE("FB error, status: 0x%x\n", Status);
    }
}
