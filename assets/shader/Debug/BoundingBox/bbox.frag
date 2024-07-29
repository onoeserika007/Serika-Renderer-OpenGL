#version 430 core
in vec2 vTexCoord;
in vec3 vNormal;

layout(location = 0) out vec4 FragColor;

uniform vec3 uFrameColor;
uniform int uParentUUID;
uniform int uBBoxUUID;
uniform float uBBoxArea;
uniform float uLayerDepth;

// optimize traced so deep, so you must use these two passed in params as well,
// to prevent them from optimized out.

vec3 getLayerColor(int colorEnum) {
    vec3 color;
    switch(colorEnum) {
        case 0:
            color = vec3(1.0, 0.0, 0.0); // RED
            break;
        case 1:
            color = vec3(0.0, 1.0, 0.0); // GREEN
            break;
        case 2:
            color = vec3(0.0, 0.0, 1.0); // BLUE
            break;
        case 3:
            color = vec3(1.0, 1.0, 0.0); // YELLOW
            break;
        case 4:
            color = vec3(0.0, 1.0, 1.0); // CYAN
            break;
        case 5:
            color = vec3(1.0, 0.0, 1.0); // MAGENTA
            break;
        case 6:
            color = vec3(1.0, 0.647, 0.0); // ORANGE
            break;
        case 7:
            color = vec3(0.5, 0.0, 0.5); // PURPLE
            break;
        default:
            color = vec3(1.f); // DEFAULT: BLACK
            break;
    }
    return color;
}

void main()
{
    float res = float(uParentUUID + uBBoxUUID + uBBoxArea);
    FragColor = vec4(vec3(vTexCoord, 1.f) * vNormal, 1.f);
    FragColor = vec4(getLayerColor(int(uLayerDepth)), 1.f);
}