#version 430 core
layout(location = 0) in vec3 aPos;

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

void main() {
    gl_Position = uModel * vec4(aPos, 1.0);
}