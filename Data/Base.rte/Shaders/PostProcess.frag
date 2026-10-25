#version 330 core

in vec2 textureUV;
in vec4 vertexColor;

out vec4 FragColor;

uniform sampler2D rteTexture;
uniform vec4 rteColor;


vec4 textureAA(sampler2D tex, vec2 uv) {
	vec2 texsize = vec2(textureSize(tex, 0));
	vec2 uv_texspace = uv * texsize;
	vec2 seam = floor(uv_texspace + .5);
	uv_texspace = (uv_texspace - seam) / fwidth(uv_texspace) + seam;
	uv_texspace = clamp(uv_texspace, seam - .5, seam + .5);
	return texture(tex, uv_texspace / texsize);
}

void main() {
	FragColor = textureAA(rteTexture, textureUV) * rteColor * vertexColor;
}
