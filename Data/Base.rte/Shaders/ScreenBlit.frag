#version 330 core

in vec2 textureUV;

out vec4 FragColor;

uniform sampler2D rteTexture;
uniform sampler2D rteGUITexture;

vec4 textureAA(sampler2D tex, vec2 uv) {
	vec2 texsize = vec2(textureSize(tex, 0));
	vec2 uv_texspace = uv * texsize;
	vec2 seam = floor(uv_texspace + .5);
	uv_texspace = (uv_texspace - seam) / fwidth(uv_texspace) + seam;
	uv_texspace = clamp(uv_texspace, seam - .5, seam + .5);
	return texture(tex, uv_texspace / texsize);
}

void main() {
	vec4 guiColor = textureAA(rteGUITexture, vec2(textureUV.x, -textureUV.y));
	float guiSolid = float((guiColor.r + guiColor.g + guiColor.b) > 0.0);
	float blendRatio = max(guiColor.a, guiSolid);
	FragColor = (textureAA(rteTexture, textureUV) * (1.0F - blendRatio)) + guiColor * blendRatio;
}
