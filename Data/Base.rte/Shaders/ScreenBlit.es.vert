#version 300 es
// ScreenBlit.es.vert — GLSL ES 3.00 for WebGL 2

precision mediump float;

in vec3 rteVertexPosition;
in vec2 rteVertexTexUV;
in vec4 rteVertexColor;

out vec2 textureUV;
out vec4 vertexColor;

uniform mat4 rteView;
uniform mat4 rteProjection;

void main() {
    gl_Position = rteProjection * rteView * vec4(rteVertexPosition, 1.0);
    textureUV   = rteVertexTexUV;
    vertexColor = rteVertexColor;
}
