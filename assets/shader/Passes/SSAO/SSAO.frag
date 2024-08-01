#version 430 core

in vec2 vTexCoord;

layout(location = 0) out vec4 FragColor;

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

layout(std140) uniform Light {
    int uLightType;

    vec3 uLightPosition;
    vec3 uLightDirection;
    vec3 uLightAmbient;
    vec3 uLightDiffuse;
    vec3 uLightSpecular;

    float uLightCutoff;
    float uLightOuterCutoff;
    float uLightConstant;
    float uLightLinear;
    float uLightQuadratic;
};

layout(std140) uniform Scene {
	int uScreenWidth;
	int uScreenHeight;
    bool uUseSSAO;
};

layout(std140) uniform ShadowCube {
    mat4 uShadowVPs[6];
    float uFarPlane;
};

// gBuffers
// This Name Must be aligned with which in RenderPassGem=ometry.cpp!!!
uniform sampler2D gPosition_ViewDepth;
uniform sampler2D gDiffuse;
uniform sampler2D gNormal_Specular;

uniform sampler2D uRandomVecs;

// tBuffer
uniform samplerBuffer uSSAOKernel;

#define PI 3.141592653589793
#define PI2 6.283185307179586


float rand_2to1(vec2 uv) {
    // 0 - 1
    const highp float a = 12.9898, b = 78.233, c = 43758.5453;
    highp float dt = dot(uv.xy, vec2(a, b)), sn = mod(dt, PI);
    return fract(sin(sn) * c);
}


/******************** GLOBALS ***************************/
#define SAMPLE_NUM 64
#define RANDOM_VEC_NUM 16
float radius = 0.1f;

vec2 noiseScale = vec2(float(uScreenWidth) /4.0, float(uScreenHeight)/4.0);

// 前32个用作采样点，后32个用作随机旋转
vec3 vWorldNormal = normalize(vec3(texture(gNormal_Specular, vTexCoord)));
vec3 vFragPos = texture(gPosition_ViewDepth, vTexCoord).xyz;
int randIndexVec = int(rand_2to1(gl_FragCoord.xy) * float(RANDOM_VEC_NUM));

vec3 randomVec = texture(uRandomVecs, vTexCoord * noiseScale).xyz;

// vec3 randomVector = texelFetch(ssaoKernels, 0).xyz;


/*********************** MAIN **************************/
void main()
{
    // TBN
    vec3 tangent = normalize(randomVec - vWorldNormal * dot(randomVec, vWorldNormal));
    vec3 bitangent = cross(vWorldNormal, tangent);
    mat3 TBN = mat3(tangent, bitangent, vWorldNormal);
    float occlusion = 0.f;

    vec4 offset = uProjection * uView * vec4(vFragPos, 1.f);
    offset.xyz = offset.xyz / offset.w;
    float depth = texture(gPosition_ViewDepth, offset.xy).w;

    int consisderedCounts = 0;
    for (int i = 0; i < SAMPLE_NUM; i++) {
        vec4 ssaoSample = vec4(vFragPos + TBN * texelFetch(uSSAOKernel, i).xyz * radius, 1.f);
        ssaoSample = uView * ssaoSample; // stay in view space
        vec4 offset = uProjection * ssaoSample;
        offset.xyz = offset.xyz / offset.w;
        vec2 sampleCoords = (offset.xy + 1.f) / 2.f;

        // depth in view space, but we must look up with NDC coords shifted to 0-1 from -1-1
        float sceneDepth = texture(gPosition_ViewDepth, sampleCoords).w;
        float rangeCheck = smoothstep(0.f, 1.f, radius / abs(sceneDepth - (-ssaoSample.z)));
        occlusion += (sceneDepth <= -ssaoSample.z? 1.f: 0.f) * rangeCheck ;
    }
    FragColor.x = 1.f - occlusion / float(SAMPLE_NUM);
}

