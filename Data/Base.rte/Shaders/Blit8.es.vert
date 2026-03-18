#version 300 es
// Blit8.es.vert — GLSL ES 3.00 version of Blit8.vert for WebGL 2
// Converted from #version 330 core: profile changed, precision qualifier added.

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
