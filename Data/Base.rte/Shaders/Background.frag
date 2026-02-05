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


vec4 textureAA(sampler2D tex, vec2 uv) {
	vec2 texsize = vec2(textureSize(tex, 0));
	vec2 uv_texspace = uv * texsize;
	vec2 seam = floor(uv_texspace + .5);
	uv_texspace = (uv_texspace - seam) / fwidth(uv_texspace) + seam;
	uv_texspace = clamp(uv_texspace, seam - .5, seam + .5);
	return texture(tex, uv_texspace / texsize);
}

void main() {
	float red = texture(rteTexture, textureUV).r;
	if (red==0 && drawMasked) {
		discard;
	}
	if (!rteBlendInvert) {
		FragColor = textureAA(rtePalette, vec2(red * vertexColor.r, 0.0)) * vec4(rteColor.rgb, rteColor.a * vertexColor.a);
	} else {
		FragColor = vec4(vec3(1.0), 0.0) - (textureAA(rtePalette, vec2(red * vertexColor.r, 0.0)) * vec4(rteColor.rgb, -rteColor.a * vertexColor.a));
	}
}
