/*
 * SoftGLRender
 * @author 	: keith@robot9.me
 *
 */

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
 // 实现在这里

#include <stb/stb_image.h>
#include <stb/stb_image_write.h>
#include "Utils/ImageUtils.h"
#include "Utils/Logger.h"


std::shared_ptr<Buffer<RGBA>> ImageUtils::readImageRGBA(const std::string& path, bool flipY) {
    // 你可能注意到纹理上下颠倒了！这是因为OpenGL要求y轴0.0坐标是在图片的底部的，但是图片的y轴0.0坐标通常在顶部。
    // 很幸运，stb_image.h能够在图像加载时帮助我们翻转y轴，只需要在加载任何图像前加入以下语句即可：
    //stbi_set_flip_vertically_on_load(flipY);
    // 似乎因为索引加载方式可以不用反转图像
    int iw = 0, ih = 0, n = 0;
    unsigned char* data = stbi_load(path.c_str(), &iw, &ih, &n, STBI_default);
    if (data == nullptr) {
        LOGD("ImageUtils::readImage failed, path: %s", path.c_str());
        return nullptr;
    }
    auto buffer = Buffer<RGBA>::makeDefault(iw, ih);

    // convert to rgba
    for (size_t y = 0; y < ih; y++) {
        for (size_t x = 0; x < iw; x++) {
            auto& to = buffer->get(x, y);
            size_t idx = x + y * iw;

            switch (n) {
            case STBI_grey: {
                to.r = data[idx];
                to.g = to.b = to.r;
                to.a = 255;
                break;
            }
            case STBI_grey_alpha: {
                to.r = data[idx * 2 + 0];
                to.g = to.b = to.r;
                to.a = data[idx * 2 + 1];
                break;
            }
            case STBI_rgb: {
                to.r = data[idx * 3 + 0];
                to.g = data[idx * 3 + 1];
                to.b = data[idx * 3 + 2];
                to.a = 255;
                break;
            }
            case STBI_rgb_alpha: {
                to.r = data[idx * 4 + 0];
                to.g = data[idx * 4 + 1];
                to.b = data[idx * 4 + 2];
                to.a = data[idx * 4 + 3];
                break;
            }
            default:
                break;
            }
        }
    }

    stbi_image_free(data);

    return buffer;
}

void ImageUtils::writeImage(char const* filename, int w, int h, int comp, const void* data, int strideInBytes,
    bool flipY) {
    stbi_flip_vertically_on_write(flipY);
    stbi_write_png(filename, w, h, comp, data, strideInBytes);
}

void ImageUtils::convertFloatImage(RGBA* dst, float* src, uint32_t width, uint32_t height) {
    float* srcPixel = src;

    float depthMin = FLT_MAX;
    float depthMax = FLT_MIN;
    for (int i = 0; i < width * height; i++) {
        float depth = *srcPixel;
        depthMin = std::min(depthMin, depth);
        depthMax = std::max(depthMax, depth);
        srcPixel++;
    }

    srcPixel = src;
    RGBA* dstPixel = dst;
    for (int i = 0; i < width * height; i++) {
        float depth = *srcPixel;
        depth = (depth - depthMin) / (depthMax - depthMin);
        dstPixel->r = glm::clamp((int)(depth * 255.f), 0, 255);
        dstPixel->g = dstPixel->r;
        dstPixel->b = dstPixel->r;
        dstPixel->a = 255;

        srcPixel++;
        dstPixel++;
    }
}


