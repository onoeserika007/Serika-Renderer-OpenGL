#pragma once
#include <complex.h>
#include <complex.h>

#include "Buffer.h"
#include <memory>

#include "Buffer.h"
#include "Buffer.h"
#include "Base/Globals.h"

class ImageUtils {
public:
    static std::shared_ptr<Buffer<RGBA>> readImageRGBA(const std::string& path, bool flipY);
    static void writeImage(char const* filename, int w, int h, int comp, const void* data, int strideInBytes,
        bool flipY);
    static void writeImage(char const* filename, const std::shared_ptr<Buffer<RGBA>>& buffer, bool flipY);

    static void convertFloatImage(RGBA* dst, float* src, uint32_t width, uint32_t height);
};