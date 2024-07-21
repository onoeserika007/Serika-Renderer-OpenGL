// #version 330 core

in vec2 TexCoord;
in vec3 fragPos;

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
    FragColor = texture(uDiffuseMap, TexCoord);
#else
    FragColor = vec4(1.f);
#endif
}