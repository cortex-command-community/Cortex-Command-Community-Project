// Blit8.frag
#version 330 core

in vec2 textureUV;
in vec4 vertexColor;
out vec4 FragColor;

uniform sampler2D rteTexture;
uniform sampler2D rtePalette;
uniform bool rteIndexed;
uniform vec4 rteColor;
uniform bool rteReplaceColor;

vec4 textureAA(sampler2D tex, vec2 uv) {
	vec2 texsize = vec2(textureSize(tex, 0));
	vec2 uv_texspace = uv * texsize;
	vec2 seam = floor(uv_texspace + .5);
	uv_texspace = (uv_texspace - seam) / fwidth(uv_texspace) + seam;
	uv_texspace = clamp(uv_texspace, seam - .5, seam + .5);
	return texture(tex, uv_texspace / texsize);
}

void main() {
	if (rteIndexed) {
		float colorIndex = texture(rteTexture, vec2(textureUV.x, textureUV.y)).r;
		FragColor = texture(rtePalette, vec2(colorIndex, 0.0F)) * vertexColor;
	} else {
		FragColor = textureAA(rteTexture, textureUV) * vertexColor;
	}
	if (FragColor.a == 0.0) {
		discard;
	} else if (rteReplaceColor) {
		FragColor.rgba = rteColor;
	}
}
