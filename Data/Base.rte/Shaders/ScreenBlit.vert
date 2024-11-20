#version 130

in vec3 rteVertexPosition;
in vec2 rteVertexTexUV;

out vec2 textureUV;

uniform mat4 rteModel;
uniform mat4 rteProjection;

void main() {
	gl_Position = rteProjection * rteModel * vec4(rteVertexPosition, 1.0);
	textureUV = rteVertexTexUV;
}
