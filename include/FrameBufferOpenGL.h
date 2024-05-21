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

    int getId() const override;

    bool isValid() const override;

    void setColorAttachment(std::shared_ptr<Texture>& color, int level, int pos = 0) override;

    void setColorAttachment(std::shared_ptr<Texture>& color, CubeMapFace face, int level, int pos = 0) override;

    void setDepthAttachment(std::shared_ptr<Texture>& depth) override;

    void bind() const override;

private:
    GLuint fbo_ = 0;
};