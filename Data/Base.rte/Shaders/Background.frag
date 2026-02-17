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
uniform bool drawingForeground = false;

uniform vec2 uViewOrigin;
uniform vec2 uViewSize;
uniform vec2 uSceneSize;

uniform float uNoiseSeed;

uniform sampler2D rteTextureLastSeen;
uniform sampler2D fowMaskTexture;
uniform sampler2D guiTexture;
uniform sampler2D moColor;
uniform sampler2D bgTerrainTex;
uniform sampler2D fgTerrainTex;
uniform sampler2D fowLastSeenMaskTexture;

const vec3 luminosityFactors = vec3(0.299, 0.587, 0.114);

vec4 textureAA(sampler2D tex, vec2 uv) {
	vec2 texsize = vec2(textureSize(tex, 0));
	vec2 uv_texspace = uv * texsize;
	vec2 seam = floor(uv_texspace + .5);
	uv_texspace = (uv_texspace - seam) / fwidth(uv_texspace) + seam;
	uv_texspace = clamp(uv_texspace, seam - .5, seam + .5);
	return texture(tex, uv_texspace / texsize);
}

vec4 ApplyPalette(float red) {
    vec4 col = textureAA(rtePalette, vec2(red * vertexColor.r, 0.0))
		* vec4(rteColor.rgb, rteColor.a * vertexColor.a);

    if (rteBlendInvert) {
		return vec4(1.0, 1.0, 1.0, 0.0) - vec4(col.rgb, -col.a);
	}

    return col;
}


float rand(vec2 co) {
    return fract(sin(dot(co, vec2(12.9898,78.233))) * 43758.5453);
}
	
vec4 ApplyPaletteAndDesat(float red, vec2 uv, float desat, float darken) {
    vec4 col = ApplyPalette(red);
	
	float texelY = uv.y * uSceneSize.y;
	float scan = sin(texelY	 * 3.14159 / 2) * 0.03;
	float noise = (rand(gl_FragCoord.xy + uNoiseSeed) * 2.0 - 1.0) * 0.025;
	
	float lum = dot(col.rgb, luminosityFactors);
	vec3 grayColor = vec3(lum);
	
	// 0 = original, 1 = full grayscale
	vec3 color = mix(col.rgb, grayColor, desat);
	
	// 1 = unchanged, <1 darker
	color *= darken;
	
	float brightness = 1.0 + noise + scan;
	color *= brightness;
	
	return vec4(color, 1.0);
}

void main() {
	
	const float PALETTE_COLOR_BLACK = 245.0 / 255.0;
	const float PALETTE_COLOR_MASK = 0.0;
	const float USDF_THRESHOLD_UNDER_WHICH_IT_IS_GROUND = 0.011;
	
	bool fragmentNotInFow = (texture(fowMaskTexture, textureUV).r > USDF_THRESHOLD_UNDER_WHICH_IT_IS_GROUND);
	
	vec2 sceneUV = (uViewOrigin + textureUV * uViewSize) / uSceneSize;
	
	//bool fragmentNotInFow = texture(fowMaskTexture, sceneUV).r == PALETTE_COLOR_MASK;
	float guiVal = texture(guiTexture, textureUV).r;
	bool fragmentIsGui = guiVal != PALETTE_COLOR_MASK;
	float moVal = texture(moColor, textureUV).r;
	float bgTerrainVal = texture(bgTerrainTex, textureUV).r;
	float fgTerrainVal = texture(fgTerrainTex, textureUV).r;
	float rteVal = texture(rteTexture, textureUV).r;
	
	if (!drawingForeground) {
		if (rteVal == 0.0 && drawMasked) {
			discard;
		}
		FragColor = ApplyPalette(rteVal);
		return;
	}
	
	if (fragmentIsGui) {
		FragColor = ApplyPalette(guiVal);
		return;
	}

	// Non-hidden pixel
	if (fragmentNotInFow) {
		// Terrain
		if (fgTerrainVal != PALETTE_COLOR_MASK) {
			FragColor = ApplyPalette(fgTerrainVal);
			return;
		}
		// MOs
		if (moVal != PALETTE_COLOR_MASK) {
			FragColor = ApplyPalette(moVal);
			return;
		}
		// Background terrain
		if (bgTerrainVal != PALETTE_COLOR_MASK) {
			FragColor = ApplyPalette(bgTerrainVal);
			return;
		}
		// Bkgr layers, here was drawn in a previous pass
		// big TODO
		discard;
	}
	// Fog-of-war pixel
	else {
		bool fragmentWasLastSeen = (texture(fowLastSeenMaskTexture, textureUV).r > USDF_THRESHOLD_UNDER_WHICH_IT_IS_GROUND);
		
		if (!fragmentWasLastSeen) {
			FragColor = vec4(0.0, 0.0, 0.0, 1.0);
			return;
		}
		
		float lastSeenTerrainColor = texture(rteTextureLastSeen, sceneUV).r;
		
		if (lastSeenTerrainColor == PALETTE_COLOR_MASK && drawMasked) {
			// Background terrain
			if (bgTerrainVal != PALETTE_COLOR_MASK) {
				FragColor = ApplyPaletteAndDesat(bgTerrainVal, textureUV, 0.5, 0.68);
				return;
			}
			discard;
		}
		
		// Palette lookup
		FragColor = ApplyPaletteAndDesat(lastSeenTerrainColor, sceneUV, 0.8, 0.8);
		return;
	}
}
