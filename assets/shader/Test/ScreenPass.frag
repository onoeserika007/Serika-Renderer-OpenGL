in vec2 TexCoords;
out vec4 FragColor;

uniform sampler2D uTexture;
uniform float uExposure;
uniform bool uUseToneMapping;

void main()
{
    const float gamma = 2.2;
    vec3 hdrColor = texture(uTexture, TexCoords).rgb;
    vec3 switcher = ceil(clamp(hdrColor - vec3(0.0031308), 0.f, 1.f));

    // 曝光色调映射
    vec3 mapped = vec3(1.0) - exp(-hdrColor * uExposure);
    // Gamma校正
    // mapped = pow(mapped, vec3(1.0 / gamma));
    mapped = pow(mapped, vec3(1.0 / gamma));
    mapped = (1 - switcher) * hdrColor * 12.92 + switcher * (pow((hdrColor * 1.055), vec3(1.0 / 2.4)) - 0.055);

	// hdrColor = pow(hdrColor, vec3(1.0 / gamma));
	if (uUseToneMapping) FragColor = vec4(mapped, 1.0);
	else FragColor = vec4(hdrColor, 1.0);
}