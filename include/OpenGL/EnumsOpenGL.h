#pragma once
#include "Base/RenderStates.h"
#include <glad/glad.h>
#include "../Material/Texture.h"
#include "Utils/Logger.h"

namespace OpenGL {

    struct TextureOpenGLDesc {
        GLint internalformat;
        GLenum format;
        GLenum elemtype;
    };
;
#define CASE_CVT_GL(PRE, TOKEN) case PRE##TOKEN: return GL_##TOKEN

    static inline TextureOpenGLDesc cvtTextureFormat(TextureFormat format) {
        TextureOpenGLDesc ret{};

        switch (format) {
            case TextureFormat_RGBA8: {
                ret.internalformat = GL_RGBA;
                ret.format = GL_RGBA;
                ret.elemtype = GL_UNSIGNED_BYTE;
                break;
            }
            case TextureFormat_FLOAT32: {
                ret.internalformat = GL_DEPTH_COMPONENT;
                ret.format = GL_DEPTH_COMPONENT;
                ret.elemtype = GL_FLOAT;
                break;
            }
            case TextureFormat_RGB8: {
                ret.internalformat = GL_RGB;
                ret.format = GL_RGB;
                ret.elemtype = GL_UNSIGNED_BYTE;
                break;
            }
            case TextureFormat_RGB16F: {
                ret.internalformat = GL_RGB16F; // targetFormat
                ret.format = GL_RGB;    // srcFormat
                ret.elemtype = GL_FLOAT;
                break;
            }
            case TextureFormat_RGB32F: {
                ret.internalformat = GL_RGB32F; // targetFormat
                ret.format = GL_RGB;    // srcFormat
                ret.elemtype = GL_FLOAT;
                break;
            }
        }

        return ret;
    }

    static inline GLuint cvtTextureTarget(TextureTarget target) {
        switch (target) {
            CASE_CVT_GL(TextureTarget_, TEXTURE_2D);
            CASE_CVT_GL(TextureTarget_, TEXTURE_2D_MULTISAMPLE);
            CASE_CVT_GL(TextureTarget_, TEXTURE_CUBE_MAP);
            CASE_CVT_GL(TextureTarget_, TEXTURE_BUFFER);
            default:
                break;
        }
        return GL_TEXTURE_2D;
    }

    static inline GLint cvtWrap(WrapMode mode) {
        switch (mode) {
            CASE_CVT_GL(Wrap_, REPEAT);
            CASE_CVT_GL(Wrap_, MIRRORED_REPEAT);
            CASE_CVT_GL(Wrap_, CLAMP_TO_EDGE);
            CASE_CVT_GL(Wrap_, CLAMP_TO_BORDER);
        default:
            break;
        }
        return GL_REPEAT;
    }

    static inline GLint cvtFilter(const FilterMode mode) {
        switch (mode) {
            CASE_CVT_GL(Filter_, LINEAR);
            CASE_CVT_GL(Filter_, NEAREST);
            CASE_CVT_GL(Filter_, LINEAR_MIPMAP_LINEAR);
            CASE_CVT_GL(Filter_, LINEAR_MIPMAP_NEAREST);
            CASE_CVT_GL(Filter_, NEAREST_MIPMAP_LINEAR);
            CASE_CVT_GL(Filter_, NEAREST_MIPMAP_NEAREST);
        default:
            break;
        }
        return GL_NEAREST;
    }

    static inline GLint cvtCubeFace(ECubeMapFace face) {
        switch (face) {
            CASE_CVT_GL(, TEXTURE_CUBE_MAP_POSITIVE_X);
            CASE_CVT_GL(, TEXTURE_CUBE_MAP_NEGATIVE_X);
            CASE_CVT_GL(, TEXTURE_CUBE_MAP_POSITIVE_Y);
            CASE_CVT_GL(, TEXTURE_CUBE_MAP_NEGATIVE_Y);
            CASE_CVT_GL(, TEXTURE_CUBE_MAP_POSITIVE_Z);
            CASE_CVT_GL(, TEXTURE_CUBE_MAP_NEGATIVE_Z);
        default:
            break;
        }
        return 0;
    }

    static inline GLint cvtDepthFunc(DepthFunction func) {
        switch (func) {
            CASE_CVT_GL(DepthFunc_, NEVER);
            CASE_CVT_GL(DepthFunc_, LESS);
            CASE_CVT_GL(DepthFunc_, EQUAL);
            CASE_CVT_GL(DepthFunc_, LEQUAL);
            CASE_CVT_GL(DepthFunc_, GREATER);
            CASE_CVT_GL(DepthFunc_, NOTEQUAL);
            CASE_CVT_GL(DepthFunc_, GEQUAL);
            CASE_CVT_GL(DepthFunc_, ALWAYS);
        default:
            break;
        }
        return 0;
    }

    static inline GLint cvtBlendFactor(BlendFactor factor) {
        switch (factor) {
            CASE_CVT_GL(BlendFactor_, ZERO);
            CASE_CVT_GL(BlendFactor_, ONE);
            CASE_CVT_GL(BlendFactor_, SRC_COLOR);
            CASE_CVT_GL(BlendFactor_, SRC_ALPHA);
            CASE_CVT_GL(BlendFactor_, DST_COLOR);
            CASE_CVT_GL(BlendFactor_, DST_ALPHA);
            CASE_CVT_GL(BlendFactor_, ONE_MINUS_SRC_COLOR);
            CASE_CVT_GL(BlendFactor_, ONE_MINUS_SRC_ALPHA);
            CASE_CVT_GL(BlendFactor_, ONE_MINUS_DST_COLOR);
            CASE_CVT_GL(BlendFactor_, ONE_MINUS_DST_ALPHA);
            CASE_CVT_GL(BlendFactor_, CONSTANT_COLOR);
            CASE_CVT_GL(BlendFactor_, ONE_MINUS_CONSTANT_COLOR);
        default:
            break;
        }
        return 0;
    }

    static inline GLint cvtBlendFunction(BlendFunction func) {
        switch (func) {
        case BlendFunc_ADD:               return GL_FUNC_ADD;
        case BlendFunc_SUBTRACT:          return GL_FUNC_SUBTRACT;
        case BlendFunc_REVERSE_SUBTRACT:  return GL_FUNC_REVERSE_SUBTRACT;
        case BlendFunc_MIN:               return GL_MIN;
        case BlendFunc_MAX:               return GL_MAX;
        default:
            break;
        }
        return 0;
    }

    static inline GLuint cvtCullMode(ECullMode mode) {
        switch (mode) {
            CASE_CVT_GL(CullMode_, BACK);
            CASE_CVT_GL(CullMode_, FRONT);
            CASE_CVT_GL(CullMode_, FRONT_AND_BACK);
            default:
                break;
        }
        return 0;
    }

    static inline GLint cvtPolygonMode(PolygonMode mode) {
        switch (mode) {
            CASE_CVT_GL(PolygonMode_, POINT);
            CASE_CVT_GL(PolygonMode_, LINE);
            CASE_CVT_GL(PolygonMode_, FILL);
        default:
            break;
        }
        return 0;
    }

    static inline GLint cvtDrawMode(PrimitiveType type) {
        switch (type) {
        case Primitive_POINT:       return GL_POINTS;
        case Primitive_LINE:        return GL_LINES;
        case Primitive_TRIANGLE:    return GL_TRIANGLES;
        default:
            break;
        }
        return 0;
    }

    static inline glm::vec4 cvtBorderColor(BorderColor color) {
        switch (color) {
        case Border_BLACK:          return glm::vec4(0.f);
        case Border_WHITE:          return glm::vec4(1.f);
        default:
            break;
        }
        return glm::vec4(0.f);
    }

}

