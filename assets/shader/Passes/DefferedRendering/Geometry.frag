#version 430 core

in vec2 vTexCoord;
in vec3 vWorldNormal;
in vec3 vFragPos;

layout (location = 0) out vec3 WorldPosOut;
layout (location = 1) out vec3 DiffuseOut;
layout (location = 2) out vec3 NormalOut;
layout (location = 3) out vec3 SpecularOut;

layout(std140) uniform Model {
    mat4 uModel;
    mat4 uView;
    mat4 uProjection;
    mat4 uNormalToWorld;
    mat4 uShadowMapMVP;
    vec3 uViewPos;
    bool uUseShadowMap;
    bool uUseShadowMapCube;
    bool uUseEnvMap;
};

#ifdef DIFFUSE_MAP
    uniform sampler2D uDiffuseMap;
#endif

#ifdef SPECULAR_MAP
    uniform sampler2D uSpecularMap;
#endif

#ifdef HEIGHT_MAP
    uniform sampler2D uHeightMap;
#endif

#define MAX_LIGHT_NUM 4
#define MAX_TEXTURE_NUM 16
#define EPSILON 1e-3

void main()
{
    // pos
    WorldPosOut = vFragPos;

    // diffuse
#ifdef DIFFUSE_MAP
    DiffuseOut = texture(uDiffuseMap, vTexCoord).xyz;
#else
    DiffuseOut = vec3(1.0f);
#endif

    // normal
    NormalOut = normalize(vWorldNormal);

    // spec
#ifdef SPECULAR_MAP
    SpecularOut = texture(uSpecularMap, vTexCoord).xyz;
#else
    SpecularOut = vec3(0.2f);
#endif
}