#pragma once
#include "raylib/raylib.h"
#include "raylib/rlgl.h"

struct BITMAP;
namespace RTE {

	/// Draw a bitmap to the screen at x, y tinted with tint.
	/// @param bitmap The bitmap to draw.
	/// @param posX The X position.
	/// @param posY The Y position.
	/// @param tint Tint color
	void DrawTexture(BITMAP* bitmap, int posX, int posY, RLColor tint);

	/// Draw a bitmap to the screen at x, y tinted with tint.
	/// @param bitmap The bitmap to draw.
	/// @param pos The position.
	/// @param tint Tint color
	void DrawTextureV(BITMAP* bitmap, Vector2 pos, RLColor tint);

	/// Draw a bitmap to the screen at x, y tinted with tint, rotated and scaled.
	/// @param bitmap The bitmap to draw.
	/// @param pos The position.
	/// @param rotation Rotation in Radians.
	/// @param scale Scale.
	/// @param tint Tint color
	void DrawTextureEx(BITMAP* bitmap, Vector2 pos, float rotation, float scale, RLColor tint);
	/// @brief Draw part of a bitmap to pos.
	/// @param bitmap The Bitmap
	/// @param source The Source rectangle inside bitmap.
	/// @param pos The target position.
	/// @param tint Tint color.
	void DrawTextureRec(BITMAP* bitmap, Rectangle source, Vector2 pos, RLColor tint);

	/// @brief Draw bitmap with more control.
	/// @param bitmap The bitmap.
	/// @param source The source rectangle.
	/// @param dest The destination rectangle (source will be scaled to dest)
	/// @param origin The pivot point, offset from dest.x, dest.y.
	/// @param rotation Rotation angle in radians.
	/// @param tint tint color.
	void DrawTexturePro(BITMAP* bitmap, Rectangle source, Rectangle dest, Vector2 origin, float rotation, RLColor tint);
}