#version 430 core

in vec2 vTexCoord;

layout(location = 0) out vec4 FragColor;

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

// GBuffers
uniform sampler2D gPosition;
uniform sampler2D gDiffuse;
uniform sampler2D gNormal;
uniform sampler2D gSpecular;

uniform sampler2D uAmbientMap;  // use abient map as reflection map

// ShadomMap
uniform sampler2D uShadowMap;

// Skybox
uniform samplerCube uCubeMap;

/************* CONSTANTS **************/
#define MAX_LIGHT_NUM 4
#define MAX_TEXTURE_NUM 16
#define EPSILON 1e-3
#define DISTURBANCE 3e-3
#define PI 3.141592653589793
#define PI2 6.283185307179586
#define NUM_RINGS 10
#define NUM_SAMPLES 10
#define LIGHT_WIDTH 10.f
#define RESOLUTION 1024.f

vec3 calcDirLight(vec3 normal, vec3 viewDir);
vec3 calcPointLight(vec3 normal, vec3 fragPos, vec3 viewDir);
float calcSpotLight(vec3 fragPos);

float rand() {
    return fract(sin(dot(gl_FragCoord.xy, vec2(12.9898, 78.233))) * 43758.5453);
}

float getDisturb() {
    return rand() * 2. - 1.;
}

float rand_1to1(float x) {
    // -1 -1
    // return rand() * 2. - 1.;
    return fract(sin(x) * 10000.0);
}

float rand_2to1(vec2 uv) {
    // 0 - 1
    const highp float a = 12.9898, b = 78.233, c = 43758.5453;
    highp float dt = dot(uv.xy, vec2(a, b)), sn = mod(dt, PI);
    return fract(sin(sn) * c);
}

vec3 calcPhong(vec3 normal, vec3 viewDir, vec3 fragPos);

float calcStandardShadowMap(sampler2D shadowMap, vec3 shadowCoord);
float PCSS(sampler2D shadowMap, vec3 coords);
float PCF(sampler2D shadowMap, vec3 shadowCoord, float filterSize);

/********* GLOBALS ********/
vec3 vWorldNormal = vec3(texture(gNormal, vTexCoord));
vec3 vFragPos = vec3(texture(gPosition, vTexCoord));

/*********************** MAIN **************************/
void main() {
    vec3 viewDir = normalize(uViewPos - vFragPos);

    vec3 phongColor = calcPhong(vWorldNormal, viewDir, vFragPos);

    // env mapping
    if(uUseEnvMap) {
        vec3 reflectDir = reflect(-viewDir, vWorldNormal);
        vec3 mappingColor = vec3(texture(uCubeMap, reflectDir));
        vec3 specCoef = vec3(texture(gSpecular, vTexCoord));
        phongColor += mappingColor * specCoef;
    }

    // shadow Map
    float visibility = 1.;
    if(uUseShadowMap) {
        vec4 posFromLight = uShadowMapMVP * vec4(vFragPos, 1.);
        vec3 shadowCoords = (posFromLight.xyz / posFromLight.w + 1.0) / 2.0;
//        visibility = calcStandardShadowMap(uShadowMap, shadowCoords);
//        visibility = PCF(uShadowMap, shadowCoords, 1. / RESOLUTION);
        visibility = PCSS(uShadowMap, shadowCoords);
    }

    FragColor = vec4(phongColor * visibility, 1.);
}

/*********************** PHONG SHADING **************************/

vec3 calcPhong(vec3 normal, vec3 viewDir, vec3 fragPos) {
    vec3 radiance = vec3(0.);
    if(uLightType == 0) {
        FragColor = vec4(0.0);
    }
    // point light
    else if(uLightType == 1) {
        // vec3 emissive = vec3(texture(material.emissive, vTexCoord));
        radiance = calcPointLight(normal, fragPos, viewDir);
        radiance = radiance + DISTURBANCE * getDisturb();
    }
    // directional light
    else if(uLightType == 2) {
        radiance = calcDirLight(normal, viewDir);
        radiance = radiance + DISTURBANCE * getDisturb();
    }
    // spot light
    else if(uLightType == 3) {
        radiance = radiance * calcSpotLight(fragPos);
    } else {
        radiance = vec3(1.0);
    }
    return radiance;
}

vec3 calcPointLight(vec3 normal, vec3 fragPos, vec3 viewDir) {

    float constant = uLightConstant; // cannot modify a uniform
    if(abs(constant) < EPSILON) {
        constant = 1.0;
    }

    vec3 lightDir = normalize(uLightPosition - fragPos);
    vec3 baseColor = vec3(texture(gDiffuse, vTexCoord));
    vec3 specCoef = vec3(texture(gSpecular, vTexCoord));

    // 衰减
    float distance = length(uLightPosition - fragPos);
    // division by zero problem here!
    float attenuation = 1.0 / (constant + uLightLinear * distance + uLightQuadratic * (distance * distance));

    // 漫反射着色
    float diff = max(dot(normal, lightDir), 0.0);

    // 镜面光着色
    vec3 reflectDir = reflect(-lightDir, normal); // here I gets in, O gets out;
    vec3 halfVector = normalize(lightDir + viewDir);
    float spec = pow(max(dot(halfVector, normal), 0.0), 16.);

    // 合并结果
    vec3 ambient = uLightAmbient * baseColor;
    vec3 diffuse = uLightDiffuse * diff * baseColor;
    vec3 specular = uLightSpecular * spec * specCoef;

    vec3 result = ambient + diffuse + specular;

    return result * attenuation;
}

vec3 calcDirLight(vec3 normal, vec3 viewDir) {

    vec3 lightDir = normalize(-uLightDirection);
    vec3 baseColor = vec3(texture(gDiffuse, vTexCoord));
    vec3 specCoef = vec3(texture(gSpecular, vTexCoord));

    // 漫反射着色
    float diff = max(dot(normal, lightDir), 0.0);

    // 镜面光着色
    vec3 reflectDir = reflect(-lightDir, normal);
    vec3 halfVector = normalize(lightDir + viewDir);
    float spec = pow(max(dot(halfVector, normal), 0.0), 16.);

    // 合并结果
    vec3 ambient = uLightAmbient * vec3(baseColor);
    vec3 diffuse = uLightDiffuse * diff * vec3(baseColor);
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
        epsilon = 1.0;
    }
    float intensity = clamp((theta - uLightOuterCutoff) / epsilon, 0.0, 1.0);
    return intensity;
}

/*********************** SHADOW MAP **************************/

vec2 poissonDisk[NUM_SAMPLES];

void poissonDiskSamples(const in vec2 randomSeed) {

    float ANGLE_STEP = PI2 * float(NUM_RINGS) / float(NUM_SAMPLES);
    float INV_NUM_SAMPLES = 1.0 / float(NUM_SAMPLES);

    float angle = rand_2to1(randomSeed) * PI2;
    float radius = INV_NUM_SAMPLES;
    float radiusStep = radius;

    for(int i = 0; i < NUM_SAMPLES; i++) {
        poissonDisk[i] = vec2(cos(angle), sin(angle)) * pow(radius, 0.75);
        radius += radiusStep;
        angle += ANGLE_STEP;
    }
}

void uniformDiskSamples(const in vec2 randomSeed) {

    float randNum = rand_2to1(randomSeed);
    float sampleX = rand_1to1(randNum);
    float sampleY = rand_1to1(sampleX);

    float angle = sampleX * PI2;
    float radius = sqrt(sampleY);

    for(int i = 0; i < NUM_SAMPLES; i++) {
        poissonDisk[i] = vec2(radius * cos(angle), radius * sin(angle));

        sampleX = rand_1to1(sampleY);
        sampleY = rand_1to1(sampleX);

        angle = sampleX * PI2;
        radius = sqrt(sampleY);
    }
}

float getBias() {
    vec3 LightDir = normalize(uLightPosition - vFragPos);
    vec3 Normal = normalize(vWorldNormal);
    return max(0., EPSILON * (1.0 - dot(LightDir, Normal)));
}

float findBlocker(sampler2D shadowMap, vec2 uv, float zReceiver) {
    float avgBlockerDepth = 0.0;
    float blockerNUM = 0.0;
    float blockerSearchRange = 5.0 / RESOLUTION;
    poissonDiskSamples(uv);
    for(int i = 0; i < NUM_SAMPLES; i++) {
        float blockerDepth = texture(shadowMap, uv + blockerSearchRange * poissonDisk[i]).x;
        if(blockerDepth + EPSILON <= zReceiver) {
            avgBlockerDepth += blockerDepth;
            blockerNUM += 1.0;
        }
    }
    if(blockerNUM == 0.0)
        return 1.0;
    else
        return avgBlockerDepth / blockerNUM;
}

// percentage-closer filtering
float PCF(sampler2D shadowMap, vec3 shadowCoord, float filterSize) {
    if(filterSize == 0.0)
        return 1.0;
    float visibility = 0.0;
    float num_samples = float(NUM_SAMPLES);
    vec2 randomSeed = shadowCoord.xy;
    uniformDiskSamples(randomSeed);
    // poissonDiskSamples(randomSeed);
    float FragDepth = shadowCoord.z;
    for(int i = 0; i < NUM_SAMPLES; i++) {
        float LightDepth = texture(shadowMap, shadowCoord.xy + poissonDisk[i] * filterSize).x;
        visibility += (LightDepth + EPSILON <= FragDepth - getBias() ? 0.0 : 1.0);
    }
    return visibility / num_samples;
}

float PCSS(sampler2D shadowMap, vec3 coords) {
    float zReceiver = coords.z;
    // STEP 1: avgblocker depth
    float avgBlockerDepth = findBlocker(shadowMap, coords.xy, coords.z);
    // STEP 2: penumbra size
    float penumbraSize;
    if(avgBlockerDepth == 1.0)
        penumbraSize = 0.0;
    else
        penumbraSize = (zReceiver - avgBlockerDepth) * LIGHT_WIDTH / avgBlockerDepth;
    // STEP 3: filtering
    float visibility = PCF(shadowMap, coords, penumbraSize / RESOLUTION);
    return visibility;
}

float calcStandardShadowMap(sampler2D shadowMap, vec3 shadowCoord) {
    float LightDepth = texture(uShadowMap, shadowCoord.xy).x;
    float FragDepth = shadowCoord.z;
    if(FragDepth > 1.)
        FragDepth = 0.;
    return LightDepth + EPSILON <= FragDepth - getBias() ? 0.0 : 1.0;
}
