#version 430 core

in vec2 vTexCoord;
in vec3 vFragPos;

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

layout(std140) uniform MaterialInfo {
    vec3 uAlbedo;
    vec3 uEmission;
	// Disney attributes
	float uSubsurface;
	float uMetallic;
	float uSpecular;
	float uSpecularTint;
	float uRoughness;
	float uAnisotropic;
	float uSheen;
	float uSheenTint;
	float uClearcoat;
	float uClearcoatGloss;
	float uIOR;
	float uTransmission;
};

layout(location = 0) out vec4 FragColor;


#ifdef DIFFUSE_MAP
    uniform sampler2D uDiffuseMap;
#endif

#ifdef SPECULAR_MAP
    uniform sampler2D uSpecularMap;
#endif

#ifdef HEIGHT_MAP
    uniform sampler2D uHeightMap;
#endif

void main()
{
#ifdef DIFFUSE_MAP
    FragColor = texture(uDiffuseMap, vTexCoord);
#else
    if (uUsePureEmission) {
        FragColor = vec4(uEmission, 1.f);
    }
    else {
        FragColor = vec4(0.7f);
    }
#endif
}