#version 300 es

precision highp float;
precision highp sampler2D;

in highp vec2 textureUV;

out highp vec4 FragColor;

uniform highp sampler2D rteTexture;
uniform highp vec4 rteColor;

void main() {
	FragColor = texture(rteTexture, textureUV) * rteColor;
}
