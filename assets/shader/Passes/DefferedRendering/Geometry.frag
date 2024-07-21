// #version 330 core

in vec2 TexCoord;
in vec3 worldNormal;
in vec3 fragPos;
in vec3 viewFragPos;
in vec3 localNormal;

layout (location = 0) out vec3 WorldPosOut;
layout (location = 1) out vec3 DiffuseOut;
layout (location = 2) out vec3 NormalOut;
layout (location = 3) out vec3 SpecularOut;

layout(std140) uniform Model {
    mat4 uModel;
    mat4 uView;
    mat4 uProjection;
    mat4 uNormalToWorld;
    vec3 viewPos;
};

layout(std140) uniform Light {
    uint lightType;

    vec3 position;
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;

    float cutoff;
    float outerCutoff;
    float constant;
    float linear;
    float quadratic;
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
    WorldPosOut = fragPos;
#ifdef DIFFUSE_MAP
    DiffuseOut = texture(uDiffuseMap, TexCoord).xyz;
#endif
    NormalOut = normalize(worldNormal);
#ifdef SPECULAR_MAP
    SpecularOut = texture(uSpecularMap, TexCoord).xyz;
#endif
}