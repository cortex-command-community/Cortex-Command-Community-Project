#version 300 es
// Background.es.frag — GLSL ES 3.00 for WebGL 2
// Stripped of GL_KHR_blend_equation_advanced (not in WebGL 2)
precision mediump float;

in  vec2 textureUV;
in  vec4 vertexColor;
out vec4 FragColor;

uniform sampler2D rteTexture;
uniform sampler2D rtePalette;
uniform vec4      rteColor;
uniform bool      rteBlendInvert;
uniform bool      drawMasked;

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
    float colorIndex = texture(rteTexture, textureUV).r;
    vec4 pal = textureAA(rtePalette, vec2(colorIndex, 0.5));
    if (drawMasked && pal.a < 0.01) discard;
    vec4 col = pal * rteColor * vertexColor;
    if (rteBlendInvert) col.rgb = vec3(1.0) - col.rgb;
    FragColor = col;
}
