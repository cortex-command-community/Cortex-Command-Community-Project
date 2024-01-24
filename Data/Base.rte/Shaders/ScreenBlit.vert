#version 300 es

precision highp float;

layout(location = 0) in highp vec2 rteVertexPosition;
layout(location = 1) in highp vec2 rteTexUV;

out highp vec2 textureUV;

uniform highp mat4 rteTransform;
uniform highp mat4 rteProjection;

void main() {
	gl_Position = rteProjection * rteTransform * vec4(rteVertexPosition.xy, 0.0, 1.0);
	textureUV = rteTexUV;
}
