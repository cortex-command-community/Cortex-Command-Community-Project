#version 330 core
#extension GL_KHR_blend_equation_advanced: enable
#extension GL_ARB_sample_shading: enable

in vec2 textureUV;
in vec4 vertexColor;

#ifdef GL_KHR_blend_equation_advanced
layout(blend_support_all_equations) out;
#endif
out vec4 FragColor;
uniform sampler2D rteTexture;
uniform sampler2D rtePalette;
uniform vec4 rteColor = vec4(1.0);
uniform bool rteBlendInvert = false;
uniform bool drawMasked = false;


// Palette entries must be fetched exactly; smoothing the lookup coordinate let neighbouring entries bleed through (e.g. dark index 100 drawn as near-white 99).
vec4 paletteColor(float index) {
	return texelFetch(rtePalette, ivec2(int(index * 255.0 + 0.5), 0), 0);
}

void main() {
	float red = texture(rteTexture, textureUV).r;
	if (red==0 && drawMasked) {
		discard;
	}
	if (!rteBlendInvert) {
		FragColor = paletteColor(red * vertexColor.r) * vec4(rteColor.rgb, rteColor.a * vertexColor.a);
	} else {
		FragColor = vec4(vec3(1.0), 0.0) - (paletteColor(red * vertexColor.r) * vec4(rteColor.rgb, -rteColor.a * vertexColor.a));
	}
}
