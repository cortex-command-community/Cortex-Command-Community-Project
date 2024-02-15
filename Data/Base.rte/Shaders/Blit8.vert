#version 130

in vec2 rteVertexPosition;
in vec2 rteTexUV;

out vec2 textureUV;

uniform mat4 rteTransform;
uniform mat4 rteProjection;
uniform mat4 rteUVTransform;

void main() {
	gl_Position = rteProjection * rteTransform * vec4(rteVertexPosition, 0.0, 1.0);
	textureUV = (rteUVTransform * vec4(rteTexUV, 0.0, 1.0)).xy;
}
