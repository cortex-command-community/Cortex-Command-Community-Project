#pragma once

#include "Singleton.h"
#include "GraphicalPrimitive.h"

#define g_PrimitiveMan PrimitiveMan::Instance()

namespace RTE {

	class Entity;

	/// Singleton manager responsible for all primitive drawing.
	class PrimitiveMan : public Singleton<PrimitiveMan> {

	public:
#pragma region Creation
		/// Constructor method used to instantiate a PrimitiveMan object in system memory.
		PrimitiveMan() { ClearPrimitivesQueue(); }
#pragma endregion

#pragma region Destruction
		/// Delete all scheduled primitives, called on every FrameMan sim update.
		void ClearPrimitivesQueue() { m_ScheduledPrimitives.clear(); }
#pragma endregion

#pragma region Primitive Drawing
		/// Draws all stored primitives on the screen for specified player.
		/// @param player Player to draw for.
		/// @param targetBitmap Bitmap to draw on.
		/// @param targetPos Position to draw.
		void DrawPrimitives(int player, BITMAP* targetBitmap, const Vector& targetPos) const;
#pragma endregion

#pragma region Primitive Draw Scheduling
		/// Schedule to draw multiple primitives of varying type with blending enabled.
		/// @param blendMode The blending mode to use when drawing each primitive.
		/// @param blendAmountR The blending amount for the Red channel. 0-100.
		/// @param blendAmountG The blending amount for the Green channel. 0-100.
		/// @param blendAmountB The blending amount for the Blue channel. 0-100.
		/// @param blendAmountA The blending amount for the Alpha channel. 0-100.
		/// @param primitives A vector of primitives to schedule drawing for.
		void SchedulePrimitivesForBlendedDrawing(DrawBlendMode blendMode, int blendAmountR, int blendAmountG, int blendAmountB, int blendAmountA, const std::vector<GraphicalPrimitive*>& primitives);

		/// Schedule to draw a line primitive.
		/// @param startPos Start position of primitive in scene coordinates.
		/// @param endPos End position of primitive in scene coordinates.
		/// @param color Color to draw primitive with.
		void DrawLinePrimitive(const Vector& startPos, const Vector& endPos, unsigned char color) { DrawLinePrimitive(-1, startPos, endPos, color, 1); }

		/// Schedule to draw a line primitive with the option to change thickness.
		/// @param startPos Start position of primitive in scene coordinates.
		/// @param endPos End position of primitive in scene coordinates.
		/// @param color Color to draw primitive with.
		/// @param thickness Thickness of the line in pixels.
		void DrawLinePrimitive(const Vector& startPos, const Vector& endPos, unsigned char color, int thickness) { DrawLinePrimitive(-1, startPos, endPos, color, thickness); }

		/// Schedule to draw a line primitive visible only to a specified player.
		/// @param player Player screen to draw primitive on.
		/// @param startPos Start position of primitive in scene coordinates.
		/// @param endPos End position of primitive in scene coordinates.
		/// @param color Color to draw primitive with.
		void DrawLinePrimitive(int player, const Vector& startPos, const Vector& endPos, unsigned char color) { DrawLinePrimitive(player, startPos, endPos, color, 1); }

		/// Schedule to draw a line primitive visible only to a specified player with the option to change thickness.
		/// @param player Player screen to draw primitive on.
		/// @param startPos Start position of primitive in scene coordinates.
		/// @param endPos End position of primitive in scene coordinates.
		/// @param color Color to draw primitive with.
		/// @param thickness Thickness of the line in pixels.
		void DrawLinePrimitive(int player, const Vector& startPos, const Vector& endPos, unsigned char color, int thickness);

		/// Schedule to draw an arc primitive.
		/// @param centerPos Position of primitive's center in scene coordinates.
		/// @param startAngle The angle from which the arc drawing begins.
		/// @param endAngle The angle at which the arc drawing ends.
		/// @param radius Radius of the arc primitive.
		/// @param color Color to draw primitive with.
		void DrawArcPrimitive(const Vector& centerPos, float startAngle, float endAngle, int radius, unsigned char color);

		/// Schedule to draw an arc primitive with the option to change thickness.
		/// @param centerPos Position of primitive's center in scene coordinates.
		/// @param startAngle The angle from which the arc drawing begins.
		/// @param endAngle The angle at which the arc drawing ends.
		/// @param radius Radius of the arc primitive.
		/// @param color Color to draw primitive with.
		/// @param thickness Thickness of the arc in pixels.
		void DrawArcPrimitive(const Vector& centerPos, float startAngle, float endAngle, int radius, unsigned char color, int thickness);

		/// Schedule to draw an arc primitive visible only to a specified player.
		/// @param player Player screen to draw primitive on.
		/// @param centerPos Position of primitive's center in scene coordinates.
		/// @param startAngle The angle from which the arc drawing begins.
		/// @param endAngle The angle at which the arc drawing ends.
		/// @param radius Radius of the arc primitive.
		/// @param color Color to draw primitive with.
		void DrawArcPrimitive(int player, const Vector& centerPos, float startAngle, float endAngle, int radius, unsigned char color);

		/// Schedule to draw an arc primitive visible only to a specified player with the option to change thickness.
		/// @param player Player screen to draw primitive on.
		/// @param centerPos Position of primitive's center in scene coordinates.
		/// @param startAngle The angle from which the arc drawing begins.
		/// @param endAngle The angle at which the arc drawing ends.
		/// @param radius Radius of the arc primitive.
		/// @param color Color to draw primitive with.
		/// @param thickness Thickness of the arc in pixels.
		void DrawArcPrimitive(int player, const Vector& centerPos, float startAngle, float endAngle, int radius, unsigned char color, int thickness);

		/// Schedule to draw a Bezier spline primitive.
		/// @param startPos Start position of primitive in scene coordinates.
		/// @param guideA The first guide point that controls the curve of the spline. The spline won't necessarily pass through this point, but it will affect it's shape.
		/// @param guideB The second guide point that controls the curve of the spline. The spline won't necessarily pass through this point, but it will affect it's shape.
		/// @param endPos End position of primitive in scene coordinates.
		/// @param color Color to draw primitive with.
		void DrawSplinePrimitive(const Vector& startPos, const Vector& guideA, const Vector& guideB, const Vector& endPos, unsigned char color);

		/// Schedule to draw a Bezier spline primitive visible only to a specified player.
		/// @param player Player screen to draw primitive on.
		/// @param startPos Start position of primitive in scene coordinates.
		/// @param guideA The first guide point that controls the curve of the spline. The spline won't necessarily pass through this point, but it will affect it's shape.
		/// @param guideB The second guide point that controls the curve of the spline. The spline won't necessarily pass through this point, but it will affect it's shape.
		/// @param endPos End position of primitive in scene coordinates.
		/// @param color Color to draw primitive with.
		void DrawSplinePrimitive(int player, const Vector& startPos, const Vector& guideA, const Vector& guideB, const Vector& endPos, unsigned char color);

		/// Schedule to draw a box primitive.
		/// @param topLeftPos Start position of primitive in scene coordinates. Top left corner.
		/// @param bottomRightPos End position of primitive in scene coordinates. Bottom right corner.
		/// @param color Color to draw primitive with.
		void DrawBoxPrimitive(const Vector& topLeftPos, const Vector& bottomRightPos, unsigned char color);

		/// Schedule to draw a box primitive visible only to a specified player.
		/// @param player Player screen to draw primitive on.
		/// @param topLeftPos Start position of primitive in scene coordinates. Top left corner.
		/// @param bottomRightPos End position of primitive in scene coordinates. Bottom right corner.
		/// @param color Color to draw primitive with.
		void DrawBoxPrimitive(int player, const Vector& topLeftPos, const Vector& bottomRightPos, unsigned char color);

		/// Schedule to draw a filled box primitive.
		/// @param topLeftPos Start position of primitive in scene coordinates. Top left corner.
		/// @param bottomRightPos End position of primitive in scene coordinates. Bottom right corner.
		/// @param color Color to draw primitive with.
		void DrawBoxFillPrimitive(const Vector& topLeftPos, const Vector& bottomRightPos, unsigned char color);

		/// Schedule to draw a filled box primitive visible only to a specified player.
		/// @param player Player screen to draw primitive on.
		/// @param topLeftPos Start position of primitive in scene coordinates. Top left corner.
		/// @param bottomRightPos End position of primitive in scene coordinates. Bottom right corner.
		/// @param color Color to draw primitive with.
		void DrawBoxFillPrimitive(int player, const Vector& topLeftPos, const Vector& bottomRightPos, unsigned char color);

		/// Schedule to draw a rounded box primitive.
		/// @param topLeftPos Start position of primitive in scene coordinates. Top left corner.
		/// @param bottomRightPos End position of primitive in scene coordinates. Bottom right corner.
		/// @param cornerRadius The radius of the corners of the box. Smaller radius equals sharper corners.
		/// @param color Color to draw primitive with.
		void DrawRoundedBoxPrimitive(const Vector& topLeftPos, const Vector& bottomRightPos, int cornerRadius, unsigned char color);

		/// Schedule to draw a rounded box primitive visible only to a specified player.
		/// @param player Player screen to draw primitive on.
		/// @param topLeftPos Start position of primitive in scene coordinates. Top left corner.
		/// @param bottomRightPos End position of primitive in scene coordinates. Bottom right corner.
		/// @param cornerRadius The radius of the corners of the box. Smaller radius equals sharper corners.
		/// @param color Color to draw primitive with.
		void DrawRoundedBoxPrimitive(int player, const Vector& topLeftPos, const Vector& bottomRightPos, int cornerRadius, unsigned char color);

		/// Schedule to draw a filled rounded box primitive.
		/// @param topLeftPos Start position of primitive in scene coordinates. Top left corner.
		/// @param bottomRightPos End position of primitive in scene coordinates. Bottom right corner.
		/// @param cornerRadius The radius of the corners of the box. Smaller radius equals sharper corners.
		/// @param color Color to draw primitive with.
		void DrawRoundedBoxFillPrimitive(const Vector& topLeftPos, const Vector& bottomRightPos, int cornerRadius, unsigned char color);

		/// Schedule to draw a filled rounded box primitive visible only to a specified player.
		/// @param player Player screen to draw primitive on.
		/// @param topLeftPos Start position of primitive in scene coordinates. Top left corner.
		/// @param bottomRightPos End position of primitive in scene coordinates. Bottom right corner.
		/// @param cornerRadius The radius of the corners of the box. Smaller radius equals sharper corners.
		/// @param color Color to draw primitive with.
		void DrawRoundedBoxFillPrimitive(int player, const Vector& topLeftPos, const Vector& bottomRightPos, int cornerRadius, unsigned char color);

		/// Schedule to draw a circle primitive.
		/// @param centerPos Position of primitive's center in scene coordinates.
		/// @param radius Radius of circle primitive.
		/// @param color Color to draw primitive with.
		void DrawCirclePrimitive(const Vector& centerPos, int radius, unsigned char color);

		/// Schedule to draw a circle primitive visible only to a specified player.
		/// @param player Player screen to draw primitive on.
		/// @param centerPos Position of primitive's center in scene coordinates.
		/// @param radius Radius of circle primitive.
		/// @param color Color to draw primitive with.
		void DrawCirclePrimitive(int player, const Vector& centerPos, int radius, unsigned char color);

		/// Schedule to draw a filled circle primitive.
		/// @param centerPos Position of primitive's center in scene coordinates.
		/// @param radius Radius of circle primitive.
		/// @param color Color to fill primitive with.
		void DrawCircleFillPrimitive(const Vector& centerPos, int radius, unsigned char color);

		/// Schedule to draw a filled circle primitive visible only to a specified player.
		/// @param player Player screen to draw primitive on.
		/// @param centerPos Position of primitive's center in scene coordinates.
		/// @param radius Radius of circle primitive.
		/// @param color Color to fill primitive with.
		void DrawCircleFillPrimitive(int player, const Vector& centerPos, int radius, unsigned char color);

		/// Schedule to draw an ellipse primitive.
		/// @param centerPos Position of primitive's center in scene coordinates.
		/// @param horizRadius Horizontal radius of the ellipse primitive.
		/// @param vertRadius Vertical radius of the ellipse primitive.
		/// @param color Color to draw primitive with.
		void DrawEllipsePrimitive(const Vector& centerPos, int horizRadius, int vertRadius, unsigned char color);

		/// Schedule to draw an ellipse primitive visible only to a specified player.
		/// @param player Player screen to draw primitive on.
		/// @param centerPos Position of primitive's center in scene coordinates.
		/// @param horizRadius Horizontal radius of the ellipse primitive.
		/// @param vertRadius Vertical radius of the ellipse primitive.
		/// @param color Color to draw primitive with.
		void DrawEllipsePrimitive(int player, const Vector& centerPos, int horizRadius, int vertRadius, unsigned char color);

		/// Schedule to draw a filled ellipse primitive.
		/// @param centerPos Position of primitive's center in scene coordinates.
		/// @param horizRadius Horizontal radius of the ellipse primitive.
		/// @param vertRadius Vertical radius of the ellipse primitive.
		/// @param color Color to fill primitive with.
		void DrawEllipseFillPrimitive(const Vector& centerPos, int horizRadius, int vertRadius, unsigned char color);

		/// Schedule to draw a filled ellipse primitive visible only to a specified player.
		/// @param player Player screen to draw primitive on.
		/// @param centerPos Position of primitive's center in scene coordinates.
		/// @param horizRadius Horizontal radius of the ellipse primitive.
		/// @param vertRadius Vertical radius of the ellipse primitive.
		/// @param color Color to fill primitive with.
		void DrawEllipseFillPrimitive(int player, const Vector& centerPos, int horizRadius, int vertRadius, unsigned char color);

		/// Schedule to draw a triangle primitive.
		/// @param pointA Position of the first point of the triangle in scene coordinates.
		/// @param pointB Position of the second point of the triangle in scene coordinates.
		/// @param pointC Position of the third point of the triangle in scene coordinates.
		/// @param color Color to fill primitive with.
		void DrawTrianglePrimitive(const Vector& pointA, const Vector& pointB, const Vector& pointC, unsigned char color);

		/// Schedule to draw a triangle primitive visible only to a specified player.
		/// @param player Player screen to draw primitive on.
		/// @param pointA Position of the first point of the triangle in scene coordinates.
		/// @param pointB Position of the second point of the triangle in scene coordinates.
		/// @param pointC Position of the third point of the triangle in scene coordinates.
		/// @param color Color to fill primitive with.
		void DrawTrianglePrimitive(int player, const Vector& pointA, const Vector& pointB, const Vector& pointC, unsigned char color);

		/// Schedule to draw a filled triangle primitive.
		/// @param pointA Position of the first point of the triangle in scene coordinates.
		/// @param pointB Position of the second point of the triangle in scene coordinates.
		/// @param pointC Position of the third point of the triangle in scene coordinates.
		/// @param color Color to fill primitive with.
		void DrawTriangleFillPrimitive(const Vector& pointA, const Vector& pointB, const Vector& pointC, unsigned char color);

		/// Schedule to draw a filled triangle primitive visible only to a specified player.
		/// @param player Player screen to draw primitive on.
		/// @param pointA Position of the first point of the triangle in scene coordinates.
		/// @param pointB Position of the second point of the triangle in scene coordinates.
		/// @param pointC Position of the third point of the triangle in scene coordinates.
		/// @param color Color to fill primitive with.
		void DrawTriangleFillPrimitive(int player, const Vector& pointA, const Vector& pointB, const Vector& pointC, unsigned char color);

		/// Schedule to draw a polygon primitive visible only to a specified player.
		/// @param player Player screen to draw primitive on, or -1 for all players.
		/// @param startPos Start position of the primitive in scene coordinates.
		/// @param color Color to draw primitive with.
		/// @param vertices A vector containing the positions of the vertices of the polygon, relative to the center position.
		/// @param filled Whether a PolygonFillPrimitive should be scheduled instead of PolygonPrimitive.
		void DrawPolygonOrPolygonFillPrimitive(int player, const Vector& startPos, unsigned char color, const std::vector<Vector*>& vertices, bool filled);

		/// Schedule to draw a text primitive.
		/// @param start Start position of primitive in scene coordinates.
		/// @param text Text string to draw.
		/// @param isSmall Use small or large font. True for small font.
		/// @param alignment Alignment of text.
		void DrawTextPrimitive(const Vector& start, const std::string& text, bool isSmall, int alignment);

		/// Schedule to draw a text primitive.
		/// @param start Start position of primitive in scene coordinates.
		/// @param text Text string to draw.
		/// @param isSmall Use small or large font. True for small font.
		/// @param alignment Alignment of text.
		/// @param rotAngle Angle to rotate text in radians.
		void DrawTextPrimitive(const Vector& start, const std::string& text, bool isSmall, int alignment, float rotAngle);

		/// Schedule to draw a text primitive visible only to a specified player.
		/// @param player Player screen to draw primitive on.
		/// @param start Start position of primitive in scene coordinates.
		/// @param text Text string to draw.
		/// @param isSmall Use small or large font. True for small font.
		/// @param alignment Alignment of text.
		void DrawTextPrimitive(int player, const Vector& start, const std::string& text, bool isSmall, int alignment);

		/// Schedule to draw a text primitive visible only to a specified player.
		/// @param player Player screen to draw primitive on.
		/// @param start Start position of primitive in scene coordinates.
		/// @param text Text string to draw.
		/// @param isSmall Use small or large font. True for small font.
		/// @param alignment Alignment of text.
		/// @param rotAngle Angle to rotate text in radians.
		void DrawTextPrimitive(int player, const Vector& start, const std::string& text, bool isSmall, int alignment, float rotAngle);

		/// Schedule to draw a bitmap primitive.
		/// @param centerPos Position of primitive's center in scene coordinates.
		/// @param moSprite A MOSprite to draw BITMAP from.
		/// @param rotAngle Rotation angle in radians.
		/// @param frame Frame to draw.
		void DrawBitmapPrimitive(const Vector& centerPos, const MOSprite* moSprite, float rotAngle, unsigned int frame) { DrawBitmapPrimitive(-1, centerPos, moSprite, rotAngle, frame, false, false); }

		/// Schedule to draw a bitmap primitive with the option to flip the primitive horizontally and vertically.
		/// @param centerPos Position of primitive's center in scene coordinates.
		/// @param moSprite A MOSprite to draw BITMAP from.
		/// @param rotAngle Rotation angle in radians.
		/// @param frame Frame to draw.
		/// @param hFlipped Whether to flip the sprite horizontally.
		/// @param vFlipped Whether to flip the sprite vertically.
		void DrawBitmapPrimitive(const Vector& centerPos, const MOSprite* moSprite, float rotAngle, unsigned int frame, bool hFlipped, bool vFlipped) { DrawBitmapPrimitive(-1, centerPos, moSprite, rotAngle, frame, hFlipped, vFlipped); }

		/// Schedule to draw a bitmap primitive visible only to a specified player.
		/// @param player Player screen to draw primitive on.
		/// @param centerPos Position of primitive's center in scene coordinates.
		/// @param moSprite A MOSprite to draw BITMAP from.
		/// @param rotAngle Rotation angle in radians.
		/// @param frame Frame to draw.
		void DrawBitmapPrimitive(int player, const Vector& centerPos, const MOSprite* moSprite, float rotAngle, unsigned int frame) { DrawBitmapPrimitive(player, centerPos, moSprite, rotAngle, frame, false, false); }

		/// Schedule to draw a bitmap primitive visible only to a specified player with the option to flip the primitive horizontally or vertically.
		/// @param player Player screen to draw primitive on.
		/// @param centerPos Position of primitive's center in scene coordinates.
		/// @param moSprite A MOSprite to draw BITMAP from.
		/// @param rotAngle Rotation angle in radians.
		/// @param frame Frame to draw.
		/// @param hFlipped Whether to flip the sprite horizontally.
		/// @param vFlipped Whether to flip the sprite vertically.
		void DrawBitmapPrimitive(int player, const Vector& centerPos, const MOSprite* moSprite, float rotAngle, unsigned int frame, bool hFlipped, bool vFlipped);

		/// Schedule to draw a bitmap primitive.
		/// @param centerPos Position of primitive's center in scene coordinates.
		/// @param filePath Path to the bitmap to draw.
		/// @param rotAngle Rotation angle in radians.
		void DrawBitmapPrimitive(const Vector& centerPos, const std::string& filePath, float rotAngle) { DrawBitmapPrimitive(-1, centerPos, filePath, rotAngle, false, false); }

		/// Schedule to draw a bitmap primitive with the option to flip the primitive horizontally and vertically.
		/// @param centerPos Position of primitive's center in scene coordinates.
		/// @param filePath An entity to draw sprite from.
		/// @param rotAngle Rotation angle in radians.
		/// @param hFlipped Whether to flip the sprite horizontally.
		/// @param vFlipped Whether to flip the sprite vertically.
		void DrawBitmapPrimitive(const Vector& centerPos, const std::string& filePath, float rotAngle, bool hFlipped, bool vFlipped) { DrawBitmapPrimitive(-1, centerPos, filePath, rotAngle, hFlipped, vFlipped); }

		/// Schedule to draw a bitmap primitive visible only to a specified player.
		/// @param player Player screen to draw primitive on.
		/// @param centerPos Position of primitive's center in scene coordinates.
		/// @param filePath Path to the bitmap to draw.
		/// @param rotAngle Rotation angle in radians.
		void DrawBitmapPrimitive(int player, const Vector& centerPos, const std::string& filePath, float rotAngle) { DrawBitmapPrimitive(player, centerPos, filePath, rotAngle, false, false); }

		/// Schedule to draw a bitmap primitive visible only to a specified player with the option to flip the primitive horizontally or vertically.
		/// @param player Player screen to draw primitive on.
		/// @param centerPos Position of primitive's center in scene coordinates.
		/// @param filePath Path to the bitmap to draw.
		/// @param rotAngle Rotation angle in radians.
		/// @param hFlipped Whether to flip the sprite horizontally.
		/// @param vFlipped Whether to flip the sprite vertically.
		void DrawBitmapPrimitive(int player, const Vector& centerPos, const std::string& filePath, float rotAngle, bool hFlipped, bool vFlipped);

		/// Schedule to draw the GUI icon of an object.
		/// @param centerPos Position of primitive's center in scene coordinates.
		/// @param entity An entity to draw sprite from.
		void DrawIconPrimitive(const Vector& centerPos, Entity* entity) { DrawIconPrimitive(-1, centerPos, entity); }

		/// Schedule to draw the GUI icon of an object, visible only to a specified player.
		/// @param player Player screen to draw primitive on.
		/// @param centerPos Position of primitive's center in scene coordinates.
		/// @param entity An entity to draw sprite from.
		void DrawIconPrimitive(int player, const Vector& centerPos, Entity* entity);
#pragma endregion

	protected:
		std::mutex m_Mutex; //!< Mutex so that mutiple threads (i.e multithreaded scripts) can safely queue up draws
		std::deque<std::unique_ptr<GraphicalPrimitive>> m_ScheduledPrimitives; //!< List of graphical primitives scheduled to draw this frame, cleared every frame during FrameMan::Draw().

	private:
		/// Constructs a unique_ptr of the appropriate derived type from the passed in GraphicalPrimitive raw pointer.
		/// This is used for preparing primitives constructed in Lua for scheduling.
		/// @param primitive Raw pointer to the GraphicalPrimitive object to make unique.
		/// @return A unique_ptr of the appropriate derived GraphicalPrimitive type. Ownership is transferred!
		std::unique_ptr<GraphicalPrimitive> MakeUniqueOfAppropriateTypeFromPrimitiveRawPtr(GraphicalPrimitive* primitive);

		/// Safely schedules a primite to draw in a thread-safe manner.
		/// @param primitive A unique ptr of the primitive to schedule for drawing.
		void SchedulePrimitive(std::unique_ptr<GraphicalPrimitive>&& primitive);

		// Disallow the use of some implicit methods.
		PrimitiveMan(const PrimitiveMan& reference) = delete;
		PrimitiveMan& operator=(const PrimitiveMan& rhs) = delete;
	};
} // namespace RTE
