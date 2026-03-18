#version 300 es
// ScreenBlit.es.frag — GLSL ES 3.00 version of ScreenBlit.frag for WebGL 2
//
// Composites the GUI overlay (rteGUITexture) on top of the scene (rteTexture).
// Uses a soft anti-aliased texture sampling helper and alpha/luma blending.

precision mediump float;

in  vec2 textureUV;
out vec4 FragColor;

uniform sampler2D rteTexture;      // 32bpp post-processed scene
uniform sampler2D rteGUITexture;   // 32bpp GUI overlay

// Smooth sub-pixel edge anti-aliasing for upscaled pixel art.
// Equivalent to the textureAA() in the desktop version, ported to ES3.
vec4 textureAA(sampler2D tex, vec2 uv) {
    vec2 texsize    = vec2(textureSize(tex, 0));
    vec2 uv_ts      = uv * texsize;
    vec2 seam       = floor(uv_ts + 0.5);
    vec2 dv         = fwidth(uv_ts);         // dFdx + dFdy — ES3.00 supports fwidth
    uv_ts           = (uv_ts - seam) / max(dv, vec2(0.001)) + seam;
    uv_ts           = clamp(uv_ts, seam - 0.5, seam + 0.5);
    return texture(tex, uv_ts / texsize);
}

void main() {
    // Y-flip for scene texture (bottom-up GL convention)
    vec4 guiColor  = textureAA(rteGUITexture, vec2(textureUV.x, 1.0 - textureUV.y));
    float guiSolid = float((guiColor.r + guiColor.g + guiColor.b) > 0.0);
    float blend    = max(guiColor.a, guiSolid);
    FragColor      = mix(textureAA(rteTexture, textureUV), guiColor, blend);
}
