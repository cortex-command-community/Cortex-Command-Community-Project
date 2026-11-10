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

void main() {
	if (noise(gl_FragCoord.xy + textureUV) < 0.5) {
		discard;
	}
	float red = texture(rteTexture, textureUV).r;
	// Palette entries must be fetched exactly; smoothing the lookup coordinate let neighbouring entries bleed through (e.g. dark index 100 drawn as near-white 99).
	FragColor = texelFetch(rtePalette, ivec2(int(red * vertexColor.r * 255.0 + 0.5), 0), 0) * vec4(rteColor.rgb, rteColor.a * vertexColor.a);
}