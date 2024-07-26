#version 430 core
in vec4 gFragPos;

layout(location = 0) out vec4 FragColor;

layout(std140) uniform ShadowCube {
    mat4 uShadowVPs[6];
    float uFarPlane;
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

void main()
{
    float lightDistance = length(gFragPos.xyz - uLightPosition);
    
    // map to [0;1] range by dividing by far_plane
    lightDistance = lightDistance / uFarPlane;
    
    // write this as modified depth
    gl_FragDepth = lightDistance;
    FragColor = vec4(1.f);
}