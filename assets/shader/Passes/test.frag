#version 430 core

in vec3 vFragPos;
layout(location = 0) out vec4 FragColor;

layout(std140) uniform ShadowCube {
    mat4 uShadowVPs[6];
    float uFarPlane;
};

uniform samplerCube uCubeShadow;

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

void main() {
    // Get vector between fragment position and light position
    vec3 lightDir = uLightPosition - vFragPos;
    // Use the light to fragment vector to sample from the depth map    
    float closestDepth = texture(uCubeShadow, -lightDir).r;
    // It is currently in linear range between [0,1]. Re-transform back to original value
    // closestDepth *= uFarPlane;
    FragColor  = vec4(vec3(closestDepth), 1.f);
}