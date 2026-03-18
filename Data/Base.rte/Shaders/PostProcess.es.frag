#version 300 es
// PostProcess.es.frag — GLSL ES 3.00 for WebGL 2

precision mediump float;

in  vec2 textureUV;
in  vec4 vertexColor;
out vec4 FragColor;

uniform sampler2D rteTexture;
uniform vec4      rteColor;

vec4 textureAA(sampler2D tex, vec2 uv) {
    vec2 texsize = vec2(textureSize(tex, 0));
    vec2 uv_ts   = uv * texsize;
    vec2 seam    = floor(uv_ts + 0.5);
    vec2 dv      = max(fwidth(uv_ts), vec2(0.001));
    uv_ts        = (uv_ts - seam) / dv + seam;
    uv_ts        = clamp(uv_ts, seam - 0.5, seam + 0.5);
    return texture(tex, uv_ts / texsize);
}

void main() {
    FragColor = textureAA(rteTexture, textureUV) * rteColor * vertexColor;
}
