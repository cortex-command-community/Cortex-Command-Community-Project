#pragma once

#include "MOSprite.h"

#include <array>
#include <string>
#include <vector>

namespace RTE {

#pragma region Graphical Primitive
	/// Class used to schedule drawing of graphical primitives created from Lua.
	/// All coordinates passed to GraphicalPrimitive objects are Scene coordinates.
	class GraphicalPrimitive {

	public:
/// Convenience macro to cut down on duplicate methods in classes that extend GraphicalPrimitive.
#define GraphicalPrimitiveOverrideMethods \
	const PrimitiveType GetPrimitiveType() const override { return c_PrimitiveType; } \
	void Draw(BITMAP* drawScreen, const Vector& targetPos) override;

		/// Enumeration of the different primitive types derived from GraphicalPrimitive.
		enum class PrimitiveType {
			None, // Not derived (base GraphicalPrimitive).
			Line,
			Arc,
			Spline,
			Box,
			BoxFill,
			RoundedBox,
			RoundedBoxFill,
			Circle,
			CircleFill,
			Ellipse,
			EllipseFill,
			Triangle,
			TriangleFill,
			Polygon,
			PolygonFill,
			Text,
			Bitmap
		};

		Vector m_StartPos; //!< Start position of the primitive.
		Vector m_EndPos; //!< End position of the primitive.
		unsigned char m_Color = 0; //!< Color to draw this primitive with.
		int m_Player = -1; //!< Player screen to draw this primitive on.
		DrawBlendMode m_BlendMode = DrawBlendMode::NoBlend; //!< The blending mode that will be used when drawing this primitive.
		std::array<int, 4> m_ColorChannelBlendAmounts = {BlendAmountLimits::MinBlend, BlendAmountLimits::MinBlend, BlendAmountLimits::MinBlend, BlendAmountLimits::MinBlend}; //!< The blending amount for each color channel when drawing in blended mode.

		/// Destructor method used to clean up a GraphicalPrimitive object before deletion from system memory.
		virtual ~GraphicalPrimitive() = default;

		/// Translates coordinates from scene to this bitmap offset producing two coordinates.
		/// @param targetPos Target position.
		/// @param scenePos Position on scene.
		/// @param drawLeftPos 'Left' position of bitmap on scene with negative values as if scene seam is 0,0.
		/// @param drawRightPos 'Right' position of bitmap on scene with positive values.
		/// @remark
		/// Unfortunately it's hard to explain how this works. It tries to represent scene bitmap as two parts with center in 0,0.
		/// Right part is just plain visible part with coordinates from [0, scenewidth] and left part is imaginary bitmap as if we traversed it across the seam right-to-left with coordinates [0, -scenewidth].
		/// So in order to be drawn each screen coordinates calculated twice for left and right 'bitmaps' and then one of them either flies away off-screen or gets drawn on the screen.
		/// When we cross the seam either left or right part is actually drawn in the bitmap, and negative coordinates of right part are compensated by view point offset coordinates when we cross the seam right to left.
		/// I really don't know how to make it simpler, because it has so many special cases and simply wrapping all out-of-the scene coordinates don't work because this way nothing will be ever draw across the seam.
		/// You're welcome to rewrite this nightmare if you can, I wasted a whole week on this (I can admit that I'm just too dumb for this) )))
		void TranslateCoordinates(Vector targetPos, const Vector& scenePos, Vector& drawLeftPos, Vector& drawRightPos) const;

		/// Draws this primitive on provided bitmap.
		/// @param drawScreen Bitmap to draw on.
		/// @param targetPos Position of graphical primitive.
		virtual void Draw(BITMAP* drawScreen, const Vector& targetPos) = 0;

		/// Gets the type identifier of this primitive.
		/// @return The type identifier of this primitive.
		virtual const PrimitiveType GetPrimitiveType() const = 0;

	private:
		static const PrimitiveType c_PrimitiveType; //!< Type identifier of this primitive.
	};
#pragma endregion

#pragma region Line Primitive
	/// Class used to schedule drawing of line primitives created from Lua.
	class LinePrimitive : public GraphicalPrimitive {

	public:
		GraphicalPrimitiveOverrideMethods;

		/// Constructor method for LinePrimitive object.
		/// @param player Player screen to draw this primitive on.
		/// @param startPos Start position of the primitive.
		/// @param end End position of the primitive.
		/// @param color Color to draw this primitive with.
		LinePrimitive(int player, const Vector& startPos, const Vector& endPos, unsigned char color) {
			m_StartPos = startPos;
			m_EndPos = endPos;
			m_Color = color;
			m_Player = player;
		}

	private:
		static const PrimitiveType c_PrimitiveType; //!< Type identifier of this primitive.
	};
#pragma endregion

#pragma region Arc Primitive
	/// Class used to schedule drawing of arc line primitives created from Lua.
	class ArcPrimitive : public GraphicalPrimitive {

	public:
		GraphicalPrimitiveOverrideMethods;

		float m_StartAngle = 0; //!< The angle from which the arc begins.
		float m_EndAngle = 0; //!< The angle at which the arc ends.
		int m_Radius = 0; //!< Radius of the arc primitive.
		int m_Thickness = 0; //!< Thickness of the arc primitive in pixels.

		/// Constructor method for ArcPrimitive object.
		/// @param player Player screen to draw this primitive on.
		/// @param centerPos Position of this primitive's center.
		/// @param startAngle The angle from which the arc drawing begins.
		/// @param endAngle The angle at which the arc drawing ends.
		/// @param radius Radius of the arc primitive.
		/// @param color Color to draw this primitive with.
		ArcPrimitive(int player, const Vector& centerPos, float startAngle, float endAngle, int radius, int thickness, unsigned char color) :
		    m_StartAngle(startAngle), m_EndAngle(endAngle), m_Radius(radius), m_Thickness(thickness) {

			m_StartPos = centerPos;
			m_Color = color;
			m_Player = player;
		}

	private:
		static const PrimitiveType c_PrimitiveType; //!< Type identifier of this primitive.
	};
#pragma endregion

#pragma region Spline Primitive
	/// Class used to schedule drawing of spline primitives created from Lua.
	class SplinePrimitive : public GraphicalPrimitive {

	public:
		GraphicalPrimitiveOverrideMethods;

		Vector m_GuidePointAPos; //!< A guide point that controls the curve of the spline.
		Vector m_GuidePointBPos; //!< A guide point that controls the curve of the spline.

		/// Constructor method for SplinePrimitive object.
		/// @param player Player screen to draw this primitive on.
		/// @param startPos Start position of the primitive.
		/// @param guideA The first guide point that controls the curve of the spline. The spline won't necessarily pass through this point, but it will affect it's shape.
		/// @param guideB The second guide point that controls the curve of the spline. The spline won't necessarily pass through this point, but it will affect it's shape.
		/// @param endPos End position of the primitive.
		/// @param color Color to draw this primitive with.
		SplinePrimitive(int player, const Vector& startPos, const Vector& guideA, const Vector& guideB, const Vector& endPos, unsigned char color) :
		    m_GuidePointAPos(guideA), m_GuidePointBPos(guideB) {

			m_StartPos = startPos;
			m_EndPos = endPos;
			m_Color = color;
			m_Player = player;
		}

	private:
		static const PrimitiveType c_PrimitiveType; //!< Type identifier of this primitive.
	};
#pragma endregion

#pragma region Box Primitive
	/// Class used to schedule drawing of box primitives created from Lua.
	class BoxPrimitive : public GraphicalPrimitive {

	public:
		GraphicalPrimitiveOverrideMethods;

		/// Constructor method for BoxPrimitive object.
		/// @param player Player screen to draw this primitive on.
		/// @param topLeftPos Start position of the primitive. Top left corner.
		/// @param bottomRightPos End position of the primitive. Bottom right corner.
		/// @param color Color to draw this primitive with.
		BoxPrimitive(int player, const Vector& topLeftPos, const Vector& bottomRightPos, unsigned char color) {
			m_StartPos = topLeftPos;
			m_EndPos = bottomRightPos;
			m_Color = color;
			m_Player = player;
		}

	private:
		static const PrimitiveType c_PrimitiveType; //!< Type identifier of this primitive.
	};
#pragma endregion

#pragma region Filled Box Primitive
	/// Class used to schedule drawing of filled box primitives created from Lua.
	class BoxFillPrimitive : public GraphicalPrimitive {

	public:
		GraphicalPrimitiveOverrideMethods;

		/// Constructor method for BoxFillPrimitive object.
		/// @param player Player screen to draw this primitive on.
		/// @param topLeftPos Start position of the primitive. Top left corner.
		/// @param bottomRightPos End position of the primitive. Bottom right corner.
		/// @param color Color to draw this primitive with.
		BoxFillPrimitive(int player, const Vector& topLeftPos, const Vector& bottomRightPos, unsigned char color) {
			m_StartPos = topLeftPos;
			m_EndPos = bottomRightPos;
			m_Color = color;
			m_Player = player;
		}

	private:
		static const PrimitiveType c_PrimitiveType; //!< Type identifier of this primitive.
	};
#pragma endregion

#pragma region Rounded Box Primitive
	/// Class used to schedule drawing of box with round corners primitives created from Lua.
	class RoundedBoxPrimitive : public GraphicalPrimitive {

	public:
		GraphicalPrimitiveOverrideMethods;

		int m_CornerRadius = 0; //!< The radius of the corners of the box.

		/// Constructor method for RoundedBoxPrimitive object.
		/// @param player Player screen to draw this primitive on.
		/// @param topLeftPos Start position of the primitive. Top left corner.
		/// @param bottomRightPos End position of the primitive. Bottom right corner.
		/// @param cornerRadius The radius of the corners of the box. Smaller radius equals sharper corners.
		/// @param color Color to draw this primitive with.
		RoundedBoxPrimitive(int player, const Vector& topLeftPos, const Vector& bottomRightPos, int cornerRadius, unsigned char color) :
		    m_CornerRadius(cornerRadius) {

			m_StartPos = topLeftPos;
			m_EndPos = bottomRightPos;
			m_Color = color;
			m_Player = player;
		}

	private:
		static const PrimitiveType c_PrimitiveType; //!< Type identifier of this primitive.
	};
#pragma endregion

#pragma region Filled Rounded Box Primitive
	/// Class used to schedule drawing of filled box with round corners primitives created from Lua.
	class RoundedBoxFillPrimitive : public GraphicalPrimitive {

	public:
		GraphicalPrimitiveOverrideMethods;

		int m_CornerRadius = 0; //!< The radius of the corners of the box.

		/// Constructor method for RoundedBoxFillPrimitive object.
		/// @param player Player screen to draw this primitive on.
		/// @param topLeftPos Start position of the primitive. Top left corner.
		/// @param bottomRightPos End position of the primitive. Bottom right corner.
		/// @param cornerRadius The radius of the corners of the box. Smaller radius equals sharper corners.
		/// @param color Color to draw this primitive with.
		RoundedBoxFillPrimitive(int player, const Vector& topLeftPos, const Vector& bottomRightPos, int cornerRadius, unsigned char color) :
		    m_CornerRadius(cornerRadius) {

			m_StartPos = topLeftPos;
			m_EndPos = bottomRightPos;
			m_Color = color;
			m_Player = player;
		}

	private:
		static const PrimitiveType c_PrimitiveType; //!< Type identifier of this primitive.
	};
#pragma endregion

#pragma region Cicrcle Primitive
	/// Class used to schedule drawing of circle primitives created from Lua.
	class CirclePrimitive : public GraphicalPrimitive {

	public:
		GraphicalPrimitiveOverrideMethods;

		int m_Radius = 0; //!< Radius of the circle primitive.

		/// Constructor method for CirclePrimitive object.
		/// @param player Player screen to draw this primitive on.
		/// @param centerPos Position of this primitive's center.
		/// @param radius Radius of the circle primitive.
		/// @param color Color to draw this primitive with.
		CirclePrimitive(int player, const Vector& centerPos, int radius, unsigned char color) :
		    m_Radius(radius) {

			m_StartPos = centerPos;
			m_Color = color;
			m_Player = player;
		}

	private:
		static const PrimitiveType c_PrimitiveType; //!< Type identifier of this primitive.
	};
#pragma endregion

#pragma region Filled Circle Primitive
	/// Class used to schedule drawing of filled circle primitives created from Lua
	class CircleFillPrimitive : public GraphicalPrimitive {

	public:
		GraphicalPrimitiveOverrideMethods;

		int m_Radius = 0; //!< Radius of the circle primitive.

		/// Constructor method for CircleFillPrimitive object.
		/// @param player Player screen to draw this primitive on.
		/// @param centerPos Position of this primitive's center.
		/// @param radius Radius of the circle primitive.
		/// @param color Color to draw this primitive with.
		CircleFillPrimitive(int player, const Vector& centerPos, int radius, unsigned char color) :
		    m_Radius(radius) {

			m_StartPos = centerPos;
			m_Color = color;
			m_Player = player;
		}

	private:
		static const PrimitiveType c_PrimitiveType; //!< Type identifier of this primitive.
	};
#pragma endregion

#pragma region Ellipse Primitive
	/// Class used to schedule drawing of ellipse primitives created from Lua.
	class EllipsePrimitive : public GraphicalPrimitive {

	public:
		GraphicalPrimitiveOverrideMethods;

		int m_HorizRadius = 0; //!< The horizontal radius of the ellipse primitive.
		int m_VertRadius = 0; //!< The vertical radius of the ellipse primitive.

		/// Constructor method for EllipsePrimitive object.
		/// @param player Player screen to draw this primitive on.
		/// @param centerPos Position of this primitive's center.
		/// @param horizRadius Horizontal radius of the ellipse primitive.
		/// @param vertRadius Vertical radius of the ellipse primitive.
		/// @param color Color to draw this primitive with.
		EllipsePrimitive(int player, const Vector& centerPos, int horizRadius, int vertRadius, unsigned char color) :
		    m_HorizRadius(horizRadius), m_VertRadius(vertRadius) {

			m_StartPos = centerPos;
			m_Color = color;
			m_Player = player;
		}

	private:
		static const PrimitiveType c_PrimitiveType; //!< Type identifier of this primitive.
	};
#pragma endregion

#pragma region Filled Ellipse Primitive
	/// Class used to schedule drawing of filled ellipse primitives created from Lua
	class EllipseFillPrimitive : public GraphicalPrimitive {

	public:
		GraphicalPrimitiveOverrideMethods;

		int m_HorizRadius = 0; //!< The horizontal radius of the ellipse primitive.
		int m_VertRadius = 0; //!< The vertical radius of the ellipse primitive.

		/// Constructor method for EllipseFillPrimitive object.
		/// @param player Player screen to draw this primitive on.
		/// @param centerPos Position of this primitive's center.
		/// @param radius Radius of the circle primitive.
		/// @param color Color to draw this primitive with.
		EllipseFillPrimitive(int player, const Vector& centerPos, int horizRadius, int vertRadius, unsigned char color) :
		    m_HorizRadius(horizRadius), m_VertRadius(vertRadius) {

			m_StartPos = centerPos;
			m_Color = color;
			m_Player = player;
		}

	private:
		static const PrimitiveType c_PrimitiveType; //!< Type identifier of this primitive.
	};
#pragma endregion

#pragma region Triangle Primitive
	/// Class used to schedule drawing of triangle primitives created from Lua.
	class TrianglePrimitive : public GraphicalPrimitive {

	public:
		GraphicalPrimitiveOverrideMethods;

		Vector m_PointAPos; //!< First point of the triangle.
		Vector m_PointBPos; //!< Second point of the triangle.
		Vector m_PointCPos; //!< Third point of the triangle.

		/// Constructor method for TrianglePrimitive object.
		/// @param player Player screen to draw this primitive on.
		/// @param pointA Position of the first point of the triangle.
		/// @param pointB Position of the second point of the triangle
		/// @param pointC Position of the third point of the triangle
		/// @param color Color to draw this primitive with.
		TrianglePrimitive(int player, const Vector& pointA, const Vector& pointB, const Vector& pointC, unsigned char color) :
		    m_PointAPos(pointA), m_PointBPos(pointB), m_PointCPos(pointC) {

			m_Color = color;
			m_Player = player;
		}

	private:
		static const PrimitiveType c_PrimitiveType; //!< Type identifier of this primitive.
	};
#pragma endregion

#pragma region Filled Triangle Primitive
	/// Class used to schedule drawing of filled triangle primitives created from Lua.
	class TriangleFillPrimitive : public GraphicalPrimitive {

	public:
		GraphicalPrimitiveOverrideMethods;

		Vector m_PointAPos; //!< First point of the triangle.
		Vector m_PointBPos; //!< Second point of the triangle.
		Vector m_PointCPos; //!< Third point of the triangle.

		/// Constructor method for TriangleFillPrimitive object.
		/// @param player Player screen to draw this primitive on.
		/// @param pointA Position of the first point of the triangle.
		/// @param pointB Position of the second point of the triangle
		/// @param pointC Position of the third point of the triangle
		/// @param color Color to draw this primitive with.
		TriangleFillPrimitive(int player, const Vector& pointA, const Vector& pointB, const Vector& pointC, unsigned char color) :
		    m_PointAPos(pointA), m_PointBPos(pointB), m_PointCPos(pointC) {

			m_Color = color;
			m_Player = player;
		}

	private:
		static const PrimitiveType c_PrimitiveType; //!< Type identifier of this primitive.
	};
#pragma endregion

#pragma region Polygon Primitive
	/// Class used to schedule drawing of polygon primitives created from Lua.
	class PolygonPrimitive : public GraphicalPrimitive {

	public:
		GraphicalPrimitiveOverrideMethods;

		std::vector<Vector*> m_Vertices = {}; //!< Positions of the vertices of the polygon, relative to the center position.

		/// Constructor method for PolygonPrimitive object.
		/// @param player Player screen to draw this primitive on.
		/// @param startPos Start position of the primitive.
		/// @param vertices A vector containing the positions of the vertices of the polygon, relative to the center position.
		/// @param color Color to draw this primitive with.
		PolygonPrimitive(int player, const Vector& startPos, unsigned char color, const std::vector<Vector*>& vertices) :
		    m_Vertices(vertices) {

			m_StartPos = startPos;
			m_Color = color;
			m_Player = player;
		}

	private:
		static const PrimitiveType c_PrimitiveType; //!< Type identifier of this primitive.
	};
#pragma endregion

#pragma region Filled Polygon Primitive
	/// Class used to schedule drawing of filled polygon primitives created from Lua.
	class PolygonFillPrimitive : public GraphicalPrimitive {

	public:
		GraphicalPrimitiveOverrideMethods;

		std::vector<Vector*> m_Vertices = {}; //!< Positions of the vertices of the polygon, relative to the center position.

		/// Constructor method for PolygonFillPrimitive object.
		/// @param player Player screen to draw this primitive on.
		/// @param startPos Start position of the primitive.
		/// @param vertices A vector containing the positions of the vertices of the polygon, relative to the center position.
		/// @param color Color to draw this primitive with.
		PolygonFillPrimitive(int player, const Vector& startPos, unsigned char color, const std::vector<Vector*>& vertices) :
		    m_Vertices(vertices) {

			m_StartPos = startPos;
			m_Color = color;
			m_Player = player;
		}

	private:
		static const PrimitiveType c_PrimitiveType; //!< Type identifier of this primitive.
	};
#pragma endregion

#pragma region Text Primitive
	/// Class used to schedule drawing of text primitives created from Lua.
	class TextPrimitive : public GraphicalPrimitive {

	public:
		GraphicalPrimitiveOverrideMethods;

		std::string m_Text = ""; //!< String containing text to draw.
		bool m_IsSmall = false; //!< Use small or large font. True for small font.
		int m_Alignment = 0; //!< Alignment of text.
		float m_RotAngle = 0; //!< Angle to rotate text in radians.

		/// Constructor method for TextPrimitive object.
		/// @param player Player screen to draw this primitive on.
		/// @param pos Position of this primitive.
		/// @param text String containing text to draw.
		/// @param isSmall Use small or large font. True for small font.
		/// @param alignment Alignment of text.
		/// @param rotAngle Angle to rotate text in radians.
		TextPrimitive(int player, const Vector& pos, const std::string& text, bool isSmall, int alignment, float rotAngle) :
		    m_Text(text), m_IsSmall(isSmall), m_Alignment(alignment), m_RotAngle(rotAngle) {

			m_StartPos = pos;
			m_Player = player;
		}

	private:
		static const PrimitiveType c_PrimitiveType; //!< Type identifier of this primitive.
	};
#pragma endregion

#pragma region Bitmap Primitive
	/// Class used to schedule drawing of bitmap primitives created from Lua.
	class BitmapPrimitive : public GraphicalPrimitive {

	public:
		GraphicalPrimitiveOverrideMethods;

		BITMAP* m_Bitmap = nullptr; //!< Bitmap to draw.
		float m_RotAngle = 0; //!< Angle to rotate bitmap in radians.
		bool m_HFlipped = false; //!< Whether the Bitmap to draw should be horizontally flipped.
		bool m_VFlipped = false; //!< Whether the Bitmap to draw should be vertically flipped.

		/// Constructor method for BitmapPrimitive object.
		/// @param player Player screen to draw this primitive on.
		/// @param centerPos Position of this primitive's center.
		/// @param bitmap BITMAP to draw.
		/// @param rotAngle Angle to rotate BITMAP in radians.
		/// @param hFlipped Whether the BITMAP to draw should be horizontally flipped.
		/// @param vFlipped Whether the BITMAP to draw should be vertically flipped.
		BitmapPrimitive(int player, const Vector& centerPos, BITMAP* bitmap, float rotAngle, bool hFlipped, bool vFlipped) :
		    m_Bitmap(bitmap), m_RotAngle(rotAngle), m_HFlipped(hFlipped), m_VFlipped(vFlipped) {

			m_StartPos = centerPos;
			m_Player = player;
		}

		/// Constructor method for BitmapPrimitive object.
		/// @param player Player screen to draw this primitive on.
		/// @param centerPos Position of this primitive's center.
		/// @param moSprite The MOSprite to get the BITMAP to draw from.
		/// @param rotAngle Angle to rotate BITMAP in radians.
		/// @param frame Frame number of the MOSprite that will be drawn.
		/// @param hFlipped Whether the BITMAP to draw should be horizontally flipped.
		/// @param vFlipped Whether the BITMAP to draw should be vertically flipped.
		BitmapPrimitive(int player, const Vector& centerPos, const MOSprite* moSprite, float rotAngle, unsigned int frame, bool hFlipped, bool vFlipped) :
		    m_Bitmap(moSprite->GetSpriteFrame(frame)), m_RotAngle(rotAngle), m_HFlipped(hFlipped), m_VFlipped(vFlipped) {

			m_StartPos = centerPos;
			m_Player = player;
		}

		/// Constructor method for BitmapPrimitive object.
		/// @param player Player screen to draw this primitive on.
		/// @param centerPos Position of this primitive's center.
		/// @param filePath The path to get the BITMAP to draw from.
		/// @param rotAngle Angle to rotate BITMAP in radians.
		/// @param hFlipped Whether the BITMAP to draw should be horizontally flipped.
		/// @param vFlipped Whether the BITMAP to draw should be vertically flipped.
		BitmapPrimitive(int player, const Vector& centerPos, const std::string& filePath, float rotAngle, bool hFlipped, bool vFlipped) :
		    m_Bitmap(ContentFile(filePath.c_str()).GetAsBitmap()), m_RotAngle(rotAngle), m_HFlipped(hFlipped), m_VFlipped(vFlipped) {

			m_StartPos = centerPos;
			m_Player = player;
		}

	private:
		static const PrimitiveType c_PrimitiveType; //!< Type identifier of this primitive.
	};
#pragma endregion
} // namespace RTE
