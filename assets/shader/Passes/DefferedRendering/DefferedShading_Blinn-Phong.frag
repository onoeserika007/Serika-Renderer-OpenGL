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
    bool uIsFirstFrame;
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

// ShadowMap
uniform sampler2D uShadowMap;

// CubeShadowMap
uniform samplerCube uShadowMapCube;

// Skybox
uniform samplerCube uCubeMap;

// SSAO
uniform sampler2D uSSAOTexture;

#define NUM_RINGS 10
#define MAX_LIGHT_NUM 4
#define MAX_TEXTURE_NUM 16
#define EPSILON 1e-3
#define DISTURBANCE 1e-3
#define PI 3.141592653589793
#define PI2 6.283185307179586

#define NUM_SAMPLES 20

// used to displace storedDisk
uniform samplerBuffer uSSAOKernel;

vec3 calcDirLight(vec3 normal, vec3 viewDir);
vec3 calcPointLight(vec3 normal, vec3 fragPos, vec3 viewDir);
float calcSpotLight(vec3 fragPos);

float rand() {
    return fract(sin(dot(gl_FragCoord.xy, vec2(12.9898, 78.233))) * 43758.5453);
}

float getDisturb() {
    return rand() * 2.f - 1.f;
}

float rand_1to1(float x) {
    // -1 -1
    return rand() * 2.f - 1.f;
    //    return fract(sin(x) * 10000.0);
}

float rand_2to1(vec2 uv) {
    // 0 - 1
    const highp float a = 12.9898, b = 78.233, c = 43758.5453;
    highp float dt = dot(uv.xy, vec2(a, b)), sn = mod(dt, PI);
    return fract(sin(sn) * c);
}

vec3 calcPhong(vec3 normal, vec3 viewDir);

float calcStandardShadowMap(vec3 shadowCoord);
float PCSS(vec3 coords);
float PCF(vec3 shadowCoord, float filterSize);

/******************** GLOBALS ***************************/
vec3 vWorldNormal = normalize(vec3(texture(gNormal_Specular, vTexCoord)));
vec3 vFragPos = texture(gPosition_ViewDepth, vTexCoord).xyz;
vec4 vPositionFromLight = uShadowMapVP * vec4(vFragPos, 1.f);

/*********************** MAIN **************************/
void main()
{
    vec3 norm = normalize(vWorldNormal);
    vec3 viewDir = normalize(uViewPos - vFragPos);
    vec3 phongColor = calcPhong(norm, viewDir);

    // shadow Map
    float visibility = 0.5f;
    vec3 shadowCoords = (vPositionFromLight.xyz / vPositionFromLight.w + 1.0f) / 2.0f;
    if (uUseShadowMap || uUseShadowMapCube) {
        //  visibility = calcStandardShadowMap(shadowCoords);
//        visibility = PCF(shadowCoords, 1.f / 1024.f);
//       if (uUseShadowMapCube) visibility = PCF(shadowCoords, 1.f / 1024.f * uFarPlane);
        visibility = PCSS(shadowCoords);
        phongColor *= visibility;
    }

    // evironment mapping
    if (uUseEnvMap) {
        vec3 reflectDir = reflect(-viewDir, vWorldNormal);
        vec3 mappingColor = vec3(texture(uCubeMap, reflectDir));
        #ifdef AMBIENT_MAP
        vec3 reflCoef = vec3(texture(uAmbientMap, vTexCoord));
        #else
        vec3 reflCoef = vec3(0.02);
        #endif
        phongColor += mappingColor * reflCoef;
    }

    FragColor = vec4(phongColor, 1.f);
}

/*********************** PHONG SHADING **************************/

vec3 calcPhong(vec3 normal, vec3 viewDir) {
    vec3 radiance = vec3(0.f);
    if (uLightType == 0) {
        FragColor = vec4(0.0f);
    }
    // point light
    else if (uLightType == 1) {
        // vec3 emissive = vec3(texture(material.emissive, vTexCoord));
        radiance = calcPointLight(normal, vFragPos, viewDir);
        radiance = radiance + DISTURBANCE * getDisturb();
    }
    // directional light
    else if (uLightType == 2) {
        radiance = calcDirLight(normal, viewDir);
        radiance = radiance + DISTURBANCE * getDisturb();
    }
    // spot light
    else if (uLightType == 3) {
        radiance = radiance * calcSpotLight(vFragPos);
    }
    else {
        radiance = vec3(1.0f);
    }
    return radiance;
}

vec3 calcPointLight(vec3 normal, vec3 fragPos, vec3 viewDir) {

    float constant = uLightConstant; // cannot modify a uniform
    if(abs(constant) < EPSILON) {
        constant = 1.0f;
    }

    vec3 lightDir = normalize(uLightPosition - fragPos);
    vec3 baseColor = vec3(texture(gDiffuse, vTexCoord));
    vec3 specCoef = vec3(texture(gNormal_Specular, vTexCoord).w);

    // 衰减
    float distance = length(uLightPosition - fragPos);
    // division by zero problem here!
    float attenuation = 1.0f / (constant + uLightLinear * distance + uLightQuadratic * (distance * distance));

    // 漫反射着色
    float diff = max(dot(normal, lightDir), 0.0);

    // 镜面光着色
    vec3 reflectDir = reflect(-lightDir, normal); // here I gets in, O gets out;
    vec3 halfVector = normalize(lightDir + viewDir);
    float spec = pow(max(dot(halfVector, normal), 0.0), 16.f);

    // 合并结果
    vec3 ambient  = uLightAmbient  * vec3(0.3) * (uUseSSAO? texture(uSSAOTexture, vTexCoord).x : 1.f) * (uIsFirstFrame? 1.f: 0.f);
    vec3 diffuse  = uLightDiffuse  * diff * baseColor;
    vec3 specular = uLightSpecular * spec * specCoef;

    vec3 result = ambient + diffuse + specular;

    return result * attenuation;
}

vec3 calcDirLight(vec3 normal, vec3 viewDir) {

    vec3 lightDir = normalize(-uLightDirection);
    vec3 baseColor = vec3(texture(gDiffuse, vTexCoord));
    vec3 specCoef = vec3(texture(gNormal_Specular, vTexCoord).w);

    // 漫反射着色
    float diff = max(dot(normal, lightDir), 0.0);

    // 镜面光着色
    vec3 reflectDir = reflect(-lightDir, normal);
    vec3 halfVector = normalize(lightDir + viewDir);
    float spec = pow(max(dot(halfVector, normal), 0.0), 16.f);

    // 合并结果
    vec3 ambient  = uLightAmbient  * vec3(0.3) * (uUseSSAO? texture(uSSAOTexture, vTexCoord).x : 1.f) * (uIsFirstFrame? 1.f: 0.f);
    vec3 diffuse  = uLightDiffuse  * diff * vec3(baseColor);
    vec3 specular = uLightSpecular * spec * specCoef;

    vec3 result = ambient + diffuse + specular;

    return result;
}

float calcSpotLight(vec3 fragPos) {
    vec3 lightDir = normalize(uLightPosition - fragPos); // from frag to light
    float theta = dot(normalize(-lightDir), uLightDirection);
    float epsilon = uLightCutoff - uLightOuterCutoff; // inner cos - outer cos

    // zero division
    if(abs(epsilon) < EPSILON) {
        epsilon = 1.0f;
    }
    float intensity = clamp((theta - uLightOuterCutoff) / epsilon, 0.0f, 1.0f);
    return intensity;
}

/*********************** SHADOW MAP **************************/

#define LIGHT_WIDTH 70.0
#define RESOLUTION 2048.0
float getBias() {
    vec3 LightDir = normalize(uLightPosition - vFragPos);
    vec3 Normal = normalize(vWorldNormal);
    return max(EPSILON, EPSILON * (1.0 - dot(LightDir, Normal)));
}

float sampleShadowMap2D(vec2 shadowCoords) {
    return texture(uShadowMap, shadowCoords.xy).x;
}

float sampleShadowMapCube(vec3 fragPos) {
    vec3 lightDir = uLightPosition - fragPos;
    return texture(uShadowMapCube, -lightDir).x;
}

float findBlocker(vec2 uv, float zReceiver) {
    float avgBlockerDepth = 0.0;
    float blockerNUM = 0.0;
    float blockerSearchRange = 5.0 / RESOLUTION;
    // poissonDiskSamples(uv);
    for(int i=0; i<NUM_SAMPLES; i++) {
        vec3 randomVec = normalize(texelFetch(uSSAOKernel, i).xyz);
        float blockerDepth = sampleShadowMap2D(uv + blockerSearchRange * normalize(randomVec.xy));
        if (uUseShadowMapCube) blockerDepth = sampleShadowMapCube(vFragPos + blockerSearchRange * randomVec * uFarPlane);
        if(blockerDepth + EPSILON <= zReceiver) {
            avgBlockerDepth += blockerDepth;
            blockerNUM += 1.0;
        }
    }
    if(blockerNUM == 0.0) return 1.0;
    else return avgBlockerDepth / blockerNUM;
}

float PCF(vec3 shadowCoord, float filterSize) {
    if(filterSize == 0.0) return 1.0f;
    float visibility = 0.0;
    float num = float(NUM_SAMPLES);

    float FragDepth = 0.f;
    if (uUseShadowMap) FragDepth = shadowCoord.z;
    else FragDepth = length(uLightPosition - vFragPos) / uFarPlane;

    for (int i = 0; i < NUM_SAMPLES; i++) {
        vec3 randomVec = normalize(texelFetch(uSSAOKernel, i).xyz);
        float LightDepth = 0.f;
        if (uUseShadowMap) LightDepth = sampleShadowMap2D(shadowCoord.xy + randomVec.xy * filterSize);
        else LightDepth = sampleShadowMapCube(vFragPos + randomVec * filterSize);
        visibility += (LightDepth + EPSILON <= FragDepth - getBias() ? 0.0 : 1.0);
    }
    return visibility / num;
}

float PCSS(vec3 coords) {
    float zReceiver = coords.z;
    if (uUseShadowMapCube) zReceiver = length(uLightPosition - vFragPos) / uFarPlane;
    // STEP 1: avgblocker depth
    float avgBlockerDepth = findBlocker(coords.xy, zReceiver);
    // STEP 2: penumbra size
    float penumbraSize;
    if(avgBlockerDepth == 1.0) penumbraSize = 0.0;
    else penumbraSize = (zReceiver - avgBlockerDepth) * LIGHT_WIDTH / avgBlockerDepth;
    // STEP 3: filtering
    float visibility = PCF(coords, penumbraSize / RESOLUTION);
    return visibility;
}

float calcStandardShadowMap(vec3 shadowCoord) {

    float LightDepth = 0.f;
    if (uUseShadowMap) LightDepth = sampleShadowMap2D(shadowCoord.xy);
    else LightDepth = sampleShadowMapCube(vFragPos);

    // return LightDepth;

    float FragDepth = 0.f;
    if (uUseShadowMap) FragDepth = shadowCoord.z;
    else FragDepth = length(uLightPosition - vFragPos) / uFarPlane;

    if (FragDepth > 1.f) FragDepth = 0.f;
    return LightDepth + EPSILON <= FragDepth - getBias() ? 0.0 : 1.0;
}
