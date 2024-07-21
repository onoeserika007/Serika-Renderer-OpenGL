in vec2 vTexCoord;
in vec3 vNormal;

layout(location = 0) out vec4 FragColor;

// optimize traced so deep, so you must use these two passed in params as well,
// to prevent them from optimized out.

void main()
{
    FragColor = vec4(vec3(vTexCoord, 1.f) * vNormal, 1.f);
}