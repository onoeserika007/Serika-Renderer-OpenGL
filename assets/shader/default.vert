#version 330 core

uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProjection;

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 aTexCoord;
layout(location = 2) in vec3 aNormal;

void main()
{
    gl_Position = uProjection * uView * uModel * vec4(vec3(1.0f), 1.0);
}