#version 430 core
in vec2 vTexCoord;
in vec3 vNormal;

layout(location = 0) out vec4 FragColor;

uniform vec3 uLineColor;

void main()
{
    FragColor = vec4(uLineColor, 1.f);
}