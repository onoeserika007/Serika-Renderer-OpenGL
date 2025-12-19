#include "FrameBuffer.h"

void FrameBuffer::setColorAttachment(std::shared_ptr<Texture>& color, int level, int pos) {
    auto& colorAttachment = colorAttachments_[pos];
    colorAttachment.tex = color;
    colorAttachment.layer = 0;
    colorAttachment.level = level;
    colorReady_ = true;
}

void FrameBuffer::setColorAttachment(std::shared_ptr<Texture>& color, ECubeMapFace face, int level, int pos) {
    auto& colorAttachment = colorAttachments_[pos];
    colorAttachment.tex = color;
    colorAttachment.layer = 0;
    colorAttachment.level = level;
    colorReady_ = true;;
}

void FrameBuffer::setDepthAttachment(std::shared_ptr<Texture>& depth) {
    depthAttachment_.tex = depth;
    depthAttachment_.layer = 0;
    depthAttachment_.level = 0;
    depthReady_ = true;
}

const FrameBufferAttachment& FrameBuffer::getColorAttachment(const int pos) const {
    // 查询会改变map，所以这个函数不能声明为const
    static FrameBufferAttachment nullAttachment {}; // 标准保证了右值传入const ref时生命期得到延长，但是返回时却没有这样的保证
    if (colorAttachments_.contains(pos)) {
        return colorAttachments_[pos];
    }
    return nullAttachment;
}

const std::unordered_map<int, FrameBufferAttachment>& FrameBuffer::getColorAttachments() const {
    return colorAttachments_;
}

const FrameBufferAttachment& FrameBuffer::getDepthAttachment() const {
    return depthAttachment_;
}

bool FrameBuffer::isColorReady() const {
    return colorReady_;
}

bool FrameBuffer::isDepthReady() const {
    return depthReady_;
}

bool FrameBuffer::isMultiSample() const {
    if (colorReady_) {

    }
    if (depthReady_) {
        return getDepthAttachment().tex->multiSample();
    }

    return false;
}

bool FrameBuffer::isOffscreen() const {
    return offscreen_;
}

void FrameBuffer::setOffscreen(bool offscreen) {
    offscreen_ = offscreen;
}
