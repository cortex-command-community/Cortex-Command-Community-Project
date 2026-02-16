#version 330 core
in vec2 textureUV;
out vec4 FragColor;
uniform sampler2D uPrev;
uniform vec2 uViewSize;
void main() {
	FragColor = vec4(texture(uPrev, textureUV).rg, 0.0, 1.0);
}