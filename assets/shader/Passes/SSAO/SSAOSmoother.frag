#version 430 core

in vec2 vTexCoord;

layout(location = 0) out vec4 FragColor;


uniform sampler2D uSSAOInput;


/******************** GLOBALS ***************************/


/*********************** MAIN **************************/
void main()
{
    // 0 - LOD
    vec2 texelSize = 1.0 / vec2(textureSize(uSSAOInput, 0));
    float result = 0.0;
    for (int x = -2; x < 2; ++x) 
    {
        for (int y = -2; y < 2; ++y) 
        {
            vec2 offset = vec2(float(x), float(y)) * texelSize;
            result += texture(uSSAOInput, vTexCoord + offset).r;
        }
    }
    FragColor.x = result / (4.0 * 4.0);
}

