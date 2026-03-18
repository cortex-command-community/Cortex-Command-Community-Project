#version 300 es
// Flat.es.frag — GLSL ES 3.00 for WebGL 2
precision mediump float;

in  vec2 textureUV;
out vec4 FragColor;

uniform sampler2D rteTexture;
uniform vec4      rteColor;

void main() {
    FragColor = rteColor;
}
