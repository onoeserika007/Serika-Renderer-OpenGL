#version 430 core

in vec2 vTexCoord;
in vec3 vWorldNormal;
in vec3 vFragPos;

layout (location = 0) out vec4 WorldPos_ViewDepthOut;
layout (location = 1) out vec4 DiffuseOut;
layout (location = 2) out vec4 Normal_SpecularOut;
layout (location = 3) out vec4 Ao_Metal_RoughnessOut;

layout(std140) uniform Model {
    mat4 uModel;
    mat4 uView;
    mat4 uProjection;
    mat4 uNormalToWorld;
    mat4 uShadowMapVP;
    vec3 uViewPos;
    bool uUseShadowMap;
    bool uUseShadowMapCube;
    bool uUseEnvMap;
    bool uUsePureEmission;
    float uNearPlaneCamera;
	float uFarPlaneCamera;
};

layout(std140) uniform MaterialInfo {
    vec3 uAlbedo;
    vec3 uEmission;
	// Disney attributes
	float uSubsurface;
	float uMetallic;
	float uSpecular;
	float uSpecularTint;
	float uRoughness;
	float uAnisotropic;
	float uSheen;
	float uSheenTint;
	float uClearcoat;
	float uClearcoatGloss;
	float uIOR;
	float uTransmission;
};

float NEAR = uNearPlaneCamera;
float FAR = uFarPlaneCamera;

// view space
// 这里的值都是正数，但是注意，我们直接从worldPos乘ViewMatrix过去得到的都是负数
float LinearizeDepth(float depth) {
    float z = depth * 2.0 - 1.0; // 回到NDC
    return (2.0 * NEAR * FAR) / (FAR + NEAR - z * (FAR - NEAR));    
}

#ifdef DIFFUSE_MAP
    uniform sampler2D uDiffuseMap;
#endif

#ifdef SPECULAR_MAP
    uniform sampler2D uSpecularMap;
#endif

#ifdef HEIGHT_MAP
    uniform sampler2D uHeightMap;
#endif

// metalness
#ifdef METALNESS_MAP
    uniform sampler2D uMetalnessMap;
#endif

// ao map
#ifdef AMBIENT_OCCLUSION_MAP
    uniform sampler2D uAmbientOcclusionMap;
#endif

// roughness
#ifdef DIFFUSE_ROUGHNESS_MAP
    uniform sampler2D uDiffuseRoughnessMap;
#endif

#define MAX_LIGHT_NUM 4
#define MAX_TEXTURE_NUM 16
#define EPSILON 1e-3


void main()
{
    // pos
    WorldPos_ViewDepthOut.xyz = vFragPos;
    WorldPos_ViewDepthOut.w = LinearizeDepth(gl_FragCoord.z);

    // diffuse
    // ao and albedo need to be transformed from sRGB to linear
#ifdef DIFFUSE_MAP
    vec3 albedo = pow(vec3(texture(uDiffuseMap, vTexCoord).xyz), vec3(2.2));
    DiffuseOut = vec4(albedo, 1.f);
#else
    DiffuseOut = vec4(1.0f);
#endif

    // normal
    Normal_SpecularOut.xyz = normalize(vWorldNormal);

    // spec
#ifdef SPECULAR_MAP
    Normal_SpecularOut.w = texture(uSpecularMap, vTexCoord).x;
#else
    Normal_SpecularOut.w = 0.2f;
#endif

    // ao map
    // ao and albedo need to be transformed from sRGB to linear
#ifdef AMBIENT_OCCLUSION_MAP
    Ao_Metal_RoughnessOut.y = pow(texture(uAmbientOcclusionMap, vTexCoord).x, 2.2);
#else
    Ao_Metal_RoughnessOut.y = 1.f;
#endif

    // metalness
#ifdef METALNESS_MAP
    Ao_Metal_RoughnessOut.x = texture(uMetalnessMap, vTexCoord).x;
#else
    Ao_Metal_RoughnessOut.x = uMetallic;
#endif

// roughness
#ifdef DIFFUSE_ROUGHNESS_MAP
    Ao_Metal_RoughnessOut.z = texture(uDiffuseRoughnessMap, vTexCoord).x;
#else
    Ao_Metal_RoughnessOut.z = uRoughness;
#endif
}