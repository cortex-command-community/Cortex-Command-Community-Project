#version 330 core

in vec2 textureUV;
in vec4 vertexColor;

out vec4 FragColor;

uniform sampler2D rteTexture;
uniform sampler2D rtePalette;
uniform vec4 rteColor;

// Pseudo random number generator. 
float hash( vec2 a )
{
    return fract( sin( a.x * 3433.8 + a.y * 3843.98 ) * 45933.8 );
}

// Value noise courtesy of BigWingz 
// check his youtube channel he has
// a video of this one.
// Succint version by FabriceNeyret
float noise( vec2 U )
{
	vec2 id = floor( U );
	U = fract( U );
	U *= U * ( 3. - 2. * U );  

	vec2 A = vec2( hash(id), hash(id + vec2(0,1)) );
	vec2 B = vec2( hash(id + vec2(1,0)), hash(id + vec2(1,1)) );
	vec2 C = mix( A, B, U.x);

	return mix( C.x, C.y, U.y );
}

vec4 textureAA(sampler2D tex, vec2 uv) {
	vec2 texsize = vec2(textureSize(tex, 0));
	vec2 uv_texspace = uv * texsize;
	vec2 seam = floor(uv_texspace + .5);
	uv_texspace = (uv_texspace - seam) / fwidth(uv_texspace) + seam;
	uv_texspace = clamp(uv_texspace, seam - .5, seam + .5);
	return texture(tex, uv_texspace / texsize);
}

void main() {
	if (noise(gl_FragCoord.xy + textureUV) < 0.5) {
		discard;
	}
	float red = texture(rteTexture, textureUV).r;
	vec4 color = textureAA(rtePalette, vec2(red * vertexColor.r, 0.0)) * vec4(rteColor.rgb, rteColor.a * vertexColor.a);

	FragColor = color;
}