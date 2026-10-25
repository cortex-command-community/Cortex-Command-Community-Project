#include "GraphicalPrimitive.h"
#include "Matrix.h"
#include "FrameMan.h"
#include "SceneMan.h"
#include "GLResourceMan.h"
#include "SLTerrain.h"

#include "GUI.h"
#include "AllegroBitmap.h"

#include "Draw.h"

#include <array>
#include <cmath>

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
	return targetPos + scenePos;
}

void GraphicalPrimitive::DrawTiled(BITMAP* drawScreen, const Vector& targetPos) {
	Vector tiledTarget{targetPos};
	if (g_SceneMan.SceneWrapsX()) {
		tiledTarget.m_X = std::fmod(targetPos.m_X, g_SceneMan.GetSceneWidth());
	}
	if (g_SceneMan.SceneWrapsY()) {
		tiledTarget.m_Y = std::fmod(targetPos.m_Y, g_SceneMan.GetSceneHeight());
	}

	float bitmapWidth = g_SceneMan.GetSceneWidth();
	float bitmapHeight = g_SceneMan.GetSceneHeight();
	float areaToCoverX = drawScreen->w + g_SceneMan.GetTerrain()->GetOffset().m_X;
	float areaToCoverY = drawScreen->h + g_SceneMan.GetTerrain()->GetOffset().m_Y;

	for (int tiledOffsetX = 0; tiledOffsetX < areaToCoverX;) {
		float destX = tiledOffsetX - tiledTarget.m_X;

		for (int tiledOffsetY = 0; tiledOffsetY < areaToCoverY;) {
			float destY = tiledOffsetY - tiledTarget.m_Y;
			Draw(drawScreen, Vector(destX, destY));
			if (!g_SceneMan.SceneWrapsY()) {
				break;
			}
			tiledOffsetY += bitmapHeight;
		}
		if (!g_SceneMan.SceneWrapsX()) {
			break;
		}
		tiledOffsetX += bitmapWidth;
	}
	// Draw(drawScreen, targetPos);
}

void LinePrimitive::Draw(BITMAP* drawScreen, const Vector& targetPos) {
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


void TextPrimitive::CreateTextBitmap() {
	if(m_Text.empty()) {
		return;
	}
	GUIFont* font = m_IsSmall ? g_FrameMan.GetSmallFont() : g_FrameMan.GetLargeFont();
	Matrix rotation = Matrix(m_RotAngle);
	Vector targetPosAdjustment = Vector();

	int textWidth = font->CalculateWidth(m_Text);
	int textHeight = font->CalculateHeight(m_Text);

	drawing_mode(DRAW_MODE_SOLID, nullptr, 0, 0);
	
	m_TextBitmap = create_bitmap_ex(8, textWidth * 2, textHeight);
	clear_to_color(m_TextBitmap, ColorKeys::g_MaskColor);
	AllegroBitmap tempDrawAllegroBitmap(m_TextBitmap);
	font->DrawAligned(&tempDrawAllegroBitmap, textWidth, 0, m_Text, m_Alignment);

	m_TargetPosAlignment = Vector(static_cast<float>(textWidth), 0);
}

void TextPrimitive::Draw(BITMAP* drawScreen, const Vector& targetPos) {
	if (!m_TextBitmap) {
		return;
	}

	Vector drawStart = WrapCoordinates(targetPos, m_StartPos) - m_TargetPosAlignment;
	Rectangle bitmapRect(0.0f, 0.0f, m_TextBitmap->w, m_TextBitmap->h);
	Rectangle destRect(
		drawStart.m_X,
		drawStart.m_Y,
		m_TextBitmap->w,
		m_TextBitmap->h);
	DrawTexturePro(m_TextBitmap, bitmapRect, destRect, {0.0f, 0.0f}, m_RotAngle, {255, 255, 255, 255});
}

TextPrimitive::~TextPrimitive() {
	if (m_TextBitmap) {
		g_GLResourceMan.DestroyBitmapInfo(m_TextBitmap);
		destroy_bitmap(m_TextBitmap);
	}
}

void BitmapPrimitive::Draw(BITMAP* drawScreen, const Vector& targetPos) {
	if (!m_Bitmap) {
		return;
	}

	Vector drawStart = WrapCoordinates(targetPos, m_StartPos);

	Rectangle flippedRect(
		drawStart.m_X - m_Bitmap->w / 2,
		drawStart.m_Y - m_Bitmap->h / 2,
		(m_VFlipped ? -m_Bitmap->w : m_Bitmap->w) * m_Scale,
		(m_HFlipped ? -m_Bitmap->h : m_Bitmap->h) * m_Scale
	);

	DrawTexturePro(m_Bitmap, Rectangle(0.0f, 0.0f, m_Bitmap->w, m_Bitmap->h), flippedRect, {0.0f, 0.0f}, m_RotAngle, {255, 255, 255, 255});
}
