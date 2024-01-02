//Blit8.frag
#version 330 core

in vec2 textureUV;

out vec4 FragColor;

uniform sampler2D rteTexture;
uniform sampler1D rtePalette;

void main()
{
	float colorIndex = texture(rteTexture, vec2(textureUV.x, -textureUV.y)).r;
	FragColor = texture(rtePalette, colorIndex);
}
