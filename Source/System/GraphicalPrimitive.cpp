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

void GraphicalPrimitive::TranslateCoordinates(Vector targetPos, const Vector& scenePos, Vector& drawLeftPos, Vector& drawRightPos) const {
	drawLeftPos = scenePos;
	drawRightPos = scenePos;

	if (g_SceneMan.SceneWrapsX()) {
		float sceneWidth = static_cast<float>(g_SceneMan.GetSceneWidth());
		if (targetPos.m_X <= sceneWidth && targetPos.m_X > sceneWidth / 2) {
			targetPos.m_X -= sceneWidth;
		}
		drawLeftPos.m_X = (drawLeftPos.m_X > 0) ? (drawLeftPos.m_X -= sceneWidth) : (drawLeftPos.m_X -= sceneWidth + targetPos.m_X);
	}
	drawLeftPos.m_X -= targetPos.m_X;
	drawRightPos.m_X -= targetPos.m_X;

	if (g_SceneMan.SceneWrapsY()) {
		float sceneHeight = static_cast<float>(g_SceneMan.GetSceneHeight());
		if (targetPos.m_Y <= sceneHeight && targetPos.m_Y > sceneHeight / 2) {
			targetPos.m_Y -= sceneHeight;
		}
		drawLeftPos.m_Y = (drawLeftPos.m_Y > 0) ? (drawLeftPos.m_Y -= sceneHeight) : (drawLeftPos.m_Y -= sceneHeight + targetPos.m_Y);
	}
	drawLeftPos.m_Y -= targetPos.m_Y;
	drawRightPos.m_Y -= targetPos.m_Y;
}

void LinePrimitive::Draw(BITMAP* drawScreen, const Vector& targetPos) {
	if (!g_SceneMan.SceneWrapsX() && !g_SceneMan.SceneWrapsY()) {
		Vector drawStart = m_StartPos - targetPos;
		Vector drawEnd = m_EndPos - targetPos;
		DrawLine(drawStart.GetFloorIntX(), drawStart.GetFloorIntY(), drawEnd.GetFloorIntX(), drawEnd.GetFloorIntY(), {m_Color, 0, 0, g_FrameMan.GetCurrentAlpha()});
	} else {
		Vector drawStartLeft;
		Vector drawEndLeft;
		Vector drawStartRight;
		Vector drawEndRight;

		TranslateCoordinates(targetPos, m_StartPos, drawStartLeft, drawStartRight);
		TranslateCoordinates(targetPos, m_EndPos, drawEndLeft, drawEndRight);

		DrawLine(drawStartLeft.GetFloorIntX(), drawStartLeft.GetFloorIntY(), drawEndLeft.GetFloorIntX(), drawEndLeft.GetFloorIntY(), {m_Color, 0, 0, g_FrameMan.GetCurrentAlpha()});
		DrawLine(drawStartRight.GetFloorIntX(), drawStartRight.GetFloorIntY(), drawEndRight.GetFloorIntX(), drawEndRight.GetFloorIntY(), {m_Color, 0, 0, g_FrameMan.GetCurrentAlpha()});
	}
}

void ArcPrimitive::Draw(BITMAP* drawScreen, const Vector& targetPos) {
	if (!g_SceneMan.SceneWrapsX() && !g_SceneMan.SceneWrapsY()) {
		Vector drawStart = m_StartPos - targetPos;
		if (m_Thickness > 1) {
			DrawRing(drawStart, m_Radius - (m_Thickness / 2), m_Radius + (m_Thickness / 2), m_StartAngle, m_EndAngle, 0, {m_Color, 0, 0, g_FrameMan.GetCurrentAlpha()});
		} else {
			DrawCircleSector(drawStart, m_Radius, m_StartAngle, m_EndAngle, 0, {m_Color, 0, 0, g_FrameMan.GetCurrentAlpha()});
		}
	} else {
		Vector drawStartLeft;
		Vector drawStartRight;

		TranslateCoordinates(targetPos, m_StartPos, drawStartLeft, drawStartRight);

		if (m_Thickness > 1) {
			DrawRing(drawStartLeft, m_Radius - (m_Thickness / 2), m_Radius + (m_Thickness / 2), m_StartAngle, m_EndAngle, 0, {m_Color, 0, 0, g_FrameMan.GetCurrentAlpha()});
			DrawRing(drawStartRight, m_Radius - (m_Thickness / 2), m_Radius + (m_Thickness / 2), m_StartAngle, m_EndAngle, 0, {m_Color, 0, 0, g_FrameMan.GetCurrentAlpha()});
		} else {
			DrawCircleSectorLines(drawStartLeft, m_Radius, m_StartAngle, m_EndAngle, 0, {m_Color, 0, 0, g_FrameMan.GetCurrentAlpha()});
			DrawCircleSectorLines(drawStartRight, m_Radius, m_StartAngle, m_EndAngle, 0, {m_Color, 0, 0, g_FrameMan.GetCurrentAlpha()});
		}
	}
}

void SplinePrimitive::Draw(BITMAP* drawScreen, const Vector& targetPos) {
	if (!g_SceneMan.SceneWrapsX() && !g_SceneMan.SceneWrapsY()) {
		Vector drawStart = m_StartPos - targetPos;
		Vector drawGuideA = m_GuidePointAPos - targetPos;
		Vector drawGuideB = m_GuidePointBPos - targetPos;
		Vector drawEnd = m_EndPos - targetPos;

		std::array<Vector2, 8> guidePoints = {drawStart, drawGuideA, drawGuideB, drawEnd};
		DrawSplineBasis(guidePoints.data(), guidePoints.size(), 1, {m_Color, 0, 0, g_FrameMan.GetCurrentAlpha()});
	} else {
		Vector drawStartLeft;
		Vector drawGuideALeft;
		Vector drawGuideBLeft;
		Vector drawEndLeft;
		Vector drawStartRight;
		Vector drawGuideARight;
		Vector drawGuideBRight;
		Vector drawEndRight;

		TranslateCoordinates(targetPos, m_StartPos, drawStartLeft, drawStartRight);
		TranslateCoordinates(targetPos, m_GuidePointAPos, drawGuideALeft, drawGuideARight);
		TranslateCoordinates(targetPos, m_GuidePointBPos, drawGuideBLeft, drawGuideBRight);
		TranslateCoordinates(targetPos, m_EndPos, drawEndLeft, drawEndRight);

		std::array<Vector2, 8> guidePointsLeft = {drawStartLeft, drawGuideALeft, drawGuideBLeft, drawEndLeft};
		std::array<Vector2, 8> guidePointsRight = {drawStartRight, drawGuideARight, drawGuideBRight, drawEndRight};
		DrawSplineBasis(guidePointsLeft.data(), guidePointsLeft.size(), 1, {m_Color, 0, 0, g_FrameMan.GetCurrentAlpha()});
		DrawSplineBasis(guidePointsRight.data(), guidePointsRight.size(), 1, {m_Color, 0, 0, g_FrameMan.GetCurrentAlpha()});
	}
}

void BoxPrimitive::Draw(BITMAP* drawScreen, const Vector& targetPos) {
	if (!g_SceneMan.SceneWrapsX() && !g_SceneMan.SceneWrapsY()) {
		Vector drawStart = m_StartPos - targetPos;
		Vector drawEnd = m_EndPos - targetPos;
		Vector dimensions = drawEnd - drawStart;
		DrawRectangleLines(drawStart.m_X, drawStart.m_Y, dimensions.m_X, dimensions.m_Y, {m_Color, 0, 0, g_FrameMan.GetCurrentAlpha()});
	} else {
		Vector drawStartLeft;
		Vector drawEndLeft;
		Vector dimensionsLeft;
		Vector drawStartRight;
		Vector drawEndRight;
		Vector dimensionsRight;

		TranslateCoordinates(targetPos, m_StartPos, drawStartLeft, drawStartRight);
		TranslateCoordinates(targetPos, m_EndPos, drawEndLeft, drawEndRight);

		dimensionsLeft = drawEndLeft - drawStartLeft;
		dimensionsRight = drawEndRight - drawStartRight;

		DrawRectangleLines(drawStartLeft.m_X, drawStartLeft.m_Y, dimensionsLeft.m_X, dimensionsLeft.m_Y, {m_Color, 0, 0, g_FrameMan.GetCurrentAlpha()});
		DrawRectangleLines(drawStartRight.m_X, drawStartRight.m_Y, dimensionsRight.m_X, dimensionsRight.m_Y, {m_Color, 0, 0, g_FrameMan.GetCurrentAlpha()});
	}
}

void BoxFillPrimitive::Draw(BITMAP* drawScreen, const Vector& targetPos) {
	if (!g_SceneMan.SceneWrapsX() && !g_SceneMan.SceneWrapsY()) {
		Vector drawStart = m_StartPos - targetPos;
		Vector drawEnd = m_EndPos - targetPos;
		Vector dimensions = drawEnd - drawStart;
		DrawRectangle(drawStart.m_X, drawStart.m_Y, dimensions.m_X, dimensions.m_Y, {m_Color, 0, 0, g_FrameMan.GetCurrentAlpha()});
	} else {
		Vector drawStartLeft;
		Vector drawEndLeft;
		Vector dimensionsLeft;
		Vector drawStartRight;
		Vector drawEndRight;
		Vector dimensionsRight;

		TranslateCoordinates(targetPos, m_StartPos, drawStartLeft, drawStartRight);
		TranslateCoordinates(targetPos, m_EndPos, drawEndLeft, drawEndRight);

		dimensionsLeft = drawEndLeft - drawStartLeft;
		dimensionsRight = drawEndRight - drawStartRight;

		DrawRectangle(drawStartLeft.m_X, drawStartLeft.m_Y, dimensionsLeft.m_X, dimensionsLeft.m_Y, {m_Color, 0, 0, g_FrameMan.GetCurrentAlpha()});
		DrawRectangle(drawStartRight.m_X, drawStartRight.m_Y, dimensionsRight.m_X, dimensionsRight.m_Y, {m_Color, 0, 0, g_FrameMan.GetCurrentAlpha()});
	}
}

void RoundedBoxPrimitive::Draw(BITMAP* drawScreen, const Vector& targetPos) {
	if (m_StartPos.m_X > m_EndPos.m_X) {
		std::swap(m_StartPos.m_X, m_EndPos.m_X);
	}
	if (m_StartPos.m_Y > m_EndPos.m_Y) {
		std::swap(m_StartPos.m_Y, m_EndPos.m_Y);
	}

	if (!g_SceneMan.SceneWrapsX() && !g_SceneMan.SceneWrapsY()) {
		Vector drawStart = m_StartPos - targetPos;
		Vector drawEnd = m_EndPos - targetPos;

		DrawCircleSectorLines(drawStart + Vector{static_cast<float>(m_CornerRadius), static_cast<float>(m_CornerRadius)}, m_CornerRadius, 90, 180, 0, {m_Color, 0, 0, g_FrameMan.GetCurrentAlpha()});
		DrawCircleSectorLines(drawStart + Vector{static_cast<float>(m_CornerRadius), -static_cast<float>(m_CornerRadius)}, m_CornerRadius, 180, -90, 0, {m_Color, 0, 0, g_FrameMan.GetCurrentAlpha()});
		DrawCircleSectorLines(drawEnd - Vector{static_cast<float>(m_CornerRadius), -static_cast<float>(m_CornerRadius)}, m_CornerRadius, 0, 90, 0, {m_Color, 0, 0, g_FrameMan.GetCurrentAlpha()});
		DrawCircleSectorLines(drawEnd - Vector{static_cast<float>(m_CornerRadius), static_cast<float>(m_CornerRadius)}, m_CornerRadius, -90, 0, 0, {m_Color, 0, 0, g_FrameMan.GetCurrentAlpha()});

		DrawLine(drawStart.GetFloorIntX() + m_CornerRadius, drawStart.GetFloorIntY(), drawEnd.GetFloorIntX() - m_CornerRadius, drawStart.GetFloorIntY(), {m_Color, 0, 0, g_FrameMan.GetCurrentAlpha()});
		DrawLine(drawStart.GetFloorIntX() + m_CornerRadius, drawEnd.GetFloorIntY(), drawEnd.GetFloorIntX() - m_CornerRadius, drawEnd.GetFloorIntY(), {m_Color, 0, 0, g_FrameMan.GetCurrentAlpha()});
		DrawLine(drawStart.GetFloorIntX(), drawStart.GetFloorIntY() + m_CornerRadius, drawStart.GetFloorIntX(), drawEnd.GetFloorIntY() - m_CornerRadius, {m_Color, 0, 0, g_FrameMan.GetCurrentAlpha()});
		DrawLine(drawEnd.GetFloorIntX(), drawStart.GetFloorIntY() + m_CornerRadius, drawEnd.GetFloorIntY(), drawEnd.GetFloorIntY() - m_CornerRadius, {m_Color, 0, 0, g_FrameMan.GetCurrentAlpha()});
	} else {
		Vector drawStartLeft;
		Vector drawEndLeft;
		Vector drawStartRight;
		Vector drawEndRight;

		TranslateCoordinates(targetPos, m_StartPos, drawStartLeft, drawStartRight);
		TranslateCoordinates(targetPos, m_EndPos, drawEndLeft, drawEndRight);

		DrawCircleSectorLines(drawStartLeft + Vector{static_cast<float>(m_CornerRadius), static_cast<float>(m_CornerRadius)}, m_CornerRadius, 90, 180, 0, {m_Color, 0, 0, g_FrameMan.GetCurrentAlpha()});
		DrawCircleSectorLines(drawStartLeft + Vector{static_cast<float>(m_CornerRadius), -static_cast<float>(m_CornerRadius)}, m_CornerRadius, 180, -90, 0, {m_Color, 0, 0, g_FrameMan.GetCurrentAlpha()});
		DrawCircleSectorLines(drawEndLeft - Vector{static_cast<float>(m_CornerRadius), -static_cast<float>(m_CornerRadius)}, m_CornerRadius, 0, 90, 0, {m_Color, 0, 0, g_FrameMan.GetCurrentAlpha()});
		DrawCircleSectorLines(drawEndLeft - Vector{static_cast<float>(m_CornerRadius), static_cast<float>(m_CornerRadius)}, m_CornerRadius, -90, 0, 0, {m_Color, 0, 0, g_FrameMan.GetCurrentAlpha()});
		DrawLine(drawStartLeft.GetFloorIntX() + m_CornerRadius, drawStartLeft.GetFloorIntY(), drawEndLeft.GetFloorIntX() - m_CornerRadius, drawStartLeft.GetFloorIntY(),{m_Color, 0, 0, g_FrameMan.GetCurrentAlpha()});
		DrawLine(drawStartLeft.GetFloorIntX() + m_CornerRadius, drawEndLeft.GetFloorIntY(), drawEndLeft.GetFloorIntX() - m_CornerRadius, drawEndLeft.GetFloorIntY(), {m_Color, 0, 0, g_FrameMan.GetCurrentAlpha()});
		DrawLine(drawStartLeft.GetFloorIntX(), drawStartLeft.GetFloorIntY() + m_CornerRadius, drawStartLeft.GetFloorIntX(), drawEndLeft.GetFloorIntY() - m_CornerRadius, {m_Color, 0, 0, g_FrameMan.GetCurrentAlpha()});
		DrawLine(drawEndLeft.GetFloorIntX(), drawStartLeft.GetFloorIntY() + m_CornerRadius, drawEndLeft.GetFloorIntX(), drawEndLeft.GetFloorIntY() - m_CornerRadius, {m_Color, 0, 0, g_FrameMan.GetCurrentAlpha()});

		DrawCircleSectorLines(drawStartRight + Vector{static_cast<float>(m_CornerRadius), static_cast<float>(m_CornerRadius)}, m_CornerRadius, 90, 180, 0, {m_Color, 0, 0, g_FrameMan.GetCurrentAlpha()});
		DrawCircleSectorLines(drawStartRight + Vector{static_cast<float>(m_CornerRadius), -static_cast<float>(m_CornerRadius)}, m_CornerRadius, 180, -90, 0, {m_Color, 0, 0, g_FrameMan.GetCurrentAlpha()});
		DrawCircleSectorLines(drawEndRight - Vector{static_cast<float>(m_CornerRadius), -static_cast<float>(m_CornerRadius)}, m_CornerRadius, 0, 90, 0, {m_Color, 0, 0, g_FrameMan.GetCurrentAlpha()});
		DrawCircleSectorLines(drawEndRight - Vector{static_cast<float>(m_CornerRadius), static_cast<float>(m_CornerRadius)}, m_CornerRadius, -90, 0, 0, {m_Color, 0, 0, g_FrameMan.GetCurrentAlpha()});
		DrawLine(drawStartRight.GetFloorIntX() + m_CornerRadius, drawStartRight.GetFloorIntY(), drawEndRight.GetFloorIntX() - m_CornerRadius, drawStartRight.GetFloorIntY(), {m_Color, 0, 0, g_FrameMan.GetCurrentAlpha()});
		DrawLine(drawStartRight.GetFloorIntX() + m_CornerRadius, drawEndRight.GetFloorIntY(), drawEndRight.GetFloorIntX() - m_CornerRadius, drawEndRight.GetFloorIntY(), {m_Color, 0, 0, g_FrameMan.GetCurrentAlpha()});
		DrawLine(drawStartRight.GetFloorIntX(), drawStartRight.GetFloorIntY() + m_CornerRadius, drawStartRight.GetFloorIntX(), drawEndRight.GetFloorIntY() - m_CornerRadius, {m_Color, 0, 0, g_FrameMan.GetCurrentAlpha()});
		DrawLine(drawEndRight.GetFloorIntX(), drawStartRight.GetFloorIntY() + m_CornerRadius, drawEndRight.GetFloorIntX(), drawEndRight.GetFloorIntY() - m_CornerRadius, {m_Color, 0, 0, g_FrameMan.GetCurrentAlpha()});
	}
}

void RoundedBoxFillPrimitive::Draw(BITMAP* drawScreen, const Vector& targetPos) {
	if (m_StartPos.m_X > m_EndPos.m_X) {
		std::swap(m_StartPos.m_X, m_EndPos.m_X);
	}
	if (m_StartPos.m_Y > m_EndPos.m_Y) {
		std::swap(m_StartPos.m_Y, m_EndPos.m_Y);
	}

	if (!g_SceneMan.SceneWrapsX() && !g_SceneMan.SceneWrapsY()) {
		Vector drawStart = m_StartPos - targetPos;
		Vector drawEnd = m_EndPos - targetPos;

		DrawCircle(drawStart.GetFloorIntX() + m_CornerRadius, drawStart.GetFloorIntY() + m_CornerRadius, m_CornerRadius, {m_Color, 0, 0, g_FrameMan.GetCurrentAlpha()});
		DrawCircle(drawStart.GetFloorIntX() + m_CornerRadius, drawEnd.GetFloorIntY() - m_CornerRadius, m_CornerRadius, {m_Color, 0, 0, g_FrameMan.GetCurrentAlpha()});
		DrawCircle(drawEnd.GetFloorIntX() - m_CornerRadius, drawStart.GetFloorIntY() + m_CornerRadius, m_CornerRadius, {m_Color, 0, 0, g_FrameMan.GetCurrentAlpha()});
		DrawCircle(drawEnd.GetFloorIntX() - m_CornerRadius, drawEnd.GetFloorIntY() - m_CornerRadius, m_CornerRadius, {m_Color, 0, 0, g_FrameMan.GetCurrentAlpha()});

		DrawRectangle(drawStart.GetFloorIntX(), drawStart.GetFloorIntY() + m_CornerRadius, drawEnd.GetFloorIntX() - drawStart.GetFloorIntX(), drawEnd.GetFloorIntY() - drawStart.GetFloorIntY() - 2 * m_CornerRadius, {m_Color, 0, 0, g_FrameMan.GetCurrentAlpha()});
		DrawRectangle(drawStart.GetFloorIntX() + m_CornerRadius, drawStart.GetFloorIntY(), drawEnd.GetFloorIntX() - drawStart.GetFloorIntX() - 2 * m_CornerRadius, drawEnd.GetFloorIntY() - drawStart.GetFloorIntY(), {m_Color, 0, 0, g_FrameMan.GetCurrentAlpha()});
	} else {
		Vector drawStartLeft;
		Vector drawEndLeft;
		Vector drawStartRight;
		Vector drawEndRight;

		TranslateCoordinates(targetPos, m_StartPos, drawStartLeft, drawStartRight);
		TranslateCoordinates(targetPos, m_EndPos, drawEndLeft, drawEndRight);

		DrawCircle(drawStartLeft.GetFloorIntX() + m_CornerRadius, drawStartLeft.GetFloorIntY() + m_CornerRadius, m_CornerRadius, {m_Color, 0, 0, g_FrameMan.GetCurrentAlpha()});
		DrawCircle(drawStartLeft.GetFloorIntX() + m_CornerRadius, drawEndLeft.GetFloorIntY() - m_CornerRadius, m_CornerRadius, {m_Color, 0, 0, g_FrameMan.GetCurrentAlpha()});
		DrawCircle(drawEndLeft.GetFloorIntX() - m_CornerRadius, drawStartLeft.GetFloorIntY() + m_CornerRadius, m_CornerRadius, {m_Color, 0, 0, g_FrameMan.GetCurrentAlpha()});
		DrawCircle(drawEndLeft.GetFloorIntX() - m_CornerRadius, drawEndLeft.GetFloorIntY() - m_CornerRadius, m_CornerRadius, {m_Color, 0, 0, g_FrameMan.GetCurrentAlpha()});
		DrawRectangle(drawStartLeft.GetFloorIntX(), drawStartLeft.GetFloorIntY() + m_CornerRadius, drawEndLeft.GetFloorIntX() - drawStartLeft.GetFloorIntX(), drawEndLeft.GetFloorIntY() - drawStartLeft.GetFloorIntY() - 2 * m_CornerRadius, {m_Color, 0, 0, g_FrameMan.GetCurrentAlpha()});
		DrawRectangle(drawStartLeft.GetFloorIntX() + m_CornerRadius, drawStartLeft.GetFloorIntY(), drawEndLeft.GetFloorIntX() - drawStartLeft.GetFloorIntX() - 2 * m_CornerRadius, drawEndLeft.GetFloorIntY() - drawStartLeft.GetFloorIntY(), {m_Color, 0, 0, g_FrameMan.GetCurrentAlpha()});

		DrawCircle(drawStartRight.GetFloorIntX() + m_CornerRadius, drawStartRight.GetFloorIntY() + m_CornerRadius, m_CornerRadius, {m_Color, 0, 0, g_FrameMan.GetCurrentAlpha()});
		DrawCircle(drawStartRight.GetFloorIntX() + m_CornerRadius, drawEndRight.GetFloorIntY() - m_CornerRadius, m_CornerRadius, {m_Color, 0, 0, g_FrameMan.GetCurrentAlpha()});
		DrawCircle(drawEndRight.GetFloorIntX() - m_CornerRadius, drawStartRight.GetFloorIntY() + m_CornerRadius, m_CornerRadius, {m_Color, 0, 0, g_FrameMan.GetCurrentAlpha()});
		DrawCircle(drawEndRight.GetFloorIntX() - m_CornerRadius, drawEndRight.GetFloorIntY() - m_CornerRadius, m_CornerRadius, {m_Color, 0, 0, g_FrameMan.GetCurrentAlpha()});
		DrawRectangle(drawStartRight.GetFloorIntX(), drawStartRight.GetFloorIntY() + m_CornerRadius, drawEndRight.GetFloorIntX() - drawStartRight.GetFloorIntX(), drawEndRight.GetFloorIntY() - drawStartRight.GetFloorIntY() - 2 * m_CornerRadius, {m_Color, 0, 0, g_FrameMan.GetCurrentAlpha()});
		DrawRectangle(drawStartRight.GetFloorIntX() + m_CornerRadius, drawStartRight.GetFloorIntY(), drawEndRight.GetFloorIntX() - drawStartRight.GetFloorIntX() - 2 * m_CornerRadius, drawEndRight.GetFloorIntY() - drawStartRight.GetFloorIntY(), {m_Color, 0, 0, g_FrameMan.GetCurrentAlpha()});
	}
}

void CirclePrimitive::Draw(BITMAP* drawScreen, const Vector& targetPos) {
	if (!g_SceneMan.SceneWrapsX() && !g_SceneMan.SceneWrapsY()) {
		Vector drawStart = m_StartPos - targetPos;
		DrawCircleLines(drawStart.GetFloorIntX(), drawStart.GetFloorIntY(), m_Radius, {m_Color, 0, 0, g_FrameMan.GetCurrentAlpha()});
	} else {
		Vector drawStartLeft;
		Vector drawStartRight;

		TranslateCoordinates(targetPos, m_StartPos, drawStartLeft, drawStartRight);

		DrawCircleLines(drawStartLeft.GetFloorIntX(), drawStartLeft.GetFloorIntY(), m_Radius, {m_Color, 0, 0, g_FrameMan.GetCurrentAlpha()});
		DrawCircleLines(drawStartRight.GetFloorIntX(), drawStartRight.GetFloorIntY(), m_Radius, {m_Color, 0, 0, g_FrameMan.GetCurrentAlpha()});
	}
}

void CircleFillPrimitive::Draw(BITMAP* drawScreen, const Vector& targetPos) {
	if (!g_SceneMan.SceneWrapsX() && !g_SceneMan.SceneWrapsY()) {
		Vector drawStart = m_StartPos - targetPos;
		DrawCircle(drawStart.GetFloorIntX(), drawStart.GetFloorIntY(), m_Radius, {m_Color, 0, 0, g_FrameMan.GetCurrentAlpha()});
	} else {
		Vector drawStartLeft;
		Vector drawStartRight;

		TranslateCoordinates(targetPos, m_StartPos, drawStartLeft, drawStartRight);

		DrawCircle(drawStartLeft.GetFloorIntX(), drawStartLeft.GetFloorIntY(), m_Radius, {m_Color, 0, 0, g_FrameMan.GetCurrentAlpha()});
		DrawCircle(drawStartRight.GetFloorIntX(), drawStartRight.GetFloorIntY(), m_Radius, {m_Color, 0, 0, g_FrameMan.GetCurrentAlpha()});
	}
}

void EllipsePrimitive::Draw(BITMAP* drawScreen, const Vector& targetPos) {
	if (!g_SceneMan.SceneWrapsX() && !g_SceneMan.SceneWrapsY()) {
		Vector drawStart = m_StartPos - targetPos;
		DrawEllipseLines(drawStart.GetFloorIntX(), drawStart.GetFloorIntY(), m_HorizRadius, m_VertRadius, {m_Color, 0, 0, g_FrameMan.GetCurrentAlpha()});
	} else {
		Vector drawStartLeft;
		Vector drawStartRight;

		TranslateCoordinates(targetPos, m_StartPos, drawStartLeft, drawStartRight);

		DrawEllipseLines(drawStartLeft.GetFloorIntX(), drawStartLeft.GetFloorIntY(), m_HorizRadius, m_VertRadius, {m_Color, 0, 0, g_FrameMan.GetCurrentAlpha()});
		DrawEllipseLines(drawStartRight.GetFloorIntX(), drawStartRight.GetFloorIntY(), m_HorizRadius, m_VertRadius, {m_Color, 0, 0, g_FrameMan.GetCurrentAlpha()});
	}
}

void EllipseFillPrimitive::Draw(BITMAP* drawScreen, const Vector& targetPos) {
	if (!g_SceneMan.SceneWrapsX() && !g_SceneMan.SceneWrapsY()) {
		Vector drawStart = m_StartPos - targetPos;
		DrawEllipse(drawStart.GetFloorIntX(), drawStart.GetFloorIntY(), m_HorizRadius, m_VertRadius, {m_Color, 0, 0, g_FrameMan.GetCurrentAlpha()});
	} else {
		Vector drawStartLeft;
		Vector drawStartRight;

		TranslateCoordinates(targetPos, m_StartPos, drawStartLeft, drawStartRight);

		DrawEllipse(drawStartLeft.GetFloorIntX(), drawStartLeft.GetFloorIntY(), m_HorizRadius, m_VertRadius, {m_Color, 0, 0, g_FrameMan.GetCurrentAlpha()});
		DrawEllipse(drawStartRight.GetFloorIntX(), drawStartRight.GetFloorIntY(), m_HorizRadius, m_VertRadius, {m_Color, 0, 0, g_FrameMan.GetCurrentAlpha()});
	}
}

void TrianglePrimitive::Draw(BITMAP* drawScreen, const Vector& targetPos) {
	if (!g_SceneMan.SceneWrapsX() && !g_SceneMan.SceneWrapsY()) {
		Vector drawPointA = m_PointAPos - targetPos;
		Vector drawPointB = m_PointBPos - targetPos;
		Vector drawPointC = m_PointCPos - targetPos;
		DrawLine(drawPointA.GetFloorIntX(), drawPointA.GetFloorIntY(), drawPointB.GetFloorIntX(), drawPointB.GetFloorIntY(), {m_Color, 0, 0, g_FrameMan.GetCurrentAlpha()});
		DrawLine(drawPointB.GetFloorIntX(), drawPointB.GetFloorIntY(), drawPointC.GetFloorIntX(), drawPointC.GetFloorIntY(), {m_Color, 0, 0, g_FrameMan.GetCurrentAlpha()});
		DrawLine(drawPointC.GetFloorIntX(), drawPointC.GetFloorIntY(), drawPointA.GetFloorIntX(), drawPointA.GetFloorIntY(), {m_Color, 0, 0, g_FrameMan.GetCurrentAlpha()});
	} else {
		Vector drawPointALeft;
		Vector drawPointBLeft;
		Vector drawPointCLeft;
		Vector drawPointARight;
		Vector drawPointBRight;
		Vector drawPointCRight;

		TranslateCoordinates(targetPos, m_PointAPos, drawPointALeft, drawPointARight);
		TranslateCoordinates(targetPos, m_PointBPos, drawPointBLeft, drawPointBRight);
		TranslateCoordinates(targetPos, m_PointCPos, drawPointCLeft, drawPointCRight);

		DrawLine(drawPointALeft.GetFloorIntX(), drawPointALeft.GetFloorIntY(), drawPointBLeft.GetFloorIntX(), drawPointBLeft.GetFloorIntY(), {m_Color, 0, 0, g_FrameMan.GetCurrentAlpha()});
		DrawLine(drawPointARight.GetFloorIntX(), drawPointARight.GetFloorIntY(), drawPointBRight.GetFloorIntX(), drawPointBRight.GetFloorIntY(), {m_Color, 0, 0, g_FrameMan.GetCurrentAlpha()});
		DrawLine(drawPointBLeft.GetFloorIntX(), drawPointBLeft.GetFloorIntY(), drawPointCLeft.GetFloorIntX(), drawPointCLeft.GetFloorIntY(), {m_Color, 0, 0, g_FrameMan.GetCurrentAlpha()});
		DrawLine(drawPointBRight.GetFloorIntX(), drawPointBRight.GetFloorIntY(), drawPointCRight.GetFloorIntX(), drawPointCRight.GetFloorIntY(), {m_Color, 0, 0, g_FrameMan.GetCurrentAlpha()});
		DrawLine(drawPointCLeft.GetFloorIntX(), drawPointCLeft.GetFloorIntY(), drawPointALeft.GetFloorIntX(), drawPointALeft.GetFloorIntY(), {m_Color, 0, 0, g_FrameMan.GetCurrentAlpha()});
		DrawLine(drawPointCRight.GetFloorIntX(), drawPointCRight.GetFloorIntY(), drawPointARight.GetFloorIntX(), drawPointARight.GetFloorIntY(), {m_Color, 0, 0, g_FrameMan.GetCurrentAlpha()});
	}
}

void TriangleFillPrimitive::Draw(BITMAP* drawScreen, const Vector& targetPos) {
	if (!g_SceneMan.SceneWrapsX() && !g_SceneMan.SceneWrapsY()) {
		Vector drawPointA = m_PointAPos - targetPos;
		Vector drawPointB = m_PointBPos - targetPos;
		Vector drawPointC = m_PointCPos - targetPos;
		DrawTriangle(drawPointA, drawPointB, drawPointC, {m_Color, 0, 0, g_FrameMan.GetCurrentAlpha()});
	} else {
		Vector drawPointALeft;
		Vector drawPointBLeft;
		Vector drawPointCLeft;
		Vector drawPointARight;
		Vector drawPointBRight;
		Vector drawPointCRight;

		TranslateCoordinates(targetPos, m_PointAPos, drawPointALeft, drawPointARight);
		TranslateCoordinates(targetPos, m_PointBPos, drawPointBLeft, drawPointBRight);
		TranslateCoordinates(targetPos, m_PointCPos, drawPointCLeft, drawPointCRight);

		DrawTriangle(drawPointALeft, drawPointBLeft, drawPointCLeft, {m_Color, 0, 0, g_FrameMan.GetCurrentAlpha()});
		DrawTriangle(drawPointARight, drawPointBRight, drawPointCRight, {m_Color, 0, 0, g_FrameMan.GetCurrentAlpha()});
	}
}

void PolygonPrimitive::Draw(BITMAP* drawScreen, const Vector& targetPos) {
	if (!g_SceneMan.SceneWrapsX() && !g_SceneMan.SceneWrapsY()) {
		Vector drawStart;
		Vector drawEnd;
		for (int i = 0; i < m_Vertices.size(); ++i) {
			drawStart = m_StartPos - targetPos + (*m_Vertices[i]);
			drawEnd = m_StartPos - targetPos + ((i + 1 < m_Vertices.size()) ? *m_Vertices[i + 1] : *m_Vertices[0]);
			DrawLine(drawStart.GetFloorIntX(), drawStart.GetFloorIntY(), drawEnd.GetFloorIntX(), drawEnd.GetFloorIntY(), {m_Color, 0, 0, g_FrameMan.GetCurrentAlpha()});
		}
	} else {
		Vector drawStartLeft;
		Vector drawStartRight;
		Vector drawEndLeft;
		Vector drawEndRight;
		for (int i = 0; i < m_Vertices.size(); ++i) {
			TranslateCoordinates(targetPos, m_StartPos + (*m_Vertices[i]), drawStartLeft, drawStartRight);
			TranslateCoordinates(targetPos, m_StartPos + ((i + 1 < m_Vertices.size()) ? *m_Vertices[i + 1] : *m_Vertices[0]), drawEndLeft, drawEndRight);

			DrawLine(drawStartLeft.GetFloorIntX(), drawStartLeft.GetFloorIntY(), drawEndLeft.GetFloorIntX(), drawEndLeft.GetFloorIntY(), {m_Color, 0, 0, g_FrameMan.GetCurrentAlpha()});
			DrawLine(drawStartRight.GetFloorIntX(), drawStartRight.GetFloorIntY(), drawEndRight.GetFloorIntX(), drawEndRight.GetFloorIntY(), {m_Color, 0, 0, g_FrameMan.GetCurrentAlpha()});
		}
	}
}

void PolygonFillPrimitive::Draw(BITMAP* drawScreen, const Vector& targetPos) {
	size_t drawPointsSize = m_Vertices.size();

	if (!g_SceneMan.SceneWrapsX() && !g_SceneMan.SceneWrapsY()) {
		Vector drawStart = m_StartPos - targetPos;

		std::vector<Vector2> drawPoints = {};
		drawPoints.reserve(drawPointsSize);

		for (const Vector* vertex: m_Vertices) {
			drawPoints.emplace_back(drawStart.GetFloorIntX() + vertex->GetFloorIntX(), drawStart.GetFloorIntY() + vertex->GetFloorIntY());
		}
		DrawTriangleStrip(drawPoints.data(), drawPoints.size(), {m_Color, 0, 0, g_FrameMan.GetCurrentAlpha()});
	} else {
		std::vector<Vector2> drawPointsLeft = {};
		drawPointsLeft.reserve(drawPointsSize);

		std::vector<Vector2> drawPointsRight = {};
		drawPointsRight.reserve(drawPointsSize);

		Vector drawPointLeft;
		Vector drawPointRight;
		for (const Vector* vertex: m_Vertices) {
			TranslateCoordinates(targetPos, m_StartPos + (*vertex), drawPointLeft, drawPointRight);

			drawPointsLeft.emplace_back(drawPointLeft.GetFloorIntX(), drawPointLeft.GetFloorIntY());
			drawPointsRight.emplace_back(drawPointRight.GetFloorIntX(), drawPointRight.GetFloorIntY());
		}
		DrawTriangleStrip(drawPointsLeft.data(), drawPointsLeft.size(), {m_Color, 0, 0, g_FrameMan.GetCurrentAlpha()});
		DrawTriangleStrip(drawPointsRight.data(), drawPointsRight.size(), {m_Color, 0, 0, g_FrameMan.GetCurrentAlpha()});
	}
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

	if (!g_SceneMan.SceneWrapsX() && !g_SceneMan.SceneWrapsY()) {
		Vector drawStart = m_StartPos - targetPos - targetPosAdjustment;

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
	} else {
		Vector drawStartLeft;
		Vector drawStartRight;

		TranslateCoordinates(targetPos - targetPosAdjustment, m_StartPos, drawStartLeft, drawStartRight);

		if (m_BlendMode > DrawBlendMode::NoBlend) {
			if (m_RotAngle != 0) {
				rotate_sprite_trans(drawScreen, tempDrawBitmap, drawStartLeft.GetFloorIntX(), drawStartLeft.GetFloorIntY(), ftofix(rotation.GetAllegroAngle()));
				rotate_sprite_trans(drawScreen, tempDrawBitmap, drawStartRight.GetFloorIntX(), drawStartRight.GetFloorIntY(), ftofix(rotation.GetAllegroAngle()));
			} else {
				draw_trans_sprite(drawScreen, tempDrawBitmap, drawStartLeft.GetFloorIntX(), drawStartLeft.GetFloorIntY());
				draw_trans_sprite(drawScreen, tempDrawBitmap, drawStartRight.GetFloorIntX(), drawStartRight.GetFloorIntY());
			}
		} else {
			if (m_RotAngle != 0) {
				rotate_sprite(drawScreen, tempDrawBitmap, drawStartLeft.GetFloorIntX(), drawStartLeft.GetFloorIntY(), ftofix(rotation.GetAllegroAngle()));
				rotate_sprite(drawScreen, tempDrawBitmap, drawStartRight.GetFloorIntX(), drawStartRight.GetFloorIntY(), ftofix(rotation.GetAllegroAngle()));
			} else {
				font->DrawAligned(&playerGUIBitmap, drawStartLeft.GetFloorIntX(), drawStartLeft.GetFloorIntY(), m_Text, m_Alignment);
				font->DrawAligned(&playerGUIBitmap, drawStartRight.GetFloorIntX(), drawStartRight.GetFloorIntY(), m_Text, m_Alignment);
			}
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

	if (!g_SceneMan.SceneWrapsX() && !g_SceneMan.SceneWrapsY()) {
		Vector drawStart = m_StartPos - targetPos;
		if (m_BlendMode > DrawBlendMode::NoBlend) {
			pivot_scaled_sprite_trans(drawScreen, bitmapToDraw, drawStart.GetFloorIntX(), drawStart.GetFloorIntY(), bitmapToDraw->w / 2, bitmapToDraw->h / 2, ftofix(rotation.GetAllegroAngle()), ftofix(1.0));
		} else {
			pivot_scaled_sprite(drawScreen, bitmapToDraw, drawStart.GetFloorIntX(), drawStart.GetFloorIntY(), bitmapToDraw->w / 2, bitmapToDraw->h / 2, ftofix(rotation.GetAllegroAngle()), ftofix(1.0));
		}
	} else {
		Vector drawStartLeft;
		Vector drawStartRight;

		TranslateCoordinates(targetPos, m_StartPos, drawStartLeft, drawStartRight);

		// Take into account the h-flipped pivot point
		if (m_BlendMode > DrawBlendMode::NoBlend) {
			pivot_scaled_sprite_trans(drawScreen, bitmapToDraw, drawStartLeft.GetFloorIntX(), drawStartLeft.GetFloorIntY(), bitmapToDraw->w / 2, bitmapToDraw->h / 2, ftofix(rotation.GetAllegroAngle()), ftofix(1.0));
			pivot_scaled_sprite_trans(drawScreen, bitmapToDraw, drawStartRight.GetFloorIntX(), drawStartRight.GetFloorIntY(), bitmapToDraw->w / 2, bitmapToDraw->h / 2, ftofix(rotation.GetAllegroAngle()), ftofix(1.0));
		} else {
			pivot_scaled_sprite(drawScreen, bitmapToDraw, drawStartLeft.GetFloorIntX(), drawStartLeft.GetFloorIntY(), bitmapToDraw->w / 2, bitmapToDraw->h / 2, ftofix(rotation.GetAllegroAngle()), ftofix(1.0));
			pivot_scaled_sprite(drawScreen, bitmapToDraw, drawStartRight.GetFloorIntX(), drawStartRight.GetFloorIntY(), bitmapToDraw->w / 2, bitmapToDraw->h / 2, ftofix(rotation.GetAllegroAngle()), ftofix(1.0));
		}
	}
	destroy_bitmap(bitmapToDraw);
}
