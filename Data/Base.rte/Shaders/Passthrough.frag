#version 330 core

in vec2 textureUV;
out vec4 FragColor;

uniform sampler2D texToPassthrough;

void main() {
	FragColor = vec4(texture(texToPassthrough, textureUV).rgb, 1.0);
	return;
}
