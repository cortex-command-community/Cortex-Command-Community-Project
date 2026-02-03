#include "RenderMan.h"
#include "DrawCall.h"
#include "allegro.h"
#include "glad/gl.h"
#include "SDL3/SDL.h"

#include "FrameMan.h"
#include "tracy/Tracy.hpp"

using namespace RTE;

void RenderMan::Initialize() {
	std::unique_ptr<BITMAP, BitmapDeleter> shapesBitmap = std::unique_ptr<BITMAP, BitmapDeleter>(create_bitmap_ex(32, 1, 1));
	clear_to_color(shapesBitmap.get(), makeacol32(255, 255, 255, 255));
	m_ShapesTexture = std::make_unique<BitmapTexture>(std::move(shapesBitmap));
	m_RenderBatch = std::make_unique<RenderBatch>();
	m_DefaultShader = std::make_unique<Shader>("Base.rte/Shaders/Blit8.vert", "Base.rte/Shaders/Blit8.frag");
	std::unique_ptr<BITMAP, BitmapDeleter> paletteBitmap = std::unique_ptr<BITMAP, BitmapDeleter>(create_bitmap_ex(32, 256, 1));
	SDL_Palette* palette = ContentFile::DefaultPaletteToSDL(true);
	const PALETTE& pal = g_FrameMan.GetDefaultPalette();
	for (int i = 0; i < 256; ++i) {
		SDL_Color color = palette->colors[i];
		_putpixel32(paletteBitmap.get(), i, 0, makeacol32(color.r, color.g, color.b, color.a));
	}
	m_PaletteTexture = std::make_shared<BitmapTexture>(std::move(paletteBitmap), Filter::Nearest, WrapType::ClampToEdge);
}

void RenderMan::Destroy() {
}

std::shared_ptr<DrawCall> RenderMan::BeginDraw() {
	ZoneScoped;
	std::shared_ptr<DrawCall> drawCall = m_RenderBatch->m_DrawCalls.emplace_back(new DrawCall(m_RenderBatch->m_DrawCalls.size(), m_CurrentCamera));
	drawCall->m_Shader = m_RenderBatch->m_CurrentShader;
	drawCall->m_TextureId = m_ShapesTexture->GetTextureId();
	drawCall->m_BlendMode = m_RenderBatch->m_CurrentBlendMode;
	m_RenderBatch->m_CurrentDepth += RenderBatch::c_DrawDepthIncrement;
	return drawCall;
}

void RenderMan::BeginFrame(Camera* camera) {
	ZoneScoped;
	m_CurrentCamera = camera;
	m_RenderBatch->m_CurrentShader = m_DefaultShader.get();
	m_RenderBatch->m_CurrentBlendMode = Blend::ALPHA;
	m_RenderBatch->BeginFrame();
}

void RenderMan::DrawActiveBatch() {
	ZoneScoped;
	m_RenderBatch->Flush();
}
