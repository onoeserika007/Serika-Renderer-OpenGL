// #version 330 core

in vec2 TexCoord;
in vec3 worldNormal;
in vec3 fragPos;
in vec3 viewFragPos;

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

mat3 calcDirLight(vec3 normal, vec3 viewDir);
mat3 calcPointLight(vec3 normal, vec3 fragPos, vec3 viewDir);
float calcSpotLight(vec3 fragPos);

void main()
{
    vec3 norm = normalize(worldNormal);
    vec3 viewDir = normalize(viewPos - fragPos);
    if (uLightType == 0) {
        gl_FragColor = vec4(0.0f);
        return;
    }
    // point light
    else if (uLightType == 1) {
        // vec3 emissive = vec3(texture(material.emissive, TexCoord));
        mat3 result = calcPointLight(norm, fragPos, viewDir);

        gl_FragColor = vec4(result[0] + result[1] + result[2], 1.0);
    }
    // directional light
    else if (uLightType == 2) {
        mat3 result = calcDirLight(norm, viewDir);

        gl_FragColor = vec4(result[0] + result[1] + result[2], 1.0);
    }
    // spot light
    else if (uLightType == 3) {
        float result = calcSpotLight(fragPos);

        gl_FragColor = vec4(1.0f) * result;
    }
    else {
        gl_FragColor = vec4(1.0f);
    }
}

mat3 calcDirLight(vec3 normal, vec3 viewDir) {
    vec3 lightDir = normalize(uLightPosition - uLightDirection);
    vec3 ambient  = vec3(0.f);
    vec3 diffuse  = vec3(0.f);
    vec3 specular = vec3(0.f);
    #ifdef DIFFUSE_MAP
    // 漫反射着色
    float diff = max(dot(normal, lightDir), 0.0);
    // 合并结果
    ambient  = uLightAmbient  * vec3(texture(uDiffuseMap, TexCoord));
    diffuse  = uLightDiffuse  * diff * vec3(texture(uDiffuseMap, TexCoord));
    #endif

    #ifdef SPECULAR_MAP
    // 镜面光着色
    vec3 reflectDir = reflect(-lightDir, normal);
    vec3 halfVector = (reflectDir + viewDir) / 2.f;
    float spec = pow(max(dot(halfVector, normal), 0.0), 32.f);
    specular = uLightSpecular * spec * vec3(texture(uSpecularMap, TexCoord));
    #endif
    return mat3(ambient, diffuse, specular);
}

mat3 calcPointLight(vec3 normal, vec3 fragPos, vec3 viewDir) {

    float constant = uLightConstant; // cannot modify a uniform
    if(abs(constant) < EPSILON) {
        constant = 1.0f;
    }

    vec3 lightDir = normalize(uLightPosition - fragPos);
    vec3 ambient  = vec3(0.f);
    vec3 diffuse  = vec3(0.f);
    vec3 specular = vec3(0.f);

    // 衰减
    float distance = length(uLightPosition - fragPos);
    // division by zero problem here!
    float attenuation = 1.0 / (uLightConstant + uLightLinear * distance + uLightQuadratic * (distance * distance));

#ifdef DIFFUSE_MAP
    // 漫反射着色
    float diff = max(dot(normal, lightDir), 0.0);
    // 合并结果
    ambient  = uLightDiffuse  * vec3(texture(uDiffuseMap, TexCoord));
    diffuse  = uLightDiffuse  * diff * vec3(texture(uDiffuseMap, TexCoord));
#endif

    #ifdef SPECULAR_MAP
    // 镜面光着色
    vec3 reflectDir = reflect(-lightDir, normal);
    vec3 halfVector = (reflectDir + viewDir) / 2.f;
    float spec = pow(max(dot(halfVector, normal), 0.0), 32.f);
    specular = uLightSpecular * spec * vec3(texture(uSpecularMap, TexCoord));
    #endif

    ambient  *= attenuation;
    diffuse  *= attenuation;
    specular *= attenuation;

    return mat3(ambient, diffuse, specular);
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