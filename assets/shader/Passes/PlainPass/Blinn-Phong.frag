// #version 330 core

in vec2 TexCoord;
in vec3 worldNormal;
in vec3 fragPos;
in vec3 viewFragPos;

layout(location = 0) out vec4 FragColor;

layout(std140) uniform Model {
    mat4 uModel;
    mat4 uView;
    mat4 uProjection;
    mat4 uNormalToWorld;
    mat4 uShadowMapMVP;
    vec3 viewPos;
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
#define DISTURBANCE 3e-3

vec3 calcDirLight(vec3 normal, vec3 viewDir);
vec3 calcPointLight(vec3 normal, vec3 fragPos, vec3 viewDir);
float calcSpotLight(vec3 fragPos);

float rand() {
    return fract(sin(dot(gl_FragCoord.xy, vec2(12.9898, 78.233))) * 43758.5453);
}

float getDisturb() {
    return rand() * 2.f - 1.f;
}

void main()
{
    vec3 norm = normalize(worldNormal);
    vec3 viewDir = normalize(viewPos - fragPos);
    vec3 result = vec3(0.f);
    if (uLightType == 0) {
        FragColor = vec4(0.0f);
    }
    // point light
    else if (uLightType == 1) {
        // vec3 emissive = vec3(texture(material.emissive, TexCoord));
        result = calcPointLight(norm, fragPos, viewDir);
        result = result + DISTURBANCE * getDisturb();
    }
    // directional light
    else if (uLightType == 2) {
        result = calcDirLight(norm, viewDir);
    }
    // spot light
    else if (uLightType == 3) {
        result = result * calcSpotLight(fragPos);

    }
    else {
        FragColor = vec4(1.0f);
    }
    FragColor = vec4(result, 1.f);
}

vec3 calcPointLight(vec3 normal, vec3 fragPos, vec3 viewDir) {

    float constant = uLightConstant; // cannot modify a uniform
    if(abs(constant) < EPSILON) {
        constant = 1.0f;
    }

    vec3 lightDir = normalize(uLightPosition - fragPos);
    vec3 baseColor = vec3(0.3f);
    vec3 specCoef = vec3(0.f);
#ifdef DIFFUSE_MAP
    baseColor = vec3(texture(uDiffuseMap, TexCoord));
#endif

#ifdef SPECULAR_MAP
    specCoef = vec3(texture(uSpecularMap, TexCoord));
#endif

    // 衰减
    float distance = length(uLightPosition - fragPos);
    // division by zero problem here!
    float attenuation = 1.0f / (constant + uLightLinear * distance + uLightQuadratic * (distance * distance));

    // 漫反射着色
    float diff = max(dot(normal, lightDir), 0.0);

    // 镜面光着色
    vec3 reflectDir = reflect(-lightDir, normal);
    vec3 halfVector = normalize(reflectDir + viewDir);
    float spec = pow(max(dot(halfVector, normal), 0.0), 16.f);

    // 合并结果
    vec3 ambient  = uLightDiffuse  * baseColor;
    vec3 diffuse  = uLightDiffuse  * diff * baseColor;
    vec3 specular = uLightSpecular * spec * specCoef;

    vec3 result = ambient + diffuse + specular;

    return result * attenuation;
}

vec3 calcDirLight(vec3 normal, vec3 viewDir) {

    vec3 lightDir = normalize(uLightPosition - uLightDirection);
    vec3 baseColor = vec3(0.3f);
    vec3 specCoef = vec3(0.0f);
#ifdef DIFFUSE_MAP
    baseColor = vec3(texture(uDiffuseMap, TexCoord));
#endif

#ifdef SPECULAR_MAP
    specCoef = vec3(texture(uSpecularMap, TexCoord));
#endif

    // 漫反射着色
    float diff = max(dot(normal, lightDir), 0.0);

    // 镜面光着色
    vec3 reflectDir = reflect(-lightDir, normal);
    vec3 halfVector = normalize(reflectDir + viewDir);
    float spec = pow(max(dot(halfVector, normal), 0.0), 16.f);

    // 合并结果
    vec3 ambient  = uLightDiffuse  * vec3(baseColor);
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