// Blit8.frag
#version 300 es

precision highp float;
precision highp sampler2D;

in highp vec2 textureUV;

out highp vec4 FragColor;

uniform highp sampler2D rteTexture;
uniform highp sampler2D rtePalette;

void main() {
	float colorIndex = texture(rteTexture, vec2(textureUV.x, -textureUV.y)).r;
	FragColor = texture(rtePalette, vec2(colorIndex, 0.0F));
}
