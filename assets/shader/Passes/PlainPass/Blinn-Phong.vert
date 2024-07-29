#version 430 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 aTexCoord;
layout(location = 2) in vec3 aNormal;

out vec2 vTexCoord;
out vec3 vWorldNormal;
out vec3 vFragPos;
out vec4 vPositionFromLight;

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
};

void main() {
    vFragPos = vec3(uModel * vec4(aPos, 1.0));
    vTexCoord = aTexCoord;
    vWorldNormal = vec3(uNormalToWorld * vec4(aNormal, 1.0));
    vPositionFromLight = uShadowMapVP * uModel * vec4(aPos, 1.0);
    gl_Position = uProjection * uView * uModel * vec4(aPos, 1.0);
}