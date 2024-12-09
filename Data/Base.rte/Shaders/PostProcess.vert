#version 130

in vec3 rteVertexPosition;
in vec2 rteVertexTexUV;
in vec4 rteVertexColor;

out vec2 textureUV;
out vec4 vertexColor;

uniform mat4 rteModel;
uniform mat4 rteProjection;

void main() {
	gl_Position = rteProjection * rteModel * vec4(rteVertexPosition, 1.0);
	vertexColor = rteVertexColor;
	textureUV = rteVertexTexUV;
}
