#include "RenderMan.h"
#include "DrawCall.h"
#include "allegro.h"
#include "glad/gl.h"

using namespace RTE;

void RenderMan::Initialize() {
	std::unique_ptr<BITMAP, BitmapDeleter> bitmap = std::unique_ptr<BITMAP, BitmapDeleter>(create_bitmap_ex(32, 1, 1));
	clear_to_color(bitmap.get(), makeacol32(255, 255, 255, 255));
	m_ShapesTexture = std::make_unique<Texture>(std::move(bitmap));
}

void RenderMan::Destroy() {
}

std::shared_ptr<DrawCall> RenderMan::BeginDraw() {
	std::shared_ptr<DrawCall> drawCall = m_RenderBatch->m_DrawCalls.emplace_back(new DrawCall(m_RenderBatch->m_DrawCalls.size()));
	return drawCall;
}
