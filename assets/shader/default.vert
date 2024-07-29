#version 430 core
uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProjection;

// vertex in should be used, or they will be optimized out, and it won't perform as a specification for the vertex attribute input
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 aTexCoord;
layout(location = 2) in vec3 aNormal;

out vec2 vTexCoord;
out vec3 vNormal;


void main()
{
    vTexCoord = aTexCoord;
    vNormal = aNormal;
    gl_Position = uProjection * uView * uModel * vec4(aPos, 1.0);
}