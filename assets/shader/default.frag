#version 430 core
in vec2 vTexCoord;
in vec3 vNormal;

layout(location = 0) out vec4 FragColor;

uniform vec3 uFrameColor;

// optimize traced so deep, so you must use these two passed in params as well,
// to prevent them from optimized out.

void main()
{
    FragColor = vec4(vec3(vTexCoord, 1.f) * vNormal, 1.f);
//    FragColor = vec4(uFrameColor, 1.f); // write this line here will cause the line before optimized out, very huge problem.
}