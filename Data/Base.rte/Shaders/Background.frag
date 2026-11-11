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
uniform bool treatUnseenAsNeverSeen;
uniform bool fowEnabled;
uniform bool useOneStageFoW = false;

uniform vec2 uViewOrigin;
uniform vec2 uViewSize;
uniform vec2 uSceneSize;
uniform float uSdfMargin; // How far past each edge of the screen the fog of war distance textures cover, in pixels

uniform float uNoiseSeed;

uniform float usdfThresoldForFoV;
uniform float usdfOpacitySmoothingDistanceForFoV;
uniform float usdfThresoldForNeverSeen;
uniform float usdfOpacitySmoothingDistanceForNeverSeen;
uniform float scanlineAndNoiseOpacitySmoothingDistance;
uniform float liveTerrainFractionOfFoVSmoothing; // How far across the smoothing past what's in view the live terrain is drawn, as a fraction of its smoothing distance
uniform float unseenNoiseIntensity;

uniform sampler2D rteTextureLastSeen;
uniform sampler2D fowMaskTexture;
uniform sampler2D guiTexture;
uniform sampler2D moColor;
uniform sampler2D bgTerrainTex;
uniform sampler2D fgTerrainTex;
uniform sampler2D fowLastSeenMaskTexture;
uniform sampler2D lastSeenTerrainCopiedMask; // Which pixels of the last-seen terrain have had terrain copied into them, at fog of war resolution
uniform sampler2D bgLayersTexture;

const vec3 luminosityFactors = vec3(0.299, 0.587, 0.114);

// Palette entries must be fetched exactly; smoothing the lookup coordinate let neighbouring entries bleed through (e.g. dark index 100 drawn as near-white 99).
vec4 paletteColor(float index) {
	return texelFetch(rtePalette, ivec2(int(index * 255.0 + 0.5), 0), 0);
}

vec4 ApplyPalette(float red) {
    vec4 col = paletteColor(red * vertexColor.r)
		* vec4(rteColor.rgb, rteColor.a * vertexColor.a);

    if (rteBlendInvert) {
		return vec4(1.0, 1.0, 1.0, 0.0) - vec4(col.rgb, -col.a);
	}

    return col;
}

float rand(vec2 co) {
    return fract(sin(dot(co, vec2(12.9898,78.233))) * 43758.5453);
}

vec4 ApplyScanlineAndNoise(vec4 pix, vec2 uv, float scanlinePhaseOffset, float opacity) {
	float texelX = floor(uv.x * uSceneSize.x);
	float texelY = floor(uv.y * uSceneSize.y);
	float scan = sin(texelY * 3.14159 / 2 + scanlinePhaseOffset) * 0.01;
	vec2 quantizedXY = vec2(texelX, texelY);
	float noise = (rand(quantizedXY + uNoiseSeed) * 2.0 - 1.0) * unseenNoiseIntensity;

	vec3 color = pix.rgb;
	color += vec3(noise + scan, noise + scan, noise + scan) * opacity;

	return vec4(color, 1.0);
} 

vec4 ApplyDarkenAndDesat(vec4 pix, vec2 uv, float darken, float desat) {
	float lum = dot(pix.rgb, luminosityFactors);
	vec3 grayColor = vec3(lum);
	
	// 0 = original, 1 = full grayscale
	vec3 color = mix(pix.rgb, grayColor, desat);
	
	// 1 = unchanged, <1 darker
	color *= darken;
	
	return vec4(color, 1.0);
}

void main() {
	const float PALETTE_COLOR_BLACK = 245.0 / 255.0;
	const float PALETTE_COLOR_MASK = 0.0;
	//const float USDF_THRESHOLD_UNDER_WHICH_IT_IS_GROUND = 0.005;

	// Three values for low/medium/high smoothing (this'll become an option)
	//const float USDF_OPACITY_SMOOTHING_DISTANCE = 0.004; // low, just barely enough to hide the jaggies
	//const float USDF_OPACITY_SMOOTHING_DISTANCE = 0.014; // standard setting, just enough to hide the grid
	//const float USDF_OPACITY_SMOOTHING_DISTANCE = 0.020; // super smooth

	//const float SCANLINE_OPACITY_SMOOTHING_DISTANCE = 0.05f;

	// The distance textures cover a margin past each edge of the screen, so this screen pixel is further in
	vec2 sdfUV = (textureUV * uViewSize + uSdfMargin) / (uViewSize + 2.0 * uSdfMargin);
	float distanceToFoV = texture(fowMaskTexture, sdfUV).r - usdfThresoldForFoV;
	float distanceToSeenBefore = texture(fowLastSeenMaskTexture, sdfUV).r - usdfThresoldForNeverSeen;
	
	bool hasBeenSeen = distanceToSeenBefore < 0;

	vec2 sceneUV = (uViewOrigin + textureUV * uViewSize) / uSceneSize;
	
	float guiVal = texture(guiTexture, textureUV).r;
	bool fragmentIsGui = guiVal != PALETTE_COLOR_MASK;
	float moVal = texture(moColor, textureUV).r;
	float bgTerrainVal = texture(bgTerrainTex, textureUV).r;
	float fgTerrainVal = texture(fgTerrainTex, textureUV).r;
	float rteVal = texture(rteTexture, textureUV).r;
	vec2 textureUV_vFlipped = vec2(textureUV.x, 1.0 - textureUV.y);
	float bgLayersVal = texture(bgLayersTexture, textureUV_vFlipped).r;
	
	if (!drawingForeground) {
		if (rteVal == 0.0 && drawMasked) {
			discard;
		}
		FragColor = vec4(rteVal, 0.0, 0.0, 1.0);
		return;
	}
	
	if (fragmentIsGui) {
		FragColor = ApplyPalette(guiVal);
		return;
	}

	// With fog of war, terrain is live across part of the smoothing band past what's in view, and what was last seen beyond that.
	// Everything drawn live is copied into the last-seen terrain as it goes out of view. Anywhere it hasn't been copied yet stays live, and the never-seen darkening fades it to black.
	bool hasLastSeenTerrain = texture(lastSeenTerrainCopiedMask, sceneUV).r < 0.5;
	bool showLiveTerrain = useOneStageFoW || !fowEnabled || !hasLastSeenTerrain || distanceToFoV < usdfOpacitySmoothingDistanceForFoV * liveTerrainFractionOfFoVSmoothing;
	bool showingForegroundTerrain;
	if (showLiveTerrain) {
		showingForegroundTerrain = fgTerrainVal != PALETTE_COLOR_MASK;
		if (showingForegroundTerrain) {
			// Terrain
			FragColor = ApplyPalette(fgTerrainVal);
		} else if (bgTerrainVal != PALETTE_COLOR_MASK) {
			// Background terrain
			FragColor = ApplyPalette(bgTerrainVal);
		} else {
			// Background layers
			FragColor = ApplyPalette(bgLayersVal);
		}
	} else {
		// Seen before, old terrain
		float lastSeenTerrainColor = texture(rteTextureLastSeen, sceneUV).r;
		showingForegroundTerrain = !(lastSeenTerrainColor == PALETTE_COLOR_MASK && drawMasked);
		if (!showingForegroundTerrain) {
			// Background terrain
			if (bgTerrainVal != PALETTE_COLOR_MASK) {
				FragColor = ApplyPalette(bgTerrainVal);
			} else {
				FragColor = ApplyPalette(bgLayersVal);
			}
		} else {
			FragColor = ApplyPalette(lastSeenTerrainColor);
		}
	}

	// MOs are behind foreground terrain, and fade out across the smoothing band past what's in view
	if (!showingForegroundTerrain && moVal != PALETTE_COLOR_MASK) {
		float moVisibility = (useOneStageFoW || !fowEnabled) ? 1.0 : 1 - clamp(distanceToFoV / usdfOpacitySmoothingDistanceForFoV, 0, 1);
		FragColor = mix(FragColor, ApplyPalette(moVal), moVisibility);
	}

	if (fowEnabled) {
		float scanlineAndNoiseLerp = clamp(distanceToFoV / scanlineAndNoiseOpacitySmoothingDistance, 0, 1);
		FragColor = ApplyScanlineAndNoise(FragColor, sceneUV, 0.1, scanlineAndNoiseLerp);

		float unseenLerp = 1 - clamp(distanceToFoV / usdfOpacitySmoothingDistanceForFoV, 0, 1);
		float darken = mix(0.68, 1, unseenLerp);
		float desat = mix(0.8, 0, unseenLerp);

		float neverSeenLerp = 1 - clamp(distanceToSeenBefore / usdfOpacitySmoothingDistanceForNeverSeen, 0, 1);
		if (treatUnseenAsNeverSeen) {
			neverSeenLerp = min(neverSeenLerp, unseenLerp);
		}

		darken = min(darken, mix(0, 1, neverSeenLerp));

		FragColor = ApplyDarkenAndDesat(FragColor, sceneUV, darken, desat);
	}
}
