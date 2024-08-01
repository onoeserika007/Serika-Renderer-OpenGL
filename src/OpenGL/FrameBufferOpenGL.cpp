#include "../include/OpenGL/FrameBufferOpenGL.h"
#include "Utils/Logger.h"
#include "Utils/OpenGLUtils.h"

FrameBufferOpenGL::FrameBufferOpenGL(bool offscreen) : FrameBuffer(offscreen) {
    if (offscreen) {
        GL_CHECK(glGenFramebuffers(1, &fbo_));
    }
    else {
        fbo_ = 0;
    }
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
        throw std::exception();
        return false;
    }
    GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, 0));

    return true;
}

void FrameBufferOpenGL::setColorAttachment(std::shared_ptr<Texture>& color, int level, int pos) {
    //if (color == colorAttachment_.tex && level == colorAttachment_.level) {
    //    return;
    //}
    if (!color) return;

    FrameBuffer::setColorAttachment(color, level, pos);
    GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, fbo_));
    if (offscreen_) {
        GL_CHECK(glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + pos, color->getId(), level));
        GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, 0));
    }
    //isValid();
}

void FrameBufferOpenGL::setColorAttachment(std::shared_ptr<Texture>& color, ECubeMapFace face, int level, int pos) {
    //if (color == colorAttachment_.tex && face == colorAttachment_.layer && level == colorAttachment_.level) {
    //    return;
    //}
    if (!color) return;

    FrameBuffer::setColorAttachment(color, face, level, pos);
    GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, fbo_));
    if (offscreen_) {
        GL_CHECK(glFramebufferTexture2D(GL_FRAMEBUFFER,
        GL_COLOR_ATTACHMENT0 + pos,
        OpenGL::cvtCubeFace(face),
        color->getId(),
        level));
        GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, 0));
    }
    //isValid();
};

void FrameBufferOpenGL::setDepthAttachment(std::shared_ptr<Texture>& depth) {
    // TODO: write and read state can be set here
    if (depth == depthAttachment_.tex) {
        return;
    }

    FrameBuffer::setDepthAttachment(depth);

    auto&& texInfo = depth->getTextureInfo();
    const GLuint target = OpenGL::cvtTextureTarget(static_cast<TextureTarget>(texInfo.target)); // target is not used
    GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, fbo_));
    if (offscreen_) {
        if (target == GL_TEXTURE_CUBE_MAP) {
            // unbind color buffer
            GL_CHECK(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, 0, 0));
        }
        GL_CHECK(glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depth->getId(), 0));
        GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, 0));
    }
    //isValid();
}

void FrameBufferOpenGL::bind() const {
    GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, fbo_));
    // multi attachment only for offscreen
    if (offscreen_) {
        std::vector<GLenum> DrawBuffers;
        for (auto& [pos, colorAttach] : colorAttachments_) {
            DrawBuffers.push_back(GL_COLOR_ATTACHMENT0 + pos);
        }
        GL_CHECK(glDrawBuffers(static_cast<int>(DrawBuffers.size()), DrawBuffers.data()));
        GLenum Status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        if (Status != GL_FRAMEBUFFER_COMPLETE) {
            LOGE("FB error, status: 0x%x\n", Status);
            // throw std::exception{};
        }
    }
}

// stupid mistake, 居然把这俩写反了
void FrameBufferOpenGL::bindForReading() const {
    GL_CHECK(glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo_));
}

void FrameBufferOpenGL::bindForWriting() const {
    GL_CHECK(glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo_));
}

void FrameBufferOpenGL::disableForColorWriting() const {
    GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, fbo_));
    GL_CHECK(glDrawBuffer(GL_NONE));
}

void FrameBufferOpenGL::disableForColorReading() const {
    GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, fbo_));
    GL_CHECK(glReadBuffer(GL_NONE));
}

void FrameBufferOpenGL::unbindAllColorAttachments() const {
    GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, fbo_));
    for (auto&& [pos, attachment]: colorAttachments_) {
        GL_CHECK(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + pos, GL_TEXTURE_2D, 0, 0));
    }
    colorAttachments_.clear();
    disableForColorWriting();
    disableForColorReading();
}

void FrameBufferOpenGL::setReadBuffer(int colorAttachmentType) {
    GL_CHECK(glReadBuffer(GL_COLOR_ATTACHMENT0 + colorAttachmentType));
}

void FrameBufferOpenGL::setWriteBuffer(int colorAttachmentType, bool bClear) {
    // setForWriting means clear
    GL_CHECK(glDrawBuffer(GL_COLOR_ATTACHMENT0 + colorAttachmentType));
    if (bClear) {
        GL_CHECK(glClear(GL_COLOR_BUFFER_BIT));
    }
}

void FrameBufferOpenGL::clearDepthBuffer() {
    GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, fbo_));
    GL_CHECK(glClear(GL_DEPTH_BUFFER_BIT));
}
