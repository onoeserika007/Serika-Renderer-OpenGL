#version 430 core
in vec3 vTextureDir;
in vec3 vFragPos;

layout(location = 0) out vec4 FragColor;

uniform samplerCube uCubeMap;

void main()
{
   FragColor = texture(uCubeMap, vTextureDir);
}