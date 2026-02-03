#pragma once
#include "raylib/raylib.h"
#include "raylib/rlgl.h"
#include "Camera.h"
#include "Vertex.h"
#include "Shapes.h"
#include "Texture.h"
#include "Rectangles.h"

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

	namespace Draw {
		std::shared_ptr<DrawCall> DrawTexture(Texture* texture, float posX, float posY, Color tint = {255, 255, 255, 255});
		std::shared_ptr<DrawCall> DrawTexture(Texture* texture, glm::vec2 pos, Color tint = {255, 255, 255, 255});
		std::shared_ptr<DrawCall> DrawTexture(Texture* texture, FloatRect dest, Color tint = {255, 255, 255, 255});
		std::shared_ptr<DrawCall> DrawTexture(Texture* texture, glm::vec2 pos, glm::vec2 origin, float angle, glm::vec2 scale, Color tint = {255, 255, 255, 255});
		std::shared_ptr<DrawCall> DrawTexture(Texture* texture, FloatRect source, glm::vec2 pos, Color tint);
		std::shared_ptr<DrawCall> DrawTexture(Texture* texture, const FloatRect& source, const FloatRect& dest, const Color& tint = {255, 255, 255, 255});
		std::shared_ptr<DrawCall> DrawTexture(Texture* bitmap, FloatRect source, FloatRect dest, glm::vec2 origin, float rotation, Color tint);
	}
}
