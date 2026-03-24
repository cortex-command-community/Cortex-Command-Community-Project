#version 330 core

in vec3 rteVertexPosition;
in vec2 rteVertexTexUV;

out vec2 textureUV;

void main() {
	gl_Position = vec4(rteVertexPosition, 1.0);
	textureUV = rteVertexTexUV;
}
