#include "Draw.h"
#include "GLStateMan.h"
#include "RenderMan.h"

using namespace RTE;
void DrawTexture(BITMAP* bitmap, int posX, int posY, RLColor tint) {
	DrawTexture(g_GLStateMan.GetStaticTextureFromBitmap(bitmap), posX, posY, tint);
}

void DrawTextureV(BITMAP* bitmap, Vector2 pos, RLColor tint) {
	DrawTextureV(g_GLStateMan.GetStaticTextureFromBitmap(bitmap), pos, tint);
}

void DrawTextureEx(BITMAP* bitmap, Vector2 pos, float rotation, float scale, RLColor tint) {
	DrawTextureEx(g_GLStateMan.GetStaticTextureFromBitmap(bitmap), pos, rotation, scale, tint);
}
void DrawTextureRec(BITMAP* bitmap, Rectangle source, Vector2 pos, RLColor tint) {
	DrawTextureRec(g_GLStateMan.GetStaticTextureFromBitmap(bitmap), source, pos, tint);
}
void DrawTexturePro(BITMAP* bitmap, Rectangle source, Rectangle dest, Vector2 origin, float rotation, RLColor tint) {
	DrawTexturePro(g_GLStateMan.GetStaticTextureFromBitmap(bitmap), source, dest, origin, rotation, tint);
}

namespace Draw {
	void DrawTexture(Texture* texture, int posX, int posY, Color tint) {
		DrawTexture(texture, Vector(posX, posY), tint);
	}

	void DrawTexture(Texture* texture, Vector pos, Color tint) {
		std::shared_ptr<DrawCall> draw = g_RenderMan.BeginDraw();
		draw->textureId = texture->GetTextureId();
		Shape::Shape rect = Shape::Rectangle(Box(pos, texture->GetDimensions().w, texture->GetDimensions().h), tint);
		draw->m_Vertices = std::move(rect.m_Vertices);
		draw->m_Indices = std::move(rect.m_Indices);
	}

} // namespace Draw
