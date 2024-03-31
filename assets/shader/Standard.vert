// #version 330 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 aTexCoord;
layout(location = 2) in vec3 aNormal;

out vec2 TexCoord;
out vec3 worldNormal;
out vec3 fragPos;
out vec3 viewFragPos;

layout(std140) uniform Model {
    mat4 uModel;
    mat4 uView;
    mat4 uProjection;
    mat4 uNormalToWorld;
    vec3 viewPos;
};

uniform float setValue;

void main() {
    fragPos = vec3(uModel * vec4(aPos, 1.0));
    viewFragPos = vec3(uView * vec4(fragPos, 1.0));
    TexCoord = aTexCoord;
    worldNormal = vec3(uNormalToWorld * vec4(aNormal, 1.0));
    gl_Position = uProjection * uView * uModel * vec4(aPos, 1.0);
}