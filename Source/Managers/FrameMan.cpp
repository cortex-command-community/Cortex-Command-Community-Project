#include "FrameMan.h"

#include "SDL3/SDL_surface.h"
#include "WindowMan.h"
#include "PostProcessMan.h"
#include "PresetMan.h"
#include "PrimitiveMan.h"
#include "PerformanceMan.h"
#include "ActivityMan.h"
#include "CameraMan.h"
#include "ConsoleMan.h"
#include "SettingsMan.h"
#include "UInputMan.h"
#include "GLResourceMan.h"

#include "SLTerrain.h"
#include "SLBackground.h"
#include "Scene.h"
#include "System.h"

#include "RenderTarget.h"

#include "GUI.h"
#include "AllegroBitmap.h"
#include "AllegroScreen.h"

#include "GLCheck.h"
#include "glad/gl.h"
#include "Draw.h"

#include "tracy/Tracy.hpp"
#include "tracy/TracyOpenGL.hpp"
#include <SDL3_image/SDL_image.h>

// SIMDe lets SSE2 code build on every platform, as NEON on ARM. Once std::simd is available on all our compilers, that can replace it.
#include "simde/x86/sse2.h"

#include <array>

using namespace RTE;

namespace {
	// How far the fog of war edge smoothing reaches past what's in view or seen before, as fractions of the screen's diagonal
	float usdfThresoldForFoV = 0.0095f;
	float usdfOpacitySmoothingDistanceForFoV = 0.01f;
	float usdfThresoldForNeverSeen = 0.0095f;
	float usdfOpacitySmoothingDistanceForNeverSeen = 0.01f;
	float scanlineAndNoiseOpacitySmoothingDistance = 0.015f;
	// How far across the smoothing past what's in view the live terrain is drawn, as a fraction of its smoothing distance. Last-seen terrain is drawn past that.
	float liveTerrainFractionOfFoVSmoothing = 1.0f;

	/// Gets the length of a screen's diagonal, which the fog of war edge smoothing measures its distances against.
	float GetScreenDiagonal(const BITMAP* screen) { return std::sqrt(static_cast<float>(screen->w * screen->w + screen->h * screen->h)); }

	/// Gets how far past each edge of a screen the fog of war edge smoothing has to look, so that what's just off screen still counts.
	/// @return The furthest the smoothing reaches, in pixels.
	int GetFogOfWarSdfMargin(const BITMAP* screen) {
		// Darkening and desaturating reach a smoothing distance past their threshold, and the scanlines and noise reach their own distance past the in view threshold
		const float furthestReach = std::max({usdfThresoldForFoV + usdfOpacitySmoothingDistanceForFoV, usdfThresoldForNeverSeen + usdfOpacitySmoothingDistanceForNeverSeen, usdfThresoldForFoV + scanlineAndNoiseOpacitySmoothingDistance});
		return static_cast<int>(std::ceil(furthestReach * GetScreenDiagonal(screen)));
	}

	/// Makes a bitmap the given size, keeping the one it has if it already is. Whatever was drawn on a kept bitmap stays, so clear it if that matters.
	void SizeBitmap(AllegroBitmap& bitmap, int width, int height) {
		if (!bitmap.GetBitmap() || bitmap.GetWidth() != width || bitmap.GetHeight() != height) {
			bitmap.Destroy();
			bitmap.Create(width, height, 8);
		}
	}
}

int FrameMan::GetFogOfWarNeverSeenReach() const {
	return m_BackBuffer8 ? static_cast<int>(std::ceil((usdfThresoldForNeverSeen + usdfOpacitySmoothingDistanceForNeverSeen) * GetScreenDiagonal(m_BackBuffer8.get()))) : 0;
}

int FrameMan::GetFogOfWarLiveTerrainReach() const {
	return m_BackBuffer8 ? static_cast<int>(std::ceil((usdfThresoldForFoV + liveTerrainFractionOfFoVSmoothing * usdfOpacitySmoothingDistanceForFoV) * GetScreenDiagonal(m_BackBuffer8.get()))) : 0;
}

void BitmapDeleter::operator()(BITMAP* bitmap) const { destroy_bitmap(bitmap); }
void SurfaceDeleter::operator()(SDL_Surface* surface) const { SDL_DestroySurface(surface); }

const std::array<std::function<void(int r, int g, int b, int a)>, DrawBlendMode::BlendModeCount> FrameMan::c_BlenderSetterFunctions = {
    nullptr, // NoBlend obviously has no blender, but we want to keep the indices matching with the enum.
    &set_burn_blender,
    &set_color_blender,
    &set_difference_blender,
    &set_dissolve_blender,
    &set_dodge_blender,
    &set_invert_blender,
    &set_luminance_blender,
    &set_multiply_blender,
    &set_saturation_blender,
    &set_screen_blender,
    nullptr // Transparency does not rely on the blender setting, it creates a map with the dedicated function instead of with the generic one.
};

FrameMan::FrameMan() {
	Clear();
}

FrameMan::~FrameMan() {
	Destroy();
}

void FrameMan::Clear() {
	m_HSplit = false;
	m_VSplit = false;
	m_TwoPlayerVSplit = false;
	m_PlayerScreen.reset();
	m_PlayerScreenWidth = 0;
	m_PlayerScreenHeight = 0;
	m_ScreenDumpBuffer.reset();
	m_WorldDumpBuffer.reset();
	m_ScenePreviewDumpGradient.reset();
	m_ScreenDumpNamePlaceholder.reset();
	m_BackBuffer8.reset();
	m_BackBuffer8GUI.reset();
	m_BackBuffer32.reset();
	m_OverlayBitmap32.reset();
	m_PaletteFile = ContentFile("Base.rte/palette.bmp");
	m_BlackColor = 245;
	m_AlmostBlackColor = 245;
	m_ColorTablePruneTimer.Reset();
	m_GUIScreens.fill(nullptr);
	m_LargeFonts.fill(nullptr);
	m_SmallFonts.fill(nullptr);
	m_TextBlinkTimer.Reset();

	for (int screenCount = 0; screenCount < c_MaxScreenCount; ++screenCount) {
		m_ScreenText[screenCount].clear();
		m_TextDuration[screenCount] = -1;
		m_TextDurationTimer[screenCount].Reset();
		m_TextBlinking[screenCount] = 0;
		m_TextCentered[screenCount] = false;
		m_HUDDisabled[screenCount] = false;
		m_FlashScreenColor[screenCount] = -1;
		m_FlashedLastFrame[screenCount] = false;
		m_FlashTimer[screenCount].Reset();
	}
}

void FrameMan::RenderBackgroundLayersBmToTexture() {
	Shader shaderPassthrough;
	g_PresetMan.GetEntityPreset("Shader", "Passthrough")->Clone(&shaderPassthrough);
	GLuint programPassthrough = shaderPassthrough.m_ProgramID;
	const Texture2D& backLayersTex = m_BackBuffer.get()->GetColorTexture();

	glBindFramebuffer(GL_FRAMEBUFFER, m_SdfFbo);

	glFramebufferTexture2D(GL_FRAMEBUFFER,
	                       GL_COLOR_ATTACHMENT0,
	                       GL_TEXTURE_2D,
	                       m_bgLayersTex, 0);

	GLenum buf = GL_COLOR_ATTACHMENT0;
	glDrawBuffers(1, &buf);
	glViewport(0, 0, backLayersTex.width, backLayersTex.height);
	glUseProgram(programPassthrough);

	GLint loc = glGetUniformLocation(programPassthrough, "texToPassthrough");
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, backLayersTex.id);
	glUniform1i(loc, 0);

	glBindVertexArray(m_SdfVao);
	glDrawArrays(GL_TRIANGLES, 0, 3);
}

void FrameMan::FogOfWarSetup(Shader& backgroundShader) {
	// The final pass draws the whole screen from these, whether there is any fog of war or not
	{ // GUI
		BITMAP* bm = m_BackBuffer8GUI.get();

		if (GUITex.id == 0) {
			LoadTextureFromBitmap8(&GUITex, bm);
		} else {
			rlUpdateTexture(GUITex.id, 0, 0, bm->w, bm->h, GUITex.format, bm->line[0]);
		}
	}

	{ // MOColor
		BITMAP* bm = MOColorBM.GetBitmap();

		if (MOColorTex.id == 0) {
			LoadTextureFromBitmap8(&MOColorTex, bm);
		} else {
			rlUpdateTexture(MOColorTex.id, 0, 0, bm->w, bm->h, MOColorTex.format, bm->line[0]);
		}
	}

	{ // BgTerrain
		BITMAP* bm = BgTerrainBM.GetBitmap();

		if (BgTerrainTex.id == 0) {
			LoadTextureFromBitmap8(&BgTerrainTex, bm);
		} else {
			rlUpdateTexture(BgTerrainTex.id, 0, 0, bm->w, bm->h, BgTerrainTex.format, bm->line[0]);
		}
	}

	{ // FgTerrain
		BITMAP* bm = FgTerrainBM.GetBitmap();

		if (FgTerrainTex.id == 0) {
			LoadTextureFromBitmap8(&FgTerrainTex, bm);
		} else {
			rlUpdateTexture(FgTerrainTex.id, 0, 0, bm->w, bm->h, FgTerrainTex.format, bm->line[0]);
		}
	}

	Scene* currentScene = g_SceneMan.GetCurrentScene();
	if (!currentScene || !currentScene->GetUnseenLayerMask()) {
		return;
	}

	// TODO: HACK. Only the first player's team's fog of war is drawn, on every screen.
	int team = g_ActivityMan.GetActivity()->GetTeamOfPlayer(0);

	SceneLayer* maskSL = currentScene->GetUnseenLayerMask(team);
	SceneLayer* lastSeenTerrainMaskSL = currentScene->GetUnseenLayerTerrainMask(team);
	SceneLayer* lastSeenTerrainSL = currentScene->GetUnseenLayerTerrain(team);

	BITMAP* maskBitmap = maskSL->GetBitmap();
	// What's in view is worked out again every frame, into a bitmap that's kept for as long as the mask stays the same size
	SizeBitmap(fowMaskBM, maskBitmap->w, maskBitmap->h);

	// The mask holds how many more updates each pixel stays in view for, but drawing only needs to know which pixels are in view: 0 for in view and 255 for not
	BITMAP* fowMaskBitmap = fowMaskBM.GetBitmap();
	const int width = std::min(maskBitmap->w, fowMaskBitmap->w);
	for (int y = 0; y < std::min(maskBitmap->h, fowMaskBitmap->h); ++y) {
		const uint8_t* holds = &Scene::GetUnseenLayerMaskHold(maskBitmap, 0, y);
		uint8_t* inView = fowMaskBitmap->line[y];
		int x = 0;
		// 16 pixels at a time. Comparing with 0 gives 255 for each hold that's 0, and 0 for the rest.
		const simde__m128i zero = simde_mm_setzero_si128();
		for (; x + 16 <= width; x += 16) {
			const simde__m128i sixteenHolds = simde_mm_loadu_si128(reinterpret_cast<const simde__m128i*>(holds + x));
			simde_mm_storeu_si128(reinterpret_cast<simde__m128i*>(inView + x), simde_mm_cmpeq_epi8(sixteenHolds, zero));
		}
		for (; x < width; ++x) {
			inView[x] = holds[x] > 0 ? 0 : 255;
		}
	}

	// Update/make textures
	// GTODO: update this on scene reentry (other places too)
	// clues probably in ClearTextures()
	{ // Fog of war mask
		BITMAP* bm = fowMaskBM.GetBitmap();

		// This is for what is currently actively looked at, non-accumulated
		if (instantVisibleFowMaskTex.id == 0) {
			LoadTextureFromBitmap8(&instantVisibleFowMaskTex, bm);
		} else {
			rlUpdateTexture(instantVisibleFowMaskTex.id, 0, 0, bm->w, bm->h, instantVisibleFowMaskTex.format, bm->line[0]);
		}
	}
	
	{ // Fog of war last-seen terrain, straight from the scene's layer
		BITMAP* bm = lastSeenTerrainSL->GetBitmap();

		if (lastSeenTex.id == 0) {
			LoadTextureFromBitmap8(&lastSeenTex, bm);
		} else {
			rlUpdateTexture(lastSeenTex.id, 0, 0, bm->w, bm->h, lastSeenTex.format, bm->line[0]);
		}
	}

	{ // Fog of war last-seen terrain chunky 1-0 mask, straight from the scene's layer
		BITMAP* bm = lastSeenTerrainMaskSL->GetBitmap();

		if (fowMaskLastSeenTex.id == 0) {
			LoadTextureFromBitmap8(&fowMaskLastSeenTex, bm);
		} else {
			rlUpdateTexture(fowMaskLastSeenTex.id, 0, 0, bm->w, bm->h, fowMaskLastSeenTex.format, bm->line[0]);
		}
	}

	if (SceneLayer* lastSeenTerrainCopiedMaskSL = currentScene->GetUnseenLayerTerrainCopiedMask(team)) { // Which pixels of the last-seen terrain have been copied
		BITMAP* bm = lastSeenTerrainCopiedMaskSL->GetBitmap();

		if (lastSeenCopiedMaskTex.id == 0) {
			LoadTextureFromBitmap8(&lastSeenCopiedMaskTex, bm);
		} else {
			rlUpdateTexture(lastSeenCopiedMaskTex.id, 0, 0, bm->w, bm->h, lastSeenCopiedMaskTex.format, bm->line[0]);
		}
	}

}

// Chunky fog of war mask -> SDF!
void FrameMan::FogOfWarSetup_DoSDF(const GLuint inputTex, GLuint& outputTex) {
#define prnt(str) g_ConsoleMan.PrintString(std::to_string(str))
#define prnt2(str) g_ConsoleMan.PrintString(str)
	const float timeInSecs = (float)g_TimerMan.GetSimTickCount() / 1000000;
	// The passes cover the screen plus a margin past each edge, so that what's just off screen still counts
	const int sdfWidth = m_BackBuffer8->w + m_SdfMargin * 2;
	const int sdfHeight = m_BackBuffer8->h + m_SdfMargin * 2;
	const Vector sdfSize(static_cast<float>(sdfWidth), static_cast<float>(sdfHeight));

	// Only repeat the mask past the scene's edges where the scene wraps, otherwise filtering blends each edge of the scene with the opposite one
	glBindTexture(GL_TEXTURE_2D, inputTex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, g_SceneMan.SceneWrapsX() ? GL_REPEAT : GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, g_SceneMan.SceneWrapsY() ? GL_REPEAT : GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	Shader shaderMask, shaderJFA, shaderUnsignedSDF, shaderPassthrough;
	g_PresetMan.GetEntityPreset("Shader", "SDF_1_SeedTexture")->Clone(&shaderMask);
	g_PresetMan.GetEntityPreset("Shader", "SDF_2_JFA")->Clone(&shaderJFA);
	g_PresetMan.GetEntityPreset("Shader", "SDF_3_UnsignedSDF")->Clone(&shaderUnsignedSDF);
	g_PresetMan.GetEntityPreset("Shader", "Passthrough")->Clone(&shaderPassthrough);
	GLuint programMask = shaderMask.m_ProgramID;
	GLuint programJFA = shaderJFA.m_ProgramID;
	GLuint programUnsignedSDF = shaderUnsignedSDF.m_ProgramID;
	GLuint programPassthrough = shaderPassthrough.m_ProgramID;

		// 1. Mask!
	glBindFramebuffer(GL_FRAMEBUFFER, m_SdfFbo);
	glFramebufferTexture2D(GL_FRAMEBUFFER,
	                       GL_COLOR_ATTACHMENT0,
	                       GL_TEXTURE_2D,
	                       m_SdfTexPing, 0);

	GLenum buf = GL_COLOR_ATTACHMENT0;
	glDrawBuffers(1, &buf);
	glViewport(0, 0, sdfWidth, sdfHeight);
	glUseProgram(programMask);

	// Set uniforms
	SceneLayer* terrainSL = g_SceneMan.GetTerrain();
	shaderMask.SetFloat("uTime", timeInSecs);
	shaderMask.SetVector2f("uViewOrigin", g_CameraMan.GetOffset(0) - Vector(static_cast<float>(m_SdfMargin), static_cast<float>(m_SdfMargin)));
	shaderMask.SetVector2f("uViewSize", sdfSize);
	shaderMask.SetVector2f("uSceneSize", Vector(terrainSL->GetBitmap()->w, terrainSL->GetBitmap()->h));

	// Set Sampler2D
	GLint loc = glGetUniformLocation(programMask, "uMask");
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, inputTex);
	glUniform1i(loc, 0);

	// Finally draw!
	glBindVertexArray(m_SdfVao);
	glDrawArrays(GL_TRIANGLES, 0, 3);
	
	//2. JFA!
	GLuint src = m_SdfTexPing, dst = m_SdfTexPong;

	for (int step = std::ceil(std::max(sdfWidth, sdfHeight) / 2); step >= 1; step /= 2) {
		// Drawing to DST
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, dst, 0);

		glViewport(0, 0, sdfWidth, sdfHeight);
		glUseProgram(programJFA);
		
		// Setting sampler2d as SRC
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, src);
		glUniform1i(glGetUniformLocation(programJFA, "uPrev"), 0);

		shaderJFA.SetFloat("uStep", step);
		shaderJFA.SetVector2f("uViewSize", sdfSize);

		glDrawArrays(GL_TRIANGLES, 0, 3);

		std::swap(src, dst);
	}

	// 3. Unsigned SDF!
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, dst, 0);

	glViewport(0, 0, sdfWidth, sdfHeight);
	glUseProgram(programUnsignedSDF);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, src);
	glUniform1i(glGetUniformLocation(programUnsignedSDF, "uSampler"), 0);

	// Distances stay relative to the screen's diagonal, not the margin's, so the smoothing distances mean the same as before
	shaderUnsignedSDF.SetFloat("uMaxDist", GetScreenDiagonal(m_BackBuffer8.get()));
	shaderUnsignedSDF.SetVector2f("uViewSize", sdfSize);

	glDrawArrays(GL_TRIANGLES, 0, 3);

	// Result now in DST! 
	// Passthrough it to outputTex
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, outputTex, 0);

	glViewport(0, 0, sdfWidth, sdfHeight);
	glUseProgram(programPassthrough);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, dst);
	glUniform1i(glGetUniformLocation(programPassthrough, "texToPassthrough"), 0);

	glDrawArrays(GL_TRIANGLES, 0, 3);

	// Done, wrapup!
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void FrameMan::ClearFowTextures() {
	auto clearTextureIfNeeded = [](Texture2D& tex) {
		if (tex.id != 0) {
			rlUnloadTexture(tex.id);
			tex.id = 0;
		}
	};
	clearTextureIfNeeded(instantVisibleFowMaskTex);
	clearTextureIfNeeded(lastSeenTex);
	clearTextureIfNeeded(fowMaskLastSeenTex);
	clearTextureIfNeeded(lastSeenCopiedMaskTex);
	clearTextureIfNeeded(GUITex);
	clearTextureIfNeeded(MOColorTex);
	clearTextureIfNeeded(BgTerrainTex);
	clearTextureIfNeeded(FgTerrainTex);
}

void FrameMan::InitOrReinitFowOglThings() {
	g_ConsoleMan.PrintString("InitOrReinitFowOglThings() called!");

	static bool initHappened = false;

	// We create handles and buffers only once
	if (!initHappened) {
		initHappened = true;

		// Create the render textures
		glGenTextures(1, &m_SdfTexPing);
		glGenTextures(1, &m_SdfTexPong);
		glGenTextures(1, &m_SdfResultFowMask);
		glGenTextures(1, &m_SdfResultFowLastSeenTerrainMask);
		glGenTextures(1, &m_bgLayersTex);
		
		// Create framebuffer
		glGenFramebuffers(1, &m_SdfFbo);
		glBindFramebuffer(GL_FRAMEBUFFER, m_SdfFbo);

		float tri[] = {
		    -1.0f, -1.0f,
		    3.0f, -1.0f,
		    -1.0f, 3.0f};

		glGenVertexArrays(1, &m_SdfVao);
		glGenBuffers(1, &m_SdfVbo);

		glBindVertexArray(m_SdfVao);
		glBindBuffer(GL_ARRAY_BUFFER, m_SdfVbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(tri), tri, GL_STATIC_DRAW);

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), 0);
	} 
	
	// But make/remake textures with differing wrapping and sizes each time
	// Two floats per pixel, unfiltered and clamped to the edges
	auto alloc_rg32f = [&](GLuint tex, int width, int height) {
		glBindTexture(GL_TEXTURE_2D, tex);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RG32F, width, height, 0, GL_RG, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	};

	// The edge smoothing covers the screen plus however far it reaches past each edge, so that what's just off screen still counts
	m_SdfMargin = GetFogOfWarSdfMargin(m_BackBuffer8.get());
	const int sdfWidth = m_BackBuffer8->w + m_SdfMargin * 2;
	const int sdfHeight = m_BackBuffer8->h + m_SdfMargin * 2;
	alloc_rg32f(m_SdfTexPing, sdfWidth, sdfHeight);
	alloc_rg32f(m_SdfTexPong, sdfWidth, sdfHeight);
	alloc_rg32f(m_SdfResultFowMask, sdfWidth, sdfHeight);
	alloc_rg32f(m_SdfResultFowLastSeenTerrainMask, sdfWidth, sdfHeight);
	alloc_rg32f(m_bgLayersTex, m_BackBuffer8->w, m_BackBuffer8->h);
}

void FrameMan::BackgroundShaderSetUniforms(Shader& backgroundShader, bool fowEnabled) {
	rlSetUniformSampler(backgroundShader.GetUniformLocation("guiTexture"), GUITex.id);
	rlSetUniformSampler(backgroundShader.GetUniformLocation("hudLayerTexture"), m_HudLayer->GetColorTexture().id);
	rlSetUniformSampler(backgroundShader.GetUniformLocation("moColor"), MOColorTex.id);
	rlSetUniformSampler(backgroundShader.GetUniformLocation("bgTerrainTex"), BgTerrainTex.id);
	rlSetUniformSampler(backgroundShader.GetUniformLocation("fgTerrainTex"), FgTerrainTex.id);
	rlSetUniformSampler(backgroundShader.GetUniformLocation("bgLayersTexture"), m_bgLayersTex);

	//rlTexture
	SceneLayer* terrainSL = g_SceneMan.GetTerrain();
	backgroundShader.SetVector2f("uViewOrigin", g_CameraMan.GetOffset(0));
	backgroundShader.SetVector2f("uViewSize", Vector(m_BackBuffer8->w, m_BackBuffer8->h));
	backgroundShader.SetVector2f("uSceneSize", Vector(terrainSL->GetBitmap()->w, terrainSL->GetBitmap()->h));
	backgroundShader.SetFloat("uNoiseSeed", static_cast<float>(rand()) * 0.0001f);
	backgroundShader.SetFloat("uSdfMargin", static_cast<float>(m_SdfMargin));

	static float unseenNoiseIntensity = 0.02f;
	backgroundShader.SetFloat("usdfThresoldForFoV", usdfThresoldForFoV);
	backgroundShader.SetFloat("usdfOpacitySmoothingDistanceForFoV", usdfOpacitySmoothingDistanceForFoV);
	backgroundShader.SetFloat("usdfThresoldForNeverSeen", usdfThresoldForNeverSeen);
	backgroundShader.SetFloat("usdfOpacitySmoothingDistanceForNeverSeen", usdfOpacitySmoothingDistanceForNeverSeen);
	backgroundShader.SetFloat("scanlineAndNoiseOpacitySmoothingDistance", scanlineAndNoiseOpacitySmoothingDistance);
	backgroundShader.SetFloat("liveTerrainFractionOfFoVSmoothing", liveTerrainFractionOfFoVSmoothing);
	backgroundShader.SetFloat("unseenNoiseIntensity", unseenNoiseIntensity);

	static bool treatUnseenAsNeverSeen = false;
	backgroundShader.SetBool("treatUnseenAsNeverSeen", treatUnseenAsNeverSeen);
	backgroundShader.SetBool("fowEnabled", fowEnabled);

	if (fowEnabled) {
		rlSetUniformSampler(backgroundShader.GetUniformLocation("rteTextureLastSeen"), lastSeenTex.id); // GTODO: not just force first player screen
		rlSetUniformSampler(backgroundShader.GetUniformLocation("fowMaskTexture"), m_SdfResultFowMask); // GTODO toggle for smooth and not in settings
		rlSetUniformSampler(backgroundShader.GetUniformLocation("fowLastSeenMaskTexture"), m_SdfResultFowLastSeenTerrainMask);
		rlSetUniformSampler(backgroundShader.GetUniformLocation("lastSeenTerrainCopiedMask"), lastSeenCopiedMaskTex.id);
	}
}

int FrameMan::Initialize() {
	set_color_depth(c_BPP);
	// Sets the allowed color conversions when loading bitmaps from files
	set_color_conversion(COLORCONV_MOST);

	LoadPalette(m_PaletteFile.GetDataPath());

	// Store the default palette for re-use when creating new color tables for different blend modes because the palette can be changed via scripts, and handling per-palette per-mode color tables is too much headache.
	get_palette(m_DefaultPalette);

	CreatePresetColorTables();
	SetTransTableFromPreset(TransparencyPreset::HalfTrans);
	CreateBackBuffers();

	ContentFile scenePreviewGradientFile("Base.rte/GUIs/PreviewSkyGradient.png");
	m_ScenePreviewDumpGradient = std::unique_ptr<BITMAP, BitmapDeleter>(scenePreviewGradientFile.GetAsBitmap(COLORCONV_8_TO_32, false));

	m_ScreenDumpNamePlaceholder = std::unique_ptr<BITMAP, BitmapDeleter>(create_bitmap_ex(24, 1, 1));
	clear_bitmap(m_ScreenDumpNamePlaceholder.get());

	// Use fastest compression in save_png().
	_png_compression_level = 1;

	return 0;
}

int FrameMan::CreateBackBuffers() {
	int resX = g_WindowMan.GetResX();
	int resY = g_WindowMan.GetResY();

	// Create the back buffer, this is still in 8bpp, we will do any post-processing on the PostProcessing bitmap
	m_BackBuffer8 = std::unique_ptr<BITMAP, BitmapDeleter>(create_bitmap_ex(8, resX, resY));
	ClearBackBuffer8();
	// The GUI drawn on the CPU has its own back buffer, which the final pass draws over everything else
	m_BackBuffer8GUI = std::unique_ptr<BITMAP, BitmapDeleter>(create_bitmap_ex(8, resX, resY));
	ClearBackBuffer8GUI();

	// Create the post-processing buffer, it'll be used for glow effects etc
	m_BackBuffer32 = std::unique_ptr<BITMAP, BitmapDeleter>(create_bitmap_ex(c_BPP, resX, resY));
	ClearBackBuffer32();

	m_BackBuffer = std::make_unique<RenderTarget>(FloatRect(0, 0, resX, resY), FloatRect(0, 0, resX, resY), 32);
	m_HudLayer = std::make_unique<RenderTarget>(FloatRect(0, 0, resX, resY), FloatRect(0, 0, resX, resY), 32);

	m_OverlayBitmap32 = std::unique_ptr<BITMAP, BitmapDeleter>(create_bitmap_ex(c_BPP, resX, resY));
	clear_to_color(m_OverlayBitmap32.get(), 0);

	m_PlayerScreenWidth = m_BackBuffer8->w;
	m_PlayerScreenHeight = m_BackBuffer8->h;

	// Create the splitscreen buffer
	if (m_HSplit || m_VSplit) {
		m_PlayerScreen8 = std::unique_ptr<BITMAP, BitmapDeleter>(create_bitmap_ex(8, resX / (m_VSplit ? 2 : 1), resY / (m_HSplit ? 2 : 1)));
		clear_to_color(m_PlayerScreen8.get(), 0);
		set_clip_state(m_PlayerScreen8.get(), 1);

		m_PlayerScreen = std::make_unique<RenderTarget>(FloatRect(0, 0, resX / (m_VSplit ? 2 : 1), resY / (m_HSplit ? 2 : 1)), FloatRect(0, 0, resX / (m_VSplit ? 2 : 1), resY / (m_HSplit ? 2 : 1)));

		// Update these to represent the split screens
		m_PlayerScreenWidth = m_PlayerScreen->GetSize().w;
		m_PlayerScreenHeight = m_PlayerScreen->GetSize().h;
	} else {
		m_PlayerScreen8 = m_BackBuffer8;
		m_PlayerScreen = m_BackBuffer;
	}

	m_ScreenDumpBuffer = std::unique_ptr<SDL_Surface, SurfaceDeleter>(SDL_CreateSurface(m_BackBuffer8->w, m_BackBuffer8->h, SDL_PIXELFORMAT_RGB24));

	return 0;
}

void FrameMan::CreatePresetColorTables() {
	// Create RGB lookup table that supposedly speeds up calculation of other color tables.
	// create_rgb_table(&m_RGBTable, m_DefaultPalette, nullptr);
	// rgb_map = &m_RGBTable;

	// Create transparency color tables. Tables for other blend modes will be created on demand.
	int transparencyPresetCount = BlendAmountLimits::MaxBlend / c_BlendAmountStep;
	for (int index = 0; index <= transparencyPresetCount; ++index) {
		int presetBlendAmount = index * c_BlendAmountStep;
		std::array<int, 4> colorChannelBlendAmounts = {presetBlendAmount, presetBlendAmount, presetBlendAmount, BlendAmountLimits::MinBlend};
		int adjustedBlendAmount = 255 - (static_cast<int>(255.0F * (1.0F / static_cast<float>(transparencyPresetCount) * static_cast<float>(index))));

		m_ColorTables.at(DrawBlendMode::BlendTransparency).try_emplace(colorChannelBlendAmounts);
		create_trans_table(&m_ColorTables[DrawBlendMode::BlendTransparency].at(colorChannelBlendAmounts).first, m_DefaultPalette, adjustedBlendAmount, adjustedBlendAmount, adjustedBlendAmount, nullptr);
		m_ColorTables[DrawBlendMode::BlendTransparency].at(colorChannelBlendAmounts).second = -1;
	}
}

void FrameMan::Destroy() {
	for (const GUIScreen* guiScreen: m_GUIScreens) {
		delete guiScreen;
	}
	for (const GUIFont* guiFont: m_LargeFonts) {
		delete guiFont;
	}
	for (const GUIFont* guiFont: m_SmallFonts) {
		delete guiFont;
	}

	// We don't need to destroy opengl things because
	// their lifetime matches the game's

	Clear();
}

void FrameMan::Update() {
	// Remove all scheduled primitives, those will be re-added by updates from other entities.
	// This needs to happen here, otherwise if there are multiple sim updates during a single frame duplicates will be added to the primitive queue.
	g_PrimitiveMan.ClearPrimitivesQueue();

	// Prune unused color tables every 5 real minutes to prevent ridiculous memory usage over time.
	if (m_ColorTablePruneTimer.IsPastRealMS(300000)) {
		long long currentTime = g_TimerMan.GetAbsoluteTime() / 10000;
		for (std::unordered_map<std::array<int, 4>, std::pair<COLOR_MAP, long long>>& colorTableMap: m_ColorTables) {
			if (colorTableMap.size() >= 100) {
				std::vector<std::array<int, 4>> markedForDelete;
				markedForDelete.reserve(colorTableMap.size());
				for (const auto& [tableKey, tableData]: colorTableMap) {
					long long lastAccessTime = tableData.second;
					// Mark tables that haven't been accessed in the last minute for deletion. Avoid marking the transparency table presets, those will have lastAccessTime set to -1.
					if (lastAccessTime != -1 && (currentTime - lastAccessTime > 60)) {
						markedForDelete.emplace_back(tableKey);
					}
				}
				for (const std::array<int, 4>& keyToDelete: markedForDelete) {
					colorTableMap.erase(keyToDelete);
				}
			}
		}
		m_ColorTablePruneTimer.Reset();
	}

	// Update redundantly in sim update to ensure our values are exactly precise for the purposes of script GetOffset()
	int screenCount = (m_HSplit ? 2 : 1) * (m_VSplit ? 2 : 1);
	for (int playerScreen = 0; playerScreen < screenCount; ++playerScreen) {
		g_CameraMan.Update(playerScreen);
	}
}

void FrameMan::ResetSplitScreens(bool hSplit, bool vSplit) {
	// Override screen splitting according to settings if needed
	if ((hSplit || vSplit) && !(hSplit && vSplit) && m_TwoPlayerVSplit) {
		hSplit = false;
		vSplit = m_TwoPlayerVSplit;
	}

	m_HSplit = hSplit;
	m_VSplit = vSplit;

	// Create the splitscreen buffer
	if (m_HSplit || m_VSplit) {
		m_PlayerScreen8 = std::unique_ptr<BITMAP, BitmapDeleter>(create_bitmap_ex(8, g_WindowMan.GetResX() / (m_VSplit ? 2 : 1), g_WindowMan.GetResY() / (m_HSplit ? 2 : 1)));
		clear_to_color(m_PlayerScreen8.get(), 0);
		set_clip_state(m_PlayerScreen8.get(), 1);

		m_PlayerScreen = std::make_unique<RenderTarget>(FloatRect(0, 0, g_WindowMan.GetResX() / (m_VSplit ? 2 : 1), g_WindowMan.GetResY() / (m_HSplit ? 2 : 1)), FloatRect(0, 0, g_WindowMan.GetResX() / (m_VSplit ? 2 : 1), g_WindowMan.GetResY() / (m_HSplit ? 2 : 1)));

		// Update these to represent the split screens
		m_PlayerScreenWidth = m_PlayerScreen->GetSize().w;
		m_PlayerScreenHeight = m_PlayerScreen->GetSize().h;
	} else {
		m_PlayerScreen8 = m_BackBuffer8;
		m_PlayerScreen = m_BackBuffer;
		// No splits, so set the screen dimensions equal to the back buffer
		m_PlayerScreenWidth = m_BackBuffer8->w;
		m_PlayerScreenHeight = m_BackBuffer8->h;
	}
	for (int i = 0; i < c_MaxScreenCount; ++i) {
		m_FlashScreenColor[i] = -1;
		m_FlashedLastFrame[i] = false;
	}
}

float FrameMan::GetResolutionMultiplier() const {
	return g_WindowMan.GetResMultiplier();
}

Vector FrameMan::GetMiddleOfPlayerScreen(int whichPlayer) {
	Vector middleOfPlayerScreen;

	if (whichPlayer == -1) {
		middleOfPlayerScreen.SetXY(static_cast<float>(g_WindowMan.GetResX() / 2), static_cast<float>(g_WindowMan.GetResY() / 2));
	} else {
		int playerScreen = g_ActivityMan.GetActivity()->ScreenOfPlayer(whichPlayer);

		middleOfPlayerScreen.SetXY(static_cast<float>(m_PlayerScreenWidth / 2), static_cast<float>(m_PlayerScreenHeight / 2));
		if ((playerScreen == 1 && g_FrameMan.GetVSplit()) || playerScreen == 3) {
			middleOfPlayerScreen.SetX(middleOfPlayerScreen.GetX() + static_cast<float>(m_PlayerScreenWidth));
		}
		if ((playerScreen == 1 && g_FrameMan.GetHSplit()) || playerScreen >= 2) {
			middleOfPlayerScreen.SetY(middleOfPlayerScreen.GetY() + static_cast<float>(m_PlayerScreenHeight));
		}
	}
	return middleOfPlayerScreen;
}

int FrameMan::GetPlayerFrameBufferWidth(int whichPlayer) const {
	return m_PlayerScreenWidth;
}

int FrameMan::GetPlayerFrameBufferHeight(int whichPlayer) const {
	return m_PlayerScreenHeight;
}

int FrameMan::CalculateTextHeight(const std::string& text, int maxWidth, bool isSmall) {
	return isSmall ? GetSmallFont()->CalculateHeight(text, maxWidth) : GetLargeFont()->CalculateHeight(text, maxWidth);
}

std::string FrameMan::SplitStringToFitWidth(const std::string& stringToSplit, int widthLimit, bool useSmallFont) {
	GUIFont* fontToUse = GetFont(useSmallFont, false);
	auto SplitSingleLineAsNeeded = [this, &widthLimit, &fontToUse](std::string& lineToSplitAsNeeded) {
		int numberOfScreenWidthsForText = static_cast<int>(std::ceil(static_cast<float>(fontToUse->CalculateWidth(lineToSplitAsNeeded)) / static_cast<float>(widthLimit)));
		if (numberOfScreenWidthsForText > 1) {
			int splitInterval = static_cast<int>(std::ceil(static_cast<float>(lineToSplitAsNeeded.size()) / static_cast<float>(numberOfScreenWidthsForText)));
			for (int i = 1; i <= numberOfScreenWidthsForText; i++) {
				size_t newLineCharacterPosition = std::min(static_cast<size_t>(i * splitInterval + (i - 1)), lineToSplitAsNeeded.size());
				if (newLineCharacterPosition == lineToSplitAsNeeded.size()) {
					break;
				}
				lineToSplitAsNeeded.insert(newLineCharacterPosition, "\n");
			}
		}
	};

	std::string splitString;
	size_t previousNewLinePos = 0;
	size_t nextNewLinePos = stringToSplit.find("\n");
	if (nextNewLinePos != std::string::npos) {
		while (nextNewLinePos != std::string::npos) {
			nextNewLinePos = stringToSplit.find("\n", previousNewLinePos);
			std::string currentLine = stringToSplit.substr(previousNewLinePos, nextNewLinePos - previousNewLinePos);
			previousNewLinePos = nextNewLinePos + 1;

			SplitSingleLineAsNeeded(currentLine);
			splitString += currentLine;
			if (nextNewLinePos != std::string::npos) {
				splitString += "\n";
			}
		}
	} else {
		splitString = stringToSplit;
		SplitSingleLineAsNeeded(splitString);
	}

	return splitString;
}

int FrameMan::CalculateTextWidth(const std::string& text, bool isSmall) {
	return isSmall ? GetSmallFont()->CalculateWidth(text) : GetLargeFont()->CalculateWidth(text);
}

void FrameMan::SetScreenText(const std::string& message, int whichScreen, int blinkInterval, int displayDuration, bool centered) {
	// See if we can overwrite the previous message
	if (whichScreen >= 0 && whichScreen < c_MaxScreenCount && m_TextDurationTimer[whichScreen].IsPastRealMS(m_TextDuration[whichScreen])) {
		m_ScreenText[whichScreen] = message;
		m_TextDuration[whichScreen] = displayDuration;
		m_TextDurationTimer[whichScreen].Reset();
		m_TextBlinking[whichScreen] = blinkInterval;
		m_TextCentered[whichScreen] = centered;
	}
}

void FrameMan::ClearScreenText(int whichScreen) {
	if (whichScreen >= 0 && whichScreen < c_MaxScreenCount) {
		m_ScreenText[whichScreen].clear();
		m_TextDuration[whichScreen] = -1;
		m_TextDurationTimer[whichScreen].Reset();
		m_TextBlinking[whichScreen] = 0;
	}
}

void FrameMan::SetAlphaBlendMode() {
	if (m_DrawingHudLayer) {
		// The HUD layer starts out clear and is blended over the world afterwards, so its alpha has to add up how opaque everything drawn into it is. Plain alpha blending multiplies alpha by itself instead.
		rlSetBlendFactorsSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA, GL_FUNC_ADD, GL_FUNC_ADD);
		rlSetBlendMode(RL_BLEND_CUSTOM_SEPARATE);
	} else {
		rlSetBlendMode(RL_BLEND_ALPHA);
	}
}

void FrameMan::SetBlendMode(DrawBlendMode blendMode) {
	GLint invertLoc = rlGetLocationUniformCurrent("rteBlendInvert");
	glUniform1i(invertLoc, 0);
	switch (blendMode) {
		case BlendTransparency: {
			SetAlphaBlendMode();
			break;
		}
		case BlendScreen: {
			rlSetBlendMode(RL_BLEND_SCREEN);
			break;
		}
		case BlendDifference: {
			rlSetBlendMode(RL_BLEND_SUBTRACT_COLORS);
			break;
		}
		case BlendMultiply: {
			rlSetBlendMode(RL_BLEND_MULTIPLIED);
			break;
		}
		case BlendBurn: {
			rlSetBlendMode(RL_BLEND_BURN);
			break;
		}
		case BlendDodge: {
			rlSetBlendMode(RL_BLEND_DODGE);
			break;
		}
		case BlendColor: {
			rlSetBlendMode(RL_BLEND_HSL_COLOR);
			break;
		}
		case BlendLuminance: {
			rlSetBlendMode(RL_BLEND_HSL_LUMINOSITY);
			break;
		}
		case BlendSaturation: {
			rlSetBlendMode(RL_BLEND_HSL_SATURATION);
			break;
		}
		case BlendInvert: {
			SetAlphaBlendMode();
			glUniform1i(invertLoc, 1);
			break;
		}
		case BlendDissolve: {
			SetAlphaBlendMode();
			const Shader* dissolve = dynamic_cast<const Shader*>(g_PresetMan.GetEntityPreset("Shader", "Dissolve"));
			dissolve->Begin();
			GLint paletteLoc = dissolve->GetUniformLocation("rtePalette");
			rlSetUniformSampler(paletteLoc, g_PostProcessMan.GetPaletteTexture());
			break;
		}
		default: {
			SetAlphaBlendMode();
			RTEAssert(blendMode < BlendModeCount, "Unkown blend mode selected!");
			break;
		}
	}
}

void FrameMan::SetColorTable(DrawBlendMode blendMode, std::array<int, 4> colorChannelBlendAmounts) {
	RTEAssert(blendMode > DrawBlendMode::NoBlend && blendMode < DrawBlendMode::BlendModeCount, "Invalid DrawBlendMode or DrawBlendMode::NoBlend passed into FrameMan::SetColorTable. See DrawBlendMode enumeration for defined values.");

	for (int& colorChannelBlendAmount: colorChannelBlendAmounts) {
		colorChannelBlendAmount = RoundToNearestMultiple(std::clamp(colorChannelBlendAmount, static_cast<int>(BlendAmountLimits::MinBlend), static_cast<int>(BlendAmountLimits::MaxBlend)), c_BlendAmountStep);
	}

	bool usedPresetTransparencyTable = false;

	switch (blendMode) {
		case DrawBlendMode::NoBlend:
			RTEAbort("Somehow ended up attempting to set a color table for DrawBlendMode::NoBlend in FrameMan::SetColorTable! This should never happen!");
			return;
		case DrawBlendMode::BlendInvert:
		case DrawBlendMode::BlendDissolve:
			// Invert and Dissolve do nothing with the RGB channels values, so set all channels to Alpha channel value to avoid creating pointless variants.
			colorChannelBlendAmounts.fill(colorChannelBlendAmounts[3]);
			break;
		case DrawBlendMode::BlendTransparency:
			// Indexed transparency has dedicated maps that don't use alpha, so min it to attempt to load one of the presets, and in case there isn't one avoid creating a variant for each alpha value.
			colorChannelBlendAmounts[3] = BlendAmountLimits::MinBlend;
			usedPresetTransparencyTable = (colorChannelBlendAmounts[0] == colorChannelBlendAmounts[1]) && (colorChannelBlendAmounts[0] == colorChannelBlendAmounts[2]);
			break;
		default:
			break;
	}

	// New color tables will be created using the default palette loaded at FrameMan initialization because handling per-palette per-mode color tables is too much headache, even if it may possibly produce better blending results.
	if (m_ColorTables[blendMode].find(colorChannelBlendAmounts) == m_ColorTables[blendMode].end()) {
		m_ColorTables[blendMode].try_emplace(colorChannelBlendAmounts);

		std::array<int, 4> adjustedColorChannelBlendAmounts = {BlendAmountLimits::MinBlend, BlendAmountLimits::MinBlend, BlendAmountLimits::MinBlend, BlendAmountLimits::MinBlend};
		for (int index = 0; index < adjustedColorChannelBlendAmounts.size(); ++index) {
			adjustedColorChannelBlendAmounts[index] = 255 - (static_cast<int>(255.0F * 0.01F * static_cast<float>(colorChannelBlendAmounts[index])));
		}

		if (blendMode == DrawBlendMode::BlendTransparency) {
			// Paletted transparency has dedicated tables so better create one instead of generic for best result. Alpha is ignored here.
			create_trans_table(&m_ColorTables[DrawBlendMode::BlendTransparency].at(colorChannelBlendAmounts).first, m_DefaultPalette, adjustedColorChannelBlendAmounts[0], adjustedColorChannelBlendAmounts[1], adjustedColorChannelBlendAmounts[2], nullptr);
		} else {
			c_BlenderSetterFunctions[blendMode](adjustedColorChannelBlendAmounts[0], adjustedColorChannelBlendAmounts[1], adjustedColorChannelBlendAmounts[2], adjustedColorChannelBlendAmounts[3]);
			create_blender_table(&m_ColorTables[blendMode].at(colorChannelBlendAmounts).first, m_DefaultPalette, nullptr);
			// Reset the blender to avoid potentially screwing some true-color draw operation. Hopefully.
			c_BlenderSetterFunctions[blendMode](255, 255, 255, 255);
		}
	}
	color_map = &m_ColorTables[blendMode].at(colorChannelBlendAmounts).first;
	m_ColorTables[blendMode].at(colorChannelBlendAmounts).second = usedPresetTransparencyTable ? -1 : (g_TimerMan.GetAbsoluteTime() / 10000);
}

void FrameMan::SetTransTableFromPreset(TransparencyPreset transPreset) {
	RTEAssert(transPreset == TransparencyPreset::LessTrans || transPreset == TransparencyPreset::HalfTrans || transPreset == TransparencyPreset::MoreTrans, "Undefined transparency preset value passed in. See TransparencyPreset enumeration for defined values.");
	std::array<int, 4> colorChannelBlendAmounts = {transPreset, transPreset, transPreset, BlendAmountLimits::MinBlend};
	if (m_ColorTables[DrawBlendMode::BlendTransparency].find(colorChannelBlendAmounts) != m_ColorTables[DrawBlendMode::BlendTransparency].end()) {
		color_map = &m_ColorTables[DrawBlendMode::BlendTransparency].at(colorChannelBlendAmounts).first;
		m_ColorTables[DrawBlendMode::BlendTransparency].at(colorChannelBlendAmounts).second = -1;
	}
	constexpr int transparencyPresetCount = BlendAmountLimits::MaxBlend / c_BlendAmountStep;
	m_CurrentAlpha = 255 - (static_cast<int>(255.0F * ((1.0F / static_cast<float>(transparencyPresetCount)) * static_cast<float>(transPreset / c_BlendAmountStep))));
}

bool FrameMan::LoadPalette(const std::string& palettePath) {
	const std::string fullPalettePath = g_PresetMan.GetFullModulePath(palettePath);
	SDL_Surface* paletteImage = IMG_Load(palettePath.c_str());
	RTEAssert(paletteImage && SDL_GetSurfacePalette(paletteImage), ("Failed to load palette from bitmap with following path:\n\n" + fullPalettePath).c_str());

	SDL_Palette* palette = SDL_GetSurfacePalette(paletteImage);
	for (size_t i = 0; i < 256; i++) {
		m_Palette[i] = {
		    palette->colors[i].r,
		    palette->colors[i].g,
		    palette->colors[i].b,
		    0};
	}
	SDL_DestroySurface(paletteImage);

	set_palette(m_Palette);

	// Update what black is now with the loaded palette
	m_BlackColor = bestfit_color(m_Palette, 0, 0, 0);
	m_AlmostBlackColor = bestfit_color(m_Palette, 5, 5, 5);

	return true;
}

int FrameMan::SaveBitmap(SaveBitmapMode modeToSave, const std::string& nameBase, BITMAP* bitmapToSave) {
	if ((modeToSave == WorldDump || modeToSave == ScenePreviewDump) && !g_ActivityMan.ActivityRunning()) {
		return 0;
	}
	if (nameBase.empty() || nameBase.size() <= 0) {
		return -1;
	}
	set_palette(m_DefaultPalette);

	// TODO: Remove this once GCC13 is released and switched to. std::format and std::chrono::time_zone are not part of latest libstdc++.
#if defined(__GNUC__) && (__GNUC__ < 13 || defined(__APPLE__)) // FIXME: macOS for some reason builds with incorrect iconv in CI which breaks format, could not debug.
	std::chrono::time_point now = std::chrono::system_clock::now();
	time_t currentTime = std::chrono::system_clock::to_time_t(now);
	tm* localCurrentTime = std::localtime(&currentTime);
	std::array<char, 32> formattedTimeAndDate = {};
	std::strftime(formattedTimeAndDate.data(), sizeof(formattedTimeAndDate), "%F_%H-%M-%S", localCurrentTime);

	std::array<char, 128> fullFileNameBuffer = {};
	// We can't get sub-second precision from timeBuffer so we'll append absolute time to not overwrite the same file when dumping multiple times per second.
	std::snprintf(fullFileNameBuffer.data(), sizeof(fullFileNameBuffer), "%s/%s_%s.%lli.png", System::GetScreenshotDirectory().c_str(), nameBase.c_str(), formattedTimeAndDate.data(), g_TimerMan.GetAbsoluteTime());

	std::string fullFileName(fullFileNameBuffer.data());
#else
	std::string fullFileName = std::format("{}{}_{:%F_%H-%M-%S}.png", System::GetScreenshotDirectory(), nameBase, std::chrono::current_zone()->to_local(std::chrono::system_clock::now()));
#endif

	bool saveSuccess = false;

	switch (modeToSave) {
		case SingleBitmap:
			if (bitmapToSave && save_png(nameBase.c_str(), bitmapToSave, m_Palette) == 0) {
				g_ConsoleMan.PrintString("SYSTEM: Bitmap was dumped to: " + nameBase);
				saveSuccess = true;
			}
			break;
		case ScreenDump:
			if (m_ScreenDumpBuffer) {
				SaveScreenToBitmap();

				// Make a copy of the buffer because it may be overwritten mid thread and everything will be on fire.
				SDL_Surface* saveSurface = SDL_ConvertSurface(m_ScreenDumpBuffer.get(), m_ScreenDumpBuffer->format);
				auto saveScreenDump = [fullFileName](SDL_Surface* bitmapToSaveCopy) {
					// nullptr for the PALETTE parameter here because we're saving a 24bpp file and it's irrelevant.
					if (IMG_SavePNG(bitmapToSaveCopy, fullFileName.c_str()) == 0) {
						g_ConsoleMan.PrintString("SYSTEM: Screen was dumped to: " + fullFileName);
					} else {
						g_ConsoleMan.PrintString("ERROR: Unable to save bitmap to: " + fullFileName);
					}
					// SDL_FreeSurface(bitmapToSaveCopy);
				};
				std::thread saveThread(saveScreenDump, saveSurface);
				saveThread.detach();

				saveSuccess = true;
			}
			break;
		case ScenePreviewDump:
		case WorldDump:
			if (!m_WorldDumpBuffer || (m_WorldDumpBuffer->w != g_SceneMan.GetSceneWidth() || m_WorldDumpBuffer->h != g_SceneMan.GetSceneHeight())) {
				m_WorldDumpBuffer = std::unique_ptr<BITMAP, BitmapDeleter>(create_bitmap_ex(c_BPP, g_SceneMan.GetSceneWidth(), g_SceneMan.GetSceneHeight()));
			}
			if (modeToSave == ScenePreviewDump) {
				DrawWorldDump(true);

				BITMAP* scenePreviewDumpBuffer = create_bitmap_ex(c_BPP, c_ScenePreviewWidth, c_ScenePreviewHeight);
				blit(m_ScenePreviewDumpGradient.get(), scenePreviewDumpBuffer, 0, 0, 0, 0, scenePreviewDumpBuffer->w, scenePreviewDumpBuffer->h);
				masked_stretch_blit(m_WorldDumpBuffer.get(), scenePreviewDumpBuffer, 0, 0, m_WorldDumpBuffer->w, m_WorldDumpBuffer->h, 0, 0, scenePreviewDumpBuffer->w, scenePreviewDumpBuffer->h);

				if (SaveIndexedPNG(fullFileName.c_str(), scenePreviewDumpBuffer) == 0) {
					g_ConsoleMan.PrintString("SYSTEM: Scene Preview was dumped to: " + fullFileName);
					saveSuccess = true;
				}
				destroy_bitmap(scenePreviewDumpBuffer);
			} else {
				DrawWorldDump();

				BITMAP* depthConvertBitmap = create_bitmap_ex(24, m_WorldDumpBuffer->w, m_WorldDumpBuffer->h);
				blit(m_WorldDumpBuffer.get(), depthConvertBitmap, 0, 0, 0, 0, m_WorldDumpBuffer->w, m_WorldDumpBuffer->h);
				SDL_Surface* saveSurface = SDL_CreateSurfaceFrom(
				    depthConvertBitmap->w,
				    depthConvertBitmap->h,
				    SDL_PIXELFORMAT_RGB24,
				    depthConvertBitmap->dat,
				    3);

				if (IMG_SavePNG(saveSurface, fullFileName.c_str()) == 0) {
					g_ConsoleMan.PrintString("SYSTEM: World was dumped to: " + fullFileName);
					saveSuccess = true;
				}
				destroy_bitmap(depthConvertBitmap);
				SDL_DestroySurface(saveSurface);
			}
			break;
		default:
			g_ConsoleMan.PrintString("ERROR: Wrong bitmap save mode passed in, no bitmap was saved!");
			return -1;
	}
	if (!saveSuccess) {
		g_ConsoleMan.PrintString("ERROR: Unable to save bitmap to: " + fullFileName);
		return -1;
	} else {
		return 0;
	}
}

void FrameMan::SaveScreenToBitmap() {
	if (!m_ScreenDumpBuffer) {
		return;
	}

	glPixelStorei(GL_PACK_ALIGNMENT, 4);
	GL_CHECK(glBindTexture(GL_TEXTURE_2D, g_WindowMan.GetScreenBuffer()->GetColorTexture().id));
	GL_CHECK(glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_UNSIGNED_BYTE, m_ScreenDumpBuffer->pixels));

	// Flip the pixels
	std::vector<char*> temp(m_ScreenDumpBuffer->pitch);
	char* pixels = reinterpret_cast<char*>(m_ScreenDumpBuffer->pixels);
	size_t pitch = m_ScreenDumpBuffer->pitch;
	for (size_t y = 0; y < m_ScreenDumpBuffer->h / 2; ++y) {
		std::swap_ranges(pixels + y * pitch, pixels + (y + 1) * pitch, pixels + (m_ScreenDumpBuffer->h - y - 1) * pitch);
	}
}

int FrameMan::SaveIndexedPNG(const char* fileName, BITMAP* bitmapToSave) const {
	set_palette(m_DefaultPalette);
	SDL_Surface* surface = SDL_CreateSurfaceFrom(bitmapToSave->w,
	                                             bitmapToSave->h,
	                                             SDL_PIXELFORMAT_INDEX8,
	                                             bitmapToSave->dat,
	                                             bitmap_color_depth(bitmapToSave) / 8);
	SDL_Palette* pal = ContentFile::DefaultPaletteToSDL();
	SDL_SetSurfacePalette(surface, pal);
	int saveResult = IMG_SavePNG(surface, fileName);
	SDL_DestroyPalette(pal);
	SDL_DestroySurface(surface);

	return saveResult;
}

int FrameMan::SharedDrawLine(BITMAP* bitmap, const Vector& start, const Vector& end, int color, int altColor, int skip, int skipStart, bool shortestWrap, bool drawDot, BITMAP* dot) const {
	RTEAssert(bitmap, "Trying to draw line to null Bitmap");
	if (drawDot) {
		RTEAssert(dot, "Trying to draw line of dots without specifying a dot Bitmap");
	}

	int error = 0;
	int dom = 0;
	int sub = 0;
	int domSteps = 0;
	int skipped = skip + (skipStart - skip);
	int intPos[2];
	int delta[2];
	int delta2[2];
	int increment[2];
	bool drawAlt = false;

	int dotHeight = drawDot ? dot->h : 0;
	int dotWidth = drawDot ? dot->w : 0;

	// acquire_bitmap(bitmap);

	// Just make the alt the same color as the main one if no one was specified
	if (altColor == 0) {
		altColor = color;
	}

	intPos[X] = start.GetFloorIntX();
	intPos[Y] = start.GetFloorIntY();

	// Wrap line around the scene if it makes it shorter
	if (shortestWrap) {
		Vector deltaVec = g_SceneMan.ShortestDistance(start, end, false);
		delta[X] = deltaVec.GetFloorIntX();
		delta[Y] = deltaVec.GetFloorIntY();
	} else {
		delta[X] = end.GetFloorIntX() - intPos[X];
		delta[Y] = end.GetFloorIntY() - intPos[Y];
	}
	if (delta[X] == 0 && delta[Y] == 0) {
		return 0;
	}

	// Bresenham's line drawing algorithm preparation
	if (delta[X] < 0) {
		increment[X] = -1;
		delta[X] = -delta[X];
	} else {
		increment[X] = 1;
	}
	if (delta[Y] < 0) {
		increment[Y] = -1;
		delta[Y] = -delta[Y];
	} else {
		increment[Y] = 1;
	}

	// Scale by 2, for better accuracy of the error at the first pixel
	delta2[X] = delta[X] << 1;
	delta2[Y] = delta[Y] << 1;

	// If X is dominant, Y is submissive, and vice versa.
	if (delta[X] > delta[Y]) {
		dom = X;
		sub = Y;
	} else {
		dom = Y;
		sub = X;
	}
	error = delta2[sub] - delta[dom];

	// Bresenham's line drawing algorithm execution
	for (domSteps = 0; domSteps < delta[dom]; ++domSteps) {
		intPos[dom] += increment[dom];
		if (error >= 0) {
			intPos[sub] += increment[sub];
			error -= delta2[dom];
		}
		error += delta2[sub];

		// Only draw pixel if we're not due to skip any
		if (++skipped > skip) {
			// Scene wrapping, if necessary
			g_SceneMan.WrapPosition(intPos[X], intPos[Y]);

			if (drawDot) {
				masked_blit(dot, bitmap, 0, 0, intPos[X] - (dotWidth / 2), intPos[Y] - (dotHeight / 2), dot->w, dot->h);
			} else {
				putpixel(bitmap, intPos[X], intPos[Y], drawAlt ? color : altColor);
			}
			drawAlt = !drawAlt;
			skipped = 0;
		}
	}

	// Return the end phase state of the skipping
	return skipped;
}

GUIFont* FrameMan::GetFont(bool isSmall, bool trueColor) {
	size_t colorIndex = trueColor ? 1 : 0;

	if (!m_GUIScreens[colorIndex]) {
		m_GUIScreens[colorIndex] = new AllegroScreen(trueColor ? m_BackBuffer32.get() : m_BackBuffer8.get());
	}

	if (isSmall) {
		if (!m_SmallFonts[colorIndex]) {
			std::string fontName = "SmallFont";
			std::string fontPath = "Base.rte/GUIs/Skins/FontSmall.png";

			if (trueColor) {
				fontName = "SmallFont32";
				fontPath = "Base.rte/GUIs/Skins/Menus/FontSmall.png";
			}
			m_SmallFonts[colorIndex] = new GUIFont(fontName);
			m_SmallFonts[colorIndex]->Load(m_GUIScreens[colorIndex], fontPath);
		}
		return m_SmallFonts[colorIndex];
	}
	if (!m_LargeFonts[colorIndex]) {
		std::string fontName = "FatFont";
		std::string fontPath = "Base.rte/GUIs/Skins/FontLarge.png";

		if (trueColor) {
			fontName = "FatFont32";
			fontPath = "Base.rte/GUIs/Skins/Menus/FontLarge.png";
		}
		m_LargeFonts[colorIndex] = new GUIFont(fontName);
		m_LargeFonts[colorIndex]->Load(m_GUIScreens[colorIndex], fontPath);
	}
	return m_LargeFonts[colorIndex];
}

void FrameMan::UpdateScreenOffsetForSplitScreen(int playerScreen, Vector& screenOffset) const {
	switch (playerScreen) {
		case Players::PlayerTwo:
			// If both splits, or just VSplit, then in upper right quadrant
			if ((m_VSplit && !m_HSplit) || (m_VSplit && m_HSplit)) {
				screenOffset.SetXY(g_WindowMan.GetResX() / 2, 0);
			} else {
				// If only HSplit, then lower left quadrant
				screenOffset.SetXY(0, g_WindowMan.GetResY() / 2);
			}
			break;
		case Players::PlayerThree:
			// Always lower left quadrant
			screenOffset.SetXY(0, g_WindowMan.GetResY() / 2);
			break;
		case Players::PlayerFour:
			// Always lower right quadrant
			screenOffset.SetXY(g_WindowMan.GetResX() / 2, g_WindowMan.GetResY() / 2);
			break;
		default:
			// Always upper left corner
			screenOffset.SetXY(0, 0);
			break;
	}
}

void FrameMan::Draw() {
	ZoneScopedN("Draw");
	TracyGpuZone("FrameMan::Draw");

	// rlSetShader(rlGetShaderIdDefault(), rlGetShaderLocsDefault());
	Shader backgroundShader;
	g_PresetMan.GetEntityPreset("Shader", "Background")->Clone(&backgroundShader);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	clear_to_color(m_BackBuffer8.get(), 0);
	clear_to_color(m_BackBuffer8GUI.get(), 0);
	// The HUD layer starts each frame clear, ready for every player screen's HUD
	m_HudLayer->Begin(true);
	m_HudLayer->End();
	m_BackBuffer->Begin(true);

	// Count how many split screens we'll need
	int screenCount = (m_HSplit ? 2 : 1) * (m_VSplit ? 2 : 1);
	RTEAssert(screenCount <= 1 || m_PlayerScreen, "Splitscreen surface not ready when needed!");

	g_PostProcessMan.ClearScreenPostEffects();

	// These accumulate the effects for each player's screen area, and are then transferred to the post-processing lists with the player screen offset applied
	std::list<PostEffect> screenRelativeEffects;
	std::list<Box> screenRelativeGlowBoxes;

	const Activity* pActivity = g_ActivityMan.GetActivity();

	for (int playerScreen = 0; playerScreen < screenCount; ++playerScreen) {

		screenRelativeEffects.clear();
		screenRelativeGlowBoxes.clear();
		rlEnableColorBlend();
		rlSetBlendMode(RL_BLEND_ALPHA);
		rlEnableDepthTest();

		m_PlayerScreen->Begin(true, 1.0f);
		backgroundShader.Begin();
		backgroundShader.Enable();
		rlSetUniformSampler(backgroundShader.GetUniformLocation("rtePalette"), g_PostProcessMan.GetPaletteTexture());
		backgroundShader.SetInt("drawMasked", 1);

		// rlSetUniformSampler(backgroundShader.GetUniformLocation("rtePalette"), g_PostProcessMan.GetPaletteTexture());
		BITMAP* drawScreen = (screenCount == 1) ? m_BackBuffer8.get() : m_PlayerScreen8.get();
		BITMAP* drawScreenGUI = (screenCount == 1) ? m_BackBuffer8GUI.get() : m_PlayerScreen8.get(); // GTODO: split screens have no GUI back buffer of their own, so their GUI goes on the player screen
		// Need to clear the backbuffers because Scene background layers can be too small to fill the whole backbuffer or drawn masked resulting in artifacts from the previous frame.
		clear_to_color(drawScreenGUI, ColorKeys::g_MaskColor);
		// If in online multiplayer mode clear to mask color otherwise the scene background layers will get drawn over.
		clear_to_color(drawScreen, 0);

		AllegroBitmap playerGUIBitmap(drawScreenGUI);

		// Update the scene view to line up with a specific screen and then draw it onto the intermediate screen
		g_CameraMan.Update(playerScreen);
		g_SceneMan.Update(playerScreen);

		Vector targetPos = g_CameraMan.GetOffset(playerScreen);

		// Adjust the drawing position on the target screen for if the target screen is larger than the scene in non-wrapping dimension.
		// Scene needs to be displayed centered on the target bitmap then, and that has to be adjusted for when drawing to the screen
		if (!g_SceneMan.SceneWrapsX() && drawScreen->w > g_SceneMan.GetSceneWidth()) {
			targetPos.m_X += (drawScreen->w - g_SceneMan.GetSceneWidth()) / 2;
		}
		if (!g_SceneMan.SceneWrapsY() && drawScreen->h > g_SceneMan.GetSceneHeight()) {
			targetPos.m_Y += (drawScreen->h - g_SceneMan.GetSceneHeight()) / 2;
		}

		// Draw the scene, into bitmaps kept from frame to frame for as long as the screens stay the same size
		SizeBitmap(MOColorBM, drawScreen->w, drawScreen->h);
		SizeBitmap(FgTerrainBM, drawScreen->w, drawScreen->h);
		SizeBitmap(BgTerrainBM, drawScreen->w, drawScreen->h);
		clear_to_color(MOColorBM.GetBitmap(), g_RedColor);
		clear_to_color(FgTerrainBM.GetBitmap(), 0);
		clear_to_color(BgTerrainBM.GetBitmap(), g_RedColor);

		g_SceneMan.Draw(drawScreen, drawScreenGUI, MOColorBM.GetBitmap(), BgTerrainBM.GetBitmap(), FgTerrainBM.GetBitmap(), targetPos);

		// The position of the current draw screen on the backbuffer
		Vector screenOffset;

		// If we are dealing with split screens, then deal with the fact that we need to draw the player screens to different locations on the final buffer
		if (screenCount > 1) {
			UpdateScreenOffsetForSplitScreen(playerScreen, screenOffset);
		}

		// The HUD and primitives go into their own layer, so they can be blended over the world once the final pass has drawn it
		BeginHudLayer(backgroundShader, screenOffset);

		g_SceneMan.DrawHUD(drawScreenGUI, targetPos);

		g_PrimitiveMan.DrawPrimitives(playerScreen, drawScreenGUI, targetPos);

		// Get only the scene-relative post effects that affect this player's screen
		if (pActivity) {
			g_PostProcessMan.GetPostScreenEffectsWrapped(targetPos, drawScreen->w, drawScreen->h, screenRelativeEffects, pActivity->GetTeamOfPlayer(pActivity->PlayerOfScreen(playerScreen)));
			g_PostProcessMan.GetGlowAreasWrapped(targetPos, drawScreen->w, drawScreen->h, screenRelativeGlowBoxes);
		}

		// TODO: Find out what keeps disabling the clipping on the draw bitmap
		// Enable clipping on the draw bitmap
		set_clip_state(drawScreen, 1);

		DrawScreenText(playerScreen, playerGUIBitmap);

		DrawScreenFlash(playerScreen, drawScreenGUI);

		EndHudLayer(backgroundShader);

		// Draw the intermediate draw splitscreen to the appropriate spot on the back buffer
		blit(drawScreen, m_BackBuffer8.get(), 0, 0, screenOffset.GetFloorIntX(), screenOffset.GetFloorIntY(), drawScreen->w, drawScreen->h);
		m_PlayerScreen->End();
		backgroundShader.End();
		if (screenCount > 1) {
			m_BackBuffer->Begin(false);
			// Copy without blending, otherwise anything drawn translucent comes out darker.
			rlDisableColorBlend();
			DrawTextureRec(m_PlayerScreen->GetColorTexture(), {0, 0, static_cast<float>(m_PlayerScreen8->w), -static_cast<float>(m_PlayerScreen8->h)}, {screenOffset.m_X, screenOffset.m_Y}, {255, 255, 255, 255});
			m_BackBuffer->End();
			rlEnableColorBlend();
		}
		g_PostProcessMan.AdjustEffectsPosToPlayerScreen(playerScreen, drawScreen, screenOffset, screenRelativeEffects, screenRelativeGlowBoxes);
	}

	// Clears the pixels that have been revealed from the unseen layers
	// GTODO: nothing fills the seen pixel lists any more, so this does nothing
	g_SceneMan.ClearSeenMaskPixels();

	// Draw separating lines for split-screens
	if (m_HSplit) {
		hline(m_BackBuffer8.get(), 0, (m_BackBuffer8->h / 2) - 1, m_BackBuffer8->w - 1, m_AlmostBlackColor);
		hline(m_BackBuffer8.get(), 0, (m_BackBuffer8->h / 2), m_BackBuffer8->w - 1, m_AlmostBlackColor);
	}
	if (m_VSplit) {
		vline(m_BackBuffer8.get(), (m_BackBuffer8->w / 2) - 1, 0, m_BackBuffer8->h - 1, m_AlmostBlackColor);
		vline(m_BackBuffer8.get(), (m_BackBuffer8->w / 2), 0, m_BackBuffer8->h - 1, m_AlmostBlackColor);
	}

	rlEnableDepthTest();
	rlZDepth(c_GuiDepth - 1.0f);
	g_GLResourceMan.UpdateDynamicBitmap(m_BackBuffer8.get(), true);

	// Fog of war things!
	bool fowEnabled;
	Activity* currentActivity = g_ActivityMan.GetActivity();
	fowEnabled = dynamic_cast<GameActivity*>(currentActivity)->GetFogOfWarEnabled();
	// The final pass draws the whole screen from these textures whether fog of war is on or not, and ignores the fog of war when it's off
	Scene* currentScene = g_SceneMan.GetCurrentScene();
	if (currentScene != m_ScenePreviouslyUsedForOglSetup) {
		m_ScenePreviouslyUsedForOglSetup = currentScene;
		ClearFowTextures();
		InitOrReinitFowOglThings();
	}
	RenderBackgroundLayersBmToTexture();

	// TODO- this needs to be done above, per screen! Right now splitscreen is fucked
	FogOfWarSetup(backgroundShader);
	// Only fog of war uses how far each pixel is from what's in view and what's been seen before
	if (fowEnabled) {
		FogOfWarSetup_DoSDF(instantVisibleFowMaskTex.id, m_SdfResultFowMask);
		FogOfWarSetup_DoSDF(fowMaskLastSeenTex.id, m_SdfResultFowLastSeenTerrainMask);
	}

	// Drawing begins!
	backgroundShader.Begin();
	backgroundShader.Enable();

	BackgroundShaderSetUniforms(backgroundShader, fowEnabled);

	rlSetUniformSampler(backgroundShader.GetUniformLocation("rtePalette"), g_PostProcessMan.GetPaletteTexture());

	// Drawing to back buffer!
	backgroundShader.SetInt("drawMasked", 1);
	backgroundShader.SetInt("toDrawFow", 1);
	backgroundShader.SetInt("drawingForeground", 1);
	
	m_BackBuffer->Begin(false);
	// Terrain!
	DrawTexture(
	    g_GLResourceMan.GetStaticTextureFromBitmap(m_BackBuffer8.get()), 
		0.0f, 
		0.0f, 
		{255, 255, 255, 255}
	);
	m_BackBuffer->End();

	backgroundShader.SetInt("toDrawFow", 0);
	backgroundShader.SetInt("drawingForeground", 0);

	backgroundShader.End();

	rlZDepth(0);
	if (g_ActivityMan.IsInActivity()) {
		g_PostProcessMan.PostProcess();
	}

	// Draw the performance stats and console on top of everything.
	g_PerformanceMan.Draw(m_BackBuffer32.get());
	g_ConsoleMan.Draw(m_BackBuffer32.get());

#ifdef DEBUG_BUILD
	// Draw scene seam
	vline(m_BackBuffer8.get(), 0, 0, g_SceneMan.GetSceneHeight(), 5);
#endif
}

void FrameMan::DrawScreenText(int playerScreen, AllegroBitmap playerGUIBitmap) {
	int textPosY = 0;
	// Only draw screen text to actual human players
	if (playerScreen < g_ActivityMan.GetActivity()->GetHumanCount()) {
		textPosY += 12;

		if (!m_ScreenText[playerScreen].empty()) {
			int bufferOrScreenWidth = GetPlayerScreenWidth();
			int bufferOrScreenHeight = GetPlayerScreenHeight();

			if (m_TextCentered[playerScreen]) {
				textPosY = (bufferOrScreenHeight / 2) - 52;
			}

			int screenOcclusionOffsetX = g_CameraMan.GetScreenOcclusion(playerScreen).GetRoundIntX();
			// If there's really no room to offset the text into, then don't
			if (GetPlayerScreenWidth() <= g_WindowMan.GetResX() / 2) {
				screenOcclusionOffsetX = 0;
			}

			std::string screenTextToDraw = m_ScreenText[playerScreen];
			if (m_TextBlinking[playerScreen] && m_TextBlinkTimer.AlternateReal(m_TextBlinking[playerScreen])) {
				screenTextToDraw = ">>> " + screenTextToDraw + " <<<";
			}
			screenTextToDraw = SplitStringToFitWidth(screenTextToDraw, bufferOrScreenWidth, false);
			GetLargeFont()->DrawAligned(&playerGUIBitmap, (bufferOrScreenWidth + screenOcclusionOffsetX) / 2, textPosY, screenTextToDraw, GUIFont::Centre);
			textPosY += 12;
		}

		// Draw info text when in MOID or material layer draw mode
		switch (g_SceneMan.GetLayerDrawMode()) {
			case g_LayerTerrainMatter:
				GetSmallFont()->DrawAligned(&playerGUIBitmap, GetPlayerScreenWidth() / 2, GetPlayerScreenHeight() - 12, "Viewing terrain material layer\nHit Ctrl+M to cycle modes", GUIFont::Centre, GUIFont::Bottom);
				break;
			default:
				break;
		}
	} else {
		// If superfluous screen (as in a three-player match), make the fourth the Observer one
		GetLargeFont()->DrawAligned(&playerGUIBitmap, GetPlayerScreenWidth() / 2, textPosY, "- Observer View -", GUIFont::Centre);
	}
}

void FrameMan::DrawScreenFlash(int playerScreen, BITMAP* playerGUIBitmap) {
	if (m_FlashScreenColor[playerScreen] != -1) {
		// If set to flash for a period of time, first be solid and then start flashing slower
		double timeTillLimit = m_FlashTimer[playerScreen].LeftTillRealTimeLimitMS();

		if (timeTillLimit < 10 || m_FlashTimer[playerScreen].AlternateReal(50)) {
			if (m_FlashedLastFrame[playerScreen]) {
				m_FlashedLastFrame[playerScreen] = false;
			} else {
				rlZDepth(c_GuiDepth);
				rlBegin(RL_QUADS);

				rlColor4ub(m_FlashScreenColor[playerScreen], 0, 0, 50);
				rlVertex2f(playerGUIBitmap->w * .25f, playerGUIBitmap->h * .25f);
				rlVertex2f(playerGUIBitmap->w - playerGUIBitmap->w * .25f, playerGUIBitmap->h * 0.25f);
				rlColor4ub(m_FlashScreenColor[playerScreen], 0, 0, 255);
				rlVertex2f(playerGUIBitmap->w, 0.0f);
				rlVertex2f(0.0f, 0.0f);

				rlColor4ub(m_FlashScreenColor[playerScreen], 0, 0, 50);
				rlVertex2f(playerGUIBitmap->w * .25f, playerGUIBitmap->h - playerGUIBitmap->h * .25f);
				rlVertex2f(playerGUIBitmap->w * .25f, playerGUIBitmap->h * 0.25f);
				rlColor4ub(m_FlashScreenColor[playerScreen], 0, 0, 255);
				rlVertex2f(0.0f, 0.0f);
				rlVertex2f(0.0f, playerGUIBitmap->h);

				rlColor4ub(m_FlashScreenColor[playerScreen], 0, 0, 50);
				rlVertex2f(playerGUIBitmap->w - playerGUIBitmap->w * .25f, playerGUIBitmap->h - playerGUIBitmap->h * .25f);
				rlVertex2f(playerGUIBitmap->w * .25f, playerGUIBitmap->h - playerGUIBitmap->h * .25f);
				rlColor4ub(m_FlashScreenColor[playerScreen], 0, 0, 255);
				rlVertex2f(0.0f, playerGUIBitmap->h);
				rlVertex2f(playerGUIBitmap->w, playerGUIBitmap->h);

				rlColor4ub(m_FlashScreenColor[playerScreen], 0, 0, 50);
				rlVertex2f(playerGUIBitmap->w - playerGUIBitmap->w * .25f, playerGUIBitmap->h * .25f);
				rlVertex2f(playerGUIBitmap->w - playerGUIBitmap->w * .25f, playerGUIBitmap->h - playerGUIBitmap->h * .25f);
				rlColor4ub(m_FlashScreenColor[playerScreen], 0, 0, 255);
				rlVertex2f(playerGUIBitmap->w, playerGUIBitmap->h);
				rlVertex2f(playerGUIBitmap->w, 0.0f);

				rlEnd();
				rlZDepth(c_DefaultDrawDepth);
				m_FlashedLastFrame[playerScreen] = true;
			}
		}
		if (m_FlashTimer[playerScreen].IsPastRealTimeLimit()) {
			m_FlashScreenColor[playerScreen] = -1;
		}
	}
}

void FrameMan::BeginHudLayer(const Shader& backgroundShader, const Vector& screenOffset) {
	// Switching targets draws what's already queued for the player screen, before the shader starts drawing colours
	m_HudLayer->Begin(false);
	// The player screen goes at its offset on the back buffer, so its HUD goes there on the layer too
	rlTranslatef(screenOffset.m_X, screenOffset.m_Y, 0.0F);
	backgroundShader.Begin();
	backgroundShader.SetBool("drawingHudLayer", true);
	m_DrawingHudLayer = true;
	SetAlphaBlendMode();
}

void FrameMan::EndHudLayer(const Shader& backgroundShader) {
	// Ending the layer draws what's queued for it, before the shader goes back to drawing palette indices
	m_HudLayer->End();
	backgroundShader.Begin();
	backgroundShader.SetBool("drawingHudLayer", false);
	m_DrawingHudLayer = false;
	SetAlphaBlendMode();
	m_PlayerScreen->Begin(false);
}

void FrameMan::DrawWorldDump(bool drawForScenePreview) const {
	float worldBitmapWidth = static_cast<float>(m_WorldDumpBuffer->w);
	float worldBitmapHeight = static_cast<float>(m_WorldDumpBuffer->h);

	// Draw sky gradient if we're not dumping a scene preview
	if (!drawForScenePreview) {
		clear_to_color(m_WorldDumpBuffer.get(), makecol32(132, 192, 252)); // Light blue color
		for (int i = 0; i < m_WorldDumpBuffer->h; i++) {
			int lineColor = makecol32(64 + ((static_cast<float>(i) / worldBitmapHeight) * (128 - 64)), 64 + ((static_cast<float>(i) / worldBitmapHeight) * (192 - 64)), 96 + ((static_cast<float>(i) / worldBitmapHeight) * (255 - 96)));
			hline(m_WorldDumpBuffer.get(), 0, i, worldBitmapWidth - 1, lineColor);
		}
	} else {
		clear_to_color(m_WorldDumpBuffer.get(), makecol32(255, 0, 255)); // Magenta
	}

	// Draw scene
	draw_sprite(m_WorldDumpBuffer.get(), g_SceneMan.GetTerrain()->GetBGColorBitmap(), 0, 0);
	draw_sprite(m_WorldDumpBuffer.get(), g_SceneMan.GetTerrain()->GetFGColorBitmap(), 0, 0);

	// If we're not dumping a scene preview, draw objects and post-effects.
	if (!drawForScenePreview) {
		std::list<PostEffect> postEffectsList;
		BITMAP* effectBitmap = nullptr;
		int effectPosX = 0;
		int effectPosY = 0;
		int effectStrength = 0;
		Vector targetPos(0, 0);

		// Draw objects
		draw_sprite(m_WorldDumpBuffer.get(), g_SceneMan.GetMOColorBitmap(), 0, 0);

		// Draw post-effects
		g_PostProcessMan.GetPostScreenEffectsWrapped(targetPos, worldBitmapWidth, worldBitmapHeight, postEffectsList, -1);

		for (const PostEffect& postEffect: postEffectsList) {
			effectBitmap = postEffect.m_Bitmap;
			effectStrength = postEffect.m_Strength;
			set_screen_blender(effectStrength, effectStrength, effectStrength, effectStrength);
			effectPosX = postEffect.m_Pos.GetFloorIntX() - (effectBitmap->w / 2);
			effectPosY = postEffect.m_Pos.GetFloorIntY() - (effectBitmap->h / 2);

			if (postEffect.m_Angle == 0.0F) {
				draw_trans_sprite(m_WorldDumpBuffer.get(), effectBitmap, effectPosX, effectPosY);
			} else {
				BITMAP* targetBitmap = g_PostProcessMan.GetTempEffectBitmap(effectBitmap);
				clear_to_color(targetBitmap, 0);

				Matrix newAngle(postEffect.m_Angle);
				rotate_sprite(targetBitmap, effectBitmap, 0, 0, ftofix(newAngle.GetAllegroAngle()));
				draw_trans_sprite(m_WorldDumpBuffer.get(), targetBitmap, effectPosX, effectPosY);
			}
		}
	}
}

void FrameMan::LoadTextureFromBitmap8(Texture2D* tex, BITMAP* bm) {
	tex->width = bm->w;
	tex->height = bm->h;
	tex->mipmaps = 1;
	tex->format = PIXELFORMAT_UNCOMPRESSED_GRAYSCALE;

	tex->id = rlLoadTexture(
	    bm->line[0],
	    tex->width,
	    tex->height,
	    tex->format,
	    tex->mipmaps);

	rlTextureParameters(tex->id, RL_TEXTURE_MIN_FILTER, RL_TEXTURE_FILTER_NEAREST);
	rlTextureParameters(tex->id, RL_TEXTURE_MAG_FILTER, RL_TEXTURE_FILTER_NEAREST);
	rlTextureParameters(tex->id, RL_TEXTURE_WRAP_S, RL_TEXTURE_WRAP_REPEAT);
	rlTextureParameters(tex->id, RL_TEXTURE_WRAP_T, RL_TEXTURE_WRAP_REPEAT);
}