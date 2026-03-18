#version 300 es
// Blit8.es.frag — GLSL ES 3.00 version of Blit8.frag for WebGL 2
//
// Reads an 8bpp indexed-colour texture (R channel = palette index 0-255),
// then looks up the RGBA colour from a 256×1 palette texture.
//
// This is the primary scene-buffer upload path:
//   CPU writes 8bpp pixels → GL_R8 texture → this shader → canvas

precision mediump float;

in  vec2 textureUV;
out vec4 FragColor;

uniform sampler2D rteTexture;   // GL_R8 — one byte per pixel, value = palette index
uniform sampler2D rtePalette;   // 256x1 RGBA — palette LUT

void main() {
    // Y-flip: Allegro bitmaps are top-down, OpenGL textures are bottom-up.
    float colorIndex = texture(rteTexture, vec2(textureUV.x, 1.0 - textureUV.y)).r;
    FragColor = texture(rtePalette, vec2(colorIndex, 0.5));
}
