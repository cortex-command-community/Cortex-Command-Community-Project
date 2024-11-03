#pragma once
#include "raylib/raylib.h"
#include "raylib/rlgl.h"

struct BITMAP;
namespace RTE {

	void DrawTexture(BITMAP* bitmap, int posX, int posY, RLColor tint);

	void DrawTextureV(BITMAP* bitmap, Vector2 pos, RLColor tint);

	void DrawTextureEx(BITMAP* bitmap, Vector2 pos, float rotation, float scale, RLColor tint);
	void DrawTextureRec(BITMAP* bitmap, Rectangle source, Vector2 pos, RLColor tint);
	void DrawTexturePro(BITMAP* bitmap, Rectangle source, Rectangle dest, Vector2 origin, float rotation, RLColor tint);
}