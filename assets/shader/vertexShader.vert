#version 330 core
//layout (location = 0) in vec3 aPos;
//layout (location = 1) in vec3 aColor;
//layout (location = 2) in vec2 aTexCoord;

// for cube
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;

out vec2 TexCoord;

uniform mat4 transform;
uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProjection;

void main()
{
    TexCoord = aTexCoord;
    gl_Position = uProjection * uView * uModel * vec4(aPos, 1.0);
    // gl_Position = projection * view * vec4(aPos, 1.0);
}