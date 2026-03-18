#version 300 es
// Dissolve.es.frag — GLSL ES 3.00 for WebGL 2
precision mediump float;

in  vec2 textureUV;
in  vec4 vertexColor;
out vec4 FragColor;

uniform sampler2D rteTexture;
uniform sampler2D rtePalette;
uniform vec4      rteColor;

float hash(vec2 a) {
    return fract(sin(a.x * 3433.8 + a.y * 3843.98) * 45933.8);
}

float noise(vec2 U) {
    vec2 id = floor(U);
    U = fract(U);
    U *= U * (3.0 - 2.0 * U);
    vec2 A = vec2(hash(id),             hash(id + vec2(0.0, 1.0)));
    vec2 B = vec2(hash(id + vec2(1.0, 0.0)), hash(id + vec2(1.0, 1.0)));
    vec2 C = mix(A, B, U.x);
    return mix(C.x, C.y, U.y);
}

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
    if (noise(gl_FragCoord.xy + textureUV) < 0.5) {
        discard;
    }
    float red = texture(rteTexture, textureUV).r;
    vec4 color = textureAA(rtePalette, vec2(red * vertexColor.r, 0.0))
               * vec4(rteColor.rgb, rteColor.a * vertexColor.a);
    FragColor = color;
}
