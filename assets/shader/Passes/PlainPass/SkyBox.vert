#version 430 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 aTexCoord;
layout(location = 2) in vec3 aNormal;

out vec3 vTextureDir;
out vec3 vFragPos;

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
    mat4 viewMatrixStrippedTranslasiton = uView;
    viewMatrixStrippedTranslasiton[3] = vec4(0., 0., 0., 1.);
    vFragPos = vec3(uModel * vec4(aPos, 1.0));
    vTextureDir = normalize(vFragPos); // 0 0 0 as viewpos for skybox
    gl_Position = (uProjection * viewMatrixStrippedTranslasiton * uModel * vec4(aPos, 1.0)).xyww;
    gl_Position.z = gl_Position.z * 0.999999; // max depth
}