#include "Draw.h"
#include "GLResourceMan.h"

namespace RTE {
	void DrawTexture(BITMAP* bitmap, int posX, int posY, RLColor tint) {
		DrawTexture(g_GLResourceMan.GetStaticTextureFromBitmap(bitmap), posX, posY, tint);
	}

	void DrawTextureV(BITMAP* bitmap, Vector2 pos, RLColor tint) {
		DrawTextureV(g_GLResourceMan.GetStaticTextureFromBitmap(bitmap), pos, tint);
	}

	void DrawTextureEx(BITMAP* bitmap, Vector2 pos, float rotation, float scale, RLColor tint) {
		DrawTextureEx(g_GLResourceMan.GetStaticTextureFromBitmap(bitmap), pos, rotation, scale, tint);
	}
	void DrawTextureRec(BITMAP* bitmap, Rectangle source, Vector2 pos, RLColor tint) {
		DrawTextureRec(g_GLResourceMan.GetStaticTextureFromBitmap(bitmap), source, pos, tint);
	}
	void DrawTexturePro(BITMAP* bitmap, Rectangle source, Rectangle dest, Vector2 origin, float rotation, RLColor tint) {
		DrawTexturePro(g_GLResourceMan.GetStaticTextureFromBitmap(bitmap), source, dest, origin, rotation, tint);
	}
}