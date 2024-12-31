#include "GraphicalPrimitive.h"
#include "Matrix.h"
#include "FrameMan.h"
#include "SceneMan.h"

#include "GUI.h"
#include "AllegroBitmap.h"

#include "Draw.h"

using namespace RTE;

const GraphicalPrimitive::PrimitiveType GraphicalPrimitive::c_PrimitiveType = PrimitiveType::None;
const GraphicalPrimitive::PrimitiveType LinePrimitive::c_PrimitiveType = PrimitiveType::Line;
const GraphicalPrimitive::PrimitiveType ArcPrimitive::c_PrimitiveType = PrimitiveType::Arc;
const GraphicalPrimitive::PrimitiveType SplinePrimitive::c_PrimitiveType = PrimitiveType::Spline;
const GraphicalPrimitive::PrimitiveType BoxPrimitive::c_PrimitiveType = PrimitiveType::Box;
const GraphicalPrimitive::PrimitiveType BoxFillPrimitive::c_PrimitiveType = PrimitiveType::BoxFill;
const GraphicalPrimitive::PrimitiveType RoundedBoxPrimitive::c_PrimitiveType = PrimitiveType::RoundedBox;
const GraphicalPrimitive::PrimitiveType RoundedBoxFillPrimitive::c_PrimitiveType = PrimitiveType::RoundedBoxFill;
const GraphicalPrimitive::PrimitiveType CirclePrimitive::c_PrimitiveType = PrimitiveType::Circle;
const GraphicalPrimitive::PrimitiveType CircleFillPrimitive::c_PrimitiveType = PrimitiveType::CircleFill;
const GraphicalPrimitive::PrimitiveType EllipsePrimitive::c_PrimitiveType = PrimitiveType::Ellipse;
const GraphicalPrimitive::PrimitiveType EllipseFillPrimitive::c_PrimitiveType = PrimitiveType::EllipseFill;
const GraphicalPrimitive::PrimitiveType TrianglePrimitive::c_PrimitiveType = PrimitiveType::Triangle;
const GraphicalPrimitive::PrimitiveType TriangleFillPrimitive::c_PrimitiveType = PrimitiveType::TriangleFill;
const GraphicalPrimitive::PrimitiveType PolygonPrimitive::c_PrimitiveType = PrimitiveType::Polygon;
const GraphicalPrimitive::PrimitiveType PolygonFillPrimitive::c_PrimitiveType = PrimitiveType::PolygonFill;
const GraphicalPrimitive::PrimitiveType TextPrimitive::c_PrimitiveType = PrimitiveType::Text;
const GraphicalPrimitive::PrimitiveType BitmapPrimitive::c_PrimitiveType = PrimitiveType::Bitmap;

Vector GraphicalPrimitive::WrapCoordinates(Vector targetPos, const Vector& scenePos) const {
	Vector drawPos = scenePos;

	if (g_SceneMan.SceneWrapsX()) {
		float sceneWidth = static_cast<float>(g_SceneMan.GetSceneWidth());
		if (targetPos.m_X <= sceneWidth && targetPos.m_X > sceneWidth / 2) {
			targetPos.m_X -= sceneWidth;
		}
	}
	drawPos.m_X -= targetPos.m_X;

	if (g_SceneMan.SceneWrapsY()) {
		float sceneHeight = static_cast<float>(g_SceneMan.GetSceneHeight());
		if (targetPos.m_Y <= sceneHeight && targetPos.m_Y > sceneHeight / 2) {
			targetPos.m_Y -= sceneHeight;
		}
	}
	drawPos.m_Y -= targetPos.m_Y;
	return drawPos;
}

void LinePrimitive::Draw(BITMAP* drawScreen, const Vector& targetPos) {
	DrawLineV(targetPos + Vector(-30, -30), targetPos + Vector(30,30), {53, 0, 0, 255});
	DrawLineV(targetPos + Vector(30, -30), targetPos + Vector(-30,30), {53, 0, 0, 255});
	Vector drawStart = WrapCoordinates(targetPos, m_StartPos);
	Vector drawEnd = WrapCoordinates(targetPos, m_EndPos);
	DrawLineEx(drawStart, drawEnd, m_Thickness, {m_Color, 0, 0, 255});
}

void ArcPrimitive::Draw(BITMAP* drawScreen, const Vector& targetPos) {
	Vector drawStart = WrapCoordinates(targetPos, m_StartPos);
	if (m_Thickness > 1) {
		DrawRing(drawStart, m_Radius - (m_Thickness / 2.0f), m_Radius + (m_Thickness / 2.0f), m_StartAngle, m_EndAngle, std::abs(m_EndAngle - m_StartAngle), {m_Color, 0, 0, 255});
	} else {
		DrawRing(drawStart, m_Radius - 0.5f, m_Radius + 0.5f, m_StartAngle, m_EndAngle, std::abs(m_EndAngle - m_StartAngle), {m_Color, 0, 0, 255});
	}
}

void SplinePrimitive::Draw(BITMAP* drawScreen, const Vector& targetPos) {
	Vector drawStart = WrapCoordinates(targetPos, m_StartPos);
	Vector drawGuideA = WrapCoordinates(targetPos, m_GuidePointAPos);
	Vector drawGuideB = WrapCoordinates(targetPos, m_GuidePointBPos);
	Vector drawEnd = WrapCoordinates(targetPos, m_EndPos);

	std::array<Vector2, 4> guidePoints = {drawStart, drawGuideA, drawGuideB, drawEnd};
	DrawSplineBasis(guidePoints.data(), guidePoints.size(), 1, {m_Color, 0, 0, 255});
}

void BoxPrimitive::Draw(BITMAP* drawScreen, const Vector& targetPos) {
	Vector drawStart = WrapCoordinates(targetPos, m_StartPos);
	Vector drawEnd = WrapCoordinates(targetPos, m_EndPos);
	Vector dimensions = drawEnd - drawStart;
	DrawRectangleLines(drawStart.m_X, drawStart.m_Y, dimensions.m_X, dimensions.m_Y, {m_Color, 0, 0, 255});
}

void BoxFillPrimitive::Draw(BITMAP* drawScreen, const Vector& targetPos) {
	Vector drawStart = WrapCoordinates(targetPos, m_StartPos);
	Vector drawEnd = WrapCoordinates(targetPos, m_EndPos);
	Vector dimensions = drawEnd - drawStart;
	DrawRectangle(drawStart.m_X, drawStart.m_Y, dimensions.m_X, dimensions.m_Y, {m_Color, 0, 0, 255});
}

void RoundedBoxPrimitive::Draw(BITMAP* drawScreen, const Vector& targetPos) {
	if (m_StartPos.m_X > m_EndPos.m_X) {
		std::swap(m_StartPos.m_X, m_EndPos.m_X);
	}
	if (m_StartPos.m_Y > m_EndPos.m_Y) {
		std::swap(m_StartPos.m_Y, m_EndPos.m_Y);
	}

	Vector drawStart = WrapCoordinates(targetPos, m_StartPos);
	Vector drawEnd = WrapCoordinates(targetPos, m_EndPos);
	Vector ringCornerTopLeft(drawStart.GetFloorIntX() + m_CornerRadius, drawStart.GetFloorIntY() + m_CornerRadius);
	Vector ringCornerBottomLeft(drawStart.GetFloorIntX() + m_CornerRadius, drawEnd.GetFloorIntY() - m_CornerRadius + 1.0f);
	Vector ringCornerTopRight(drawEnd.GetFloorIntX() - m_CornerRadius + 1.0f, drawStart.GetFloorIntY() + m_CornerRadius);
	Vector ringCornerBottomRight(drawEnd.GetFloorIntX() - m_CornerRadius + 1.0f, drawEnd.GetFloorIntY() - m_CornerRadius + 1.0f);

	DrawRing(ringCornerTopLeft, m_CornerRadius - 1.0f, m_CornerRadius, -90, -180, 90, {m_Color, 0, 0, 255});
	DrawRing(ringCornerBottomLeft, m_CornerRadius - 1.0f, m_CornerRadius, 90, 180, 90, {m_Color, 0, 0, 255});
	DrawRing(ringCornerTopRight, m_CornerRadius - 1.0f, m_CornerRadius, 0, -90, 90, {m_Color, 0, 0, 255});
	DrawRing(ringCornerBottomRight, m_CornerRadius - 1.0f, m_CornerRadius, 90, 0, 90, {m_Color, 0, 0, 255});
	DrawRectangle(drawStart.GetFloorIntX() + m_CornerRadius, drawStart.GetFloorIntY(), drawEnd.GetFloorIntX() - drawStart.GetFloorIntX() - 2 * m_CornerRadius + 1, 1, {m_Color, 0, 0, 255});
	DrawRectangle(drawStart.GetFloorIntX() + m_CornerRadius, drawEnd.GetFloorIntY(), drawEnd.GetFloorIntX() - drawStart.GetFloorIntX() - 2 * m_CornerRadius + 1, 1, {m_Color, 0, 0, 255});
	DrawRectangle(drawStart.GetFloorIntX(), drawStart.GetFloorIntY() + m_CornerRadius, 1, drawEnd.GetFloorIntY() - drawStart.GetFloorIntY() - 2 * m_CornerRadius + 1, {m_Color, 0, 0, 255});
	DrawRectangle(drawEnd.GetFloorIntX(), drawStart.GetFloorIntY() + m_CornerRadius, 1, drawEnd.GetFloorIntY() - drawStart.GetFloorIntY() - 2 * m_CornerRadius + 1, {m_Color, 0, 0, 255});
}

void RoundedBoxFillPrimitive::Draw(BITMAP* drawScreen, const Vector& targetPos) {
	if (m_StartPos.m_X > m_EndPos.m_X) {
		std::swap(m_StartPos.m_X, m_EndPos.m_X);
	}
	if (m_StartPos.m_Y > m_EndPos.m_Y) {
		std::swap(m_StartPos.m_Y, m_EndPos.m_Y);
	}

	Vector drawStart = WrapCoordinates(targetPos, m_StartPos);
	Vector drawEnd = WrapCoordinates(targetPos, m_EndPos);
	DrawCircle(drawStart.GetFloorIntX() + m_CornerRadius, drawStart.GetFloorIntY() + m_CornerRadius, m_CornerRadius, {m_Color, 0, 0, 255});
	DrawCircle(drawStart.GetFloorIntX() + m_CornerRadius, drawEnd.GetFloorIntY() - m_CornerRadius, m_CornerRadius, {m_Color, 0, 0, 255});
	DrawCircle(drawEnd.GetFloorIntX() - m_CornerRadius, drawStart.GetFloorIntY() + m_CornerRadius, m_CornerRadius, {m_Color, 0, 0, 255});
	DrawCircle(drawEnd.GetFloorIntX() - m_CornerRadius, drawEnd.GetFloorIntY() - m_CornerRadius, m_CornerRadius, {m_Color, 0, 0, 255});
	DrawRectangle(drawStart.GetFloorIntX(), drawStart.GetFloorIntY() + m_CornerRadius, drawEnd.GetFloorIntX() - drawStart.GetFloorIntX(), drawEnd.GetFloorIntY() - drawStart.GetFloorIntY() - 2 * m_CornerRadius, {m_Color, 0, 0, 255});
	DrawRectangle(drawStart.GetFloorIntX() + m_CornerRadius, drawStart.GetFloorIntY(), drawEnd.GetFloorIntX() - drawStart.GetFloorIntX() - 2 * m_CornerRadius, drawEnd.GetFloorIntY() - drawStart.GetFloorIntY(), {m_Color, 0, 0, 255});
}

void CirclePrimitive::Draw(BITMAP* drawScreen, const Vector& targetPos) {
	Vector drawStart = WrapCoordinates(targetPos, m_StartPos);
	DrawCircleLines(drawStart.GetFloorIntX(), drawStart.GetFloorIntY(), m_Radius, {m_Color, 0, 0, 255});
}

void CircleFillPrimitive::Draw(BITMAP* drawScreen, const Vector& targetPos) {
	Vector drawStart = WrapCoordinates(targetPos, m_StartPos);
	DrawCircle(drawStart.GetFloorIntX(), drawStart.GetFloorIntY(), m_Radius, {m_Color, 0, 0, 255});
}

void EllipsePrimitive::Draw(BITMAP* drawScreen, const Vector& targetPos) {
	Vector drawStart = WrapCoordinates(targetPos, m_StartPos);
	DrawEllipseLines(drawStart.GetFloorIntX(), drawStart.GetFloorIntY(), m_HorizRadius, m_VertRadius, {m_Color, 0, 0, 255});
}

void EllipseFillPrimitive::Draw(BITMAP* drawScreen, const Vector& targetPos) {
	Vector drawStart = WrapCoordinates(targetPos, m_StartPos);
	DrawEllipse(drawStart.GetFloorIntX(), drawStart.GetFloorIntY(), m_HorizRadius, m_VertRadius, {m_Color, 0, 0, 255});
}

void TrianglePrimitive::Draw(BITMAP* drawScreen, const Vector& targetPos) {
	Vector drawPointA = WrapCoordinates(targetPos, m_PointAPos);
	Vector drawPointB = WrapCoordinates(targetPos, m_PointBPos);
	Vector drawPointC = WrapCoordinates(targetPos, m_PointCPos);
	DrawLine(drawPointA.GetFloorIntX(), drawPointA.GetFloorIntY(), drawPointB.GetFloorIntX(), drawPointB.GetFloorIntY(), {m_Color, 0, 0, 255});
	DrawLine(drawPointB.GetFloorIntX(), drawPointB.GetFloorIntY(), drawPointC.GetFloorIntX(), drawPointC.GetFloorIntY(), {m_Color, 0, 0, 255});
	DrawLine(drawPointC.GetFloorIntX(), drawPointC.GetFloorIntY(), drawPointA.GetFloorIntX(), drawPointA.GetFloorIntY(), {m_Color, 0, 0, 255});
}

void TriangleFillPrimitive::Draw(BITMAP* drawScreen, const Vector& targetPos) {
	Vector drawPointA = WrapCoordinates(targetPos, m_PointAPos);
	Vector drawPointB = WrapCoordinates(targetPos, m_PointBPos);
	Vector drawPointC = WrapCoordinates(targetPos, m_PointCPos);
	DrawTriangle(drawPointA, drawPointB, drawPointC, {m_Color, 0, 0, 255});
}

void PolygonPrimitive::Draw(BITMAP* drawScreen, const Vector& targetPos) {
	Vector drawStart;
	Vector drawEnd;
	Vector startPos = WrapCoordinates(targetPos, m_StartPos);
	for (int i = 0; i < m_Vertices.size(); ++i) {
		drawStart = startPos - targetPos + (*m_Vertices[i]);
		drawEnd = startPos - targetPos + ((i + 1 < m_Vertices.size()) ? *m_Vertices[i + 1] : *m_Vertices[0]);
		DrawLine(drawStart.GetFloorIntX(), drawStart.GetFloorIntY(), drawEnd.GetFloorIntX(), drawEnd.GetFloorIntY(), {m_Color, 0, 0, 255});
	}
}

void PolygonFillPrimitive::Draw(BITMAP* drawScreen, const Vector& targetPos) {
	size_t drawPointsSize = m_Vertices.size();
	Vector drawStart = WrapCoordinates(targetPos, m_StartPos);

	std::vector<Vector2> drawPoints = {};
	drawPoints.reserve(drawPointsSize);

	for (const Vector* vertex: m_Vertices) {
		drawPoints.emplace_back(drawStart.GetFloorIntX() + vertex->GetFloorIntX(), drawStart.GetFloorIntY() + vertex->GetFloorIntY());
	}
	DrawTriangleStrip(drawPoints.data(), drawPoints.size(), {m_Color, 0, 0, 255});
}

void TextPrimitive::Draw(BITMAP* drawScreen, const Vector& targetPos) {
	if (m_Text.empty()) {
		return;
	}

	AllegroBitmap playerGUIBitmap(drawScreen);
	GUIFont* font = m_IsSmall ? g_FrameMan.GetSmallFont() : g_FrameMan.GetLargeFont();
	Matrix rotation = Matrix(m_RotAngle);
	Vector targetPosAdjustment = Vector();

	BITMAP* tempDrawBitmap = nullptr;
	if (m_BlendMode > DrawBlendMode::NoBlend || m_RotAngle != 0) {
		int textWidth = font->CalculateWidth(m_Text);
		int textHeight = font->CalculateHeight(m_Text);

		tempDrawBitmap = create_bitmap_ex(8, textWidth * 2, textHeight);
		clear_to_color(tempDrawBitmap, ColorKeys::g_MaskColor);
		AllegroBitmap tempDrawAllegroBitmap(tempDrawBitmap);
		font->DrawAligned(&tempDrawAllegroBitmap, textWidth, 0, m_Text, m_Alignment);

		targetPosAdjustment = Vector(static_cast<float>(textWidth), 0);
	}

	Vector drawStart = WrapCoordinates(targetPos, m_StartPos) - targetPosAdjustment;

	if (m_BlendMode > DrawBlendMode::NoBlend) {
		if (m_RotAngle != 0) {
			rotate_sprite_trans(drawScreen, tempDrawBitmap, drawStart.GetFloorIntX(), drawStart.GetFloorIntY(), ftofix(rotation.GetAllegroAngle()));
		} else {
			draw_trans_sprite(drawScreen, tempDrawBitmap, drawStart.GetFloorIntX(), drawStart.GetFloorIntY());
		}
	} else {
		if (m_RotAngle != 0) {
			rotate_sprite(drawScreen, tempDrawBitmap, drawStart.GetFloorIntX(), drawStart.GetFloorIntY(), ftofix(rotation.GetAllegroAngle()));
		} else {
			font->DrawAligned(&playerGUIBitmap, drawStart.GetFloorIntX(), drawStart.GetFloorIntY(), m_Text, m_Alignment);
		}
	}
	if (tempDrawBitmap) {
		destroy_bitmap(tempDrawBitmap);
	}
}

void BitmapPrimitive::Draw(BITMAP* drawScreen, const Vector& targetPos) {
	if (!m_Bitmap) {
		return;
	}

	BITMAP* bitmapToDraw = create_bitmap_ex(8, m_Bitmap->w, m_Bitmap->h);
	clear_to_color(bitmapToDraw, ColorKeys::g_MaskColor);
	draw_sprite(bitmapToDraw, m_Bitmap, 0, 0);

	if (m_HFlipped || m_VFlipped) {
		BITMAP* flipBitmap = create_bitmap_ex(8, bitmapToDraw->w, bitmapToDraw->h);
		clear_to_color(flipBitmap, ColorKeys::g_MaskColor);

		if (m_HFlipped && !m_VFlipped) {
			draw_sprite_h_flip(flipBitmap, bitmapToDraw, 0, 0);
		} else if (!m_HFlipped && m_VFlipped) {
			draw_sprite_v_flip(flipBitmap, bitmapToDraw, 0, 0);
		} else if (m_HFlipped && m_VFlipped) {
			draw_sprite_vh_flip(flipBitmap, bitmapToDraw, 0, 0);
		}

		blit(flipBitmap, bitmapToDraw, 0, 0, 0, 0, bitmapToDraw->w, bitmapToDraw->h);
		destroy_bitmap(flipBitmap);
	}

	Matrix rotation = Matrix(m_RotAngle);

	Vector drawStart = WrapCoordinates(targetPos, m_StartPos);
	if (m_BlendMode > DrawBlendMode::NoBlend) {
		pivot_scaled_sprite_trans(drawScreen, bitmapToDraw, drawStart.GetFloorIntX(), drawStart.GetFloorIntY(), bitmapToDraw->w / 2, bitmapToDraw->h / 2, ftofix(rotation.GetAllegroAngle()), ftofix(1.0));
	} else {
		pivot_scaled_sprite(drawScreen, bitmapToDraw, drawStart.GetFloorIntX(), drawStart.GetFloorIntY(), bitmapToDraw->w / 2, bitmapToDraw->h / 2, ftofix(rotation.GetAllegroAngle()), ftofix(1.0));
	}
	destroy_bitmap(bitmapToDraw);
}
