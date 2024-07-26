#pragma once

#include "Base/Globals.h"

enum DepthFunction {
    DepthFunc_NEVER,
    DepthFunc_LESS,
    DepthFunc_EQUAL,
    DepthFunc_LEQUAL,
    DepthFunc_GREATER,
    DepthFunc_NOTEQUAL,
    DepthFunc_GEQUAL,
    DepthFunc_ALWAYS,
};

enum BlendFactor {
    BlendFactor_ZERO,
    BlendFactor_ONE,
    BlendFactor_SRC_COLOR,
    BlendFactor_SRC_ALPHA,
    BlendFactor_DST_COLOR,
    BlendFactor_DST_ALPHA,
    BlendFactor_ONE_MINUS_SRC_COLOR,
    BlendFactor_ONE_MINUS_SRC_ALPHA,
    BlendFactor_ONE_MINUS_DST_COLOR,
    BlendFactor_ONE_MINUS_DST_ALPHA,
    BlendFactor_CONSTANT_COLOR,
    BlendFactor_ONE_MINUS_CONSTANT_COLOR
};

enum BlendFunction {
    BlendFunc_ADD,
    BlendFunc_SUBTRACT,
    BlendFunc_REVERSE_SUBTRACT,
    BlendFunc_MIN,
    BlendFunc_MAX,
};

enum PolygonMode {
    PolygonMode_POINT,
    PolygonMode_LINE,
    PolygonMode_FILL,
};

enum ECullMode {
    CullMode_BACK,
    CullMode_FRONT,
    CullMode_FRONT_AND_BACK
};

struct BlendParameters {
    BlendFunction blendFuncRgb = BlendFunc_ADD;
    BlendFactor blendSrcRgb = BlendFactor_ONE;
    BlendFactor blendDstRgb = BlendFactor_ZERO;

    BlendFunction blendFuncAlpha = BlendFunc_ADD;
    BlendFactor blendSrcAlpha = BlendFactor_ONE;
    BlendFactor blendDstAlpha = BlendFactor_ZERO;

    glm::vec4 blendColor;


    void SetBlendFactor(BlendFactor src, BlendFactor dst) {
        blendSrcRgb = src;
        blendSrcAlpha = src;
        blendDstRgb = dst;
        blendDstAlpha = dst;
    }

    void SetBlendFunc(BlendFunction func) {
        blendFuncRgb = func;
        blendFuncAlpha = func;
    }
};

enum PrimitiveType {
    Primitive_POINT,
    Primitive_LINE,
    Primitive_TRIANGLE,
};

struct RenderStates {
    bool blend = false;
    BlendParameters blendParams;

    bool depthTest = false;
    bool depthMask = true;
    DepthFunction depthFunc = DepthFunc_LESS;

    bool cullFace = false;
    ECullMode faceToCull = CullMode_BACK;
    PrimitiveType primitiveType = Primitive_TRIANGLE;
    PolygonMode polygonMode = PolygonMode_FILL;

    float lineWidth = 1.f;
    bool bBufferWrite = true;
    bool bBufferRead = false;
};

struct ClearStates {
    bool depthFlag = false;
    bool colorFlag = false;
    glm::vec4 clearColor = glm::vec4(0.f);
    float clearDepth = 1.f;
};

