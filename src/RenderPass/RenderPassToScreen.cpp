#include "RenderPass/RenderPassToScreen.h"

const char* VS = R"(
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;

out vec2 TexCoord;

void main()
{
	gl_Position = vec4(aPos, 1.0);
	TexCoord = vec2(aTexCoord.x, aTexCoord.y);
}
)";

const char* FS = R"(
in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2D uTexture;

void main()
{
    FragColor = texture(uTexture, TexCoord);
}
)";

// set up vertex data (and buffer(s)) and configure vertex attributes
float vertices[] = {
    // positions | texture coords
    1.f, 1.f, 0.0f, 1.0f, 1.0f,   // top right
    1.f, -1.f, 0.0f, 1.0f, 0.0f,  // bottom right
    -1.f, -1.f, 0.0f, 0.0f, 0.0f, // bottom left
    -1.f, 1.f, 0.0f, 0.0f, 1.0f   // top left
};
unsigned int indices[] = {
    0, 1, 3, // first triangle
    1, 2, 3  // second triangle
};

RenderPassToScreen::RenderPassToScreen(Renderer& renderer): RenderPass(renderer)
{
}
