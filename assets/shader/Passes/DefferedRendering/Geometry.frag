// #version 330 core

in vec2 TexCoord;
in vec3 worldNormal;
in vec3 fragPos;

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
    WorldPosOut = fragPos;

    // diffuse
#ifdef DIFFUSE_MAP
    DiffuseOut = texture(uDiffuseMap, TexCoord).xyz;
#else
    DiffuseOut = vec3(0.5f);
#endif

    // normal
    NormalOut = normalize(worldNormal);

    // spec
#ifdef SPECULAR_MAP
    SpecularOut = texture(uSpecularMap, TexCoord).xyz;
#else
    SpecularOut = vec3(0.5f);
#endif
}