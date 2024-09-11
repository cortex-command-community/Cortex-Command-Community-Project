// Blit8.frag
#version 130

in vec2 textureUV;

out vec4 FragColor;

uniform sampler2D rteTexture;
uniform sampler2D rtePalette;

void main() {
	float colorIndex = texture(rteTexture, vec2(textureUV.x, -textureUV.y)).r;
	FragColor = texture(rtePalette, vec2(colorIndex, 0.0F));
}
