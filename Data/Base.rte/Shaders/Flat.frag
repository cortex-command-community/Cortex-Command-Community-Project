#version 130

in vec2 textureUV;

out vec4 FragColor;

uniform sampler2D rteTexture;
uniform vec4 rteColor;

void main() {
	FragColor = rteColor;
}
