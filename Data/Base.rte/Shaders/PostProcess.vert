#version 130

in vec2 rteVertexPosition;
in vec2 rteTexUV;

out vec2 textureUV;

uniform mat4 rteTransform;
uniform mat4 rteProjection;

void main() {
	gl_Position = rteProjection * rteTransform * vec4(rteVertexPosition, 0.0, 1.0);
	textureUV = vec2(rteTexUV.x, -rteTexUV.y);
}
