#include "Shapes.h"
#include "DrawCall.h"
#include "RenderMan.h"

using namespace RTE;

Shape::Shape Shape::Pixel(glm::vec2 position, Color color) {
	return {};
}
Shape::Shape Shape::Line(glm::vec2 start, glm::vec2 end, Color color) {
	return {};
}
Shape::Shape Shape::Line(glm::vec2 start, glm::vec2 end, float thickness, Color color) {
	return {};
}
Shape::Shape Shape::LineStrip(const std::vector<glm::vec2>& points, Color color) {
	return {};
}
Shape::Shape Shape::LineStrip(const std::vector<glm::vec2>& points, float thickness, Color color) {
	return {};
}
Shape::Shape Shape::LineBezier(glm::vec2 start, glm::vec2 end, Color color) {
	return {};
}
Shape::Shape Shape::Circle(glm::vec2 center, float radius, Color color) {
	return {};
}
Shape::Shape Shape::CircleSector(glm::vec2 center, float radius, float startAngle, float endAngle, Color color) {
	return {};
}
Shape::Shape Shape::CircleLines(glm::vec2 center, float radius, Color color) {
	return {};
}
Shape::Shape Shape::CircleLinesSector(glm::vec2 center, float radius, Color color) {
	return {};
}
Shape::Shape Shape::Ellipse(glm::vec2 center, float radiusH, float radiusV, Color color) {
	return {};
}
Shape::Shape Shape::EllipseLines(glm::vec2 center, float radiusH, float radiusV, Color color) {
	return {};
}
Shape::Shape Shape::Ring(glm::vec2 center, float innerRadius, float outerRadius, float startAngle, float endAngle, Color color) {
	return {};
}
Shape::Shape Shape::RingLines(glm::vec2 center, float innerRadius, float outerRadius, float startAngle, float endAngle, Color color) {
	return {};
}
Shape::Shape Shape::Rectangle(Box rect, Color color) {
	Shape rectangle;
	rectangle.m_Vertices = {
	    Vertex(rect.m_Corner, {0.0f, 0.0f}, color),
	    Vertex(rect.m_Corner + Vector(rect.m_Width, rect.m_Height), {1.0f, 1.0f}, color),
	    Vertex(rect.m_Corner + Vector(rect.m_Width, 0.0f), {1.0f, 0.0f}, color),
	    Vertex(rect.m_Corner + Vector(0.0f, rect.m_Height), {0.0f, 1.0f}, color)};

	rectangle.m_Indices = {
		0, 1, 2,
		0, 3, 1
	};

	return rectangle;
}
Shape::Shape Shape::RectangleLines(Box rect, Color color) {
	return {};
}
Shape::Shape Shape::RoundedRectangle(Box rect, float cornerRadius, Color color) {
	return {};
}
Shape::Shape Shape::RoundedRectangleLines(Box rect, float cornerRadius, Color color) {
	return {};
}
Shape::Shape Shape::RoundedRectangleLines(Box rect, float cornerRadius, float thickness, Color color) {
	return {};
}
Shape::Shape Shape::Triangle(glm::vec2 point1, glm::vec2 point2, glm::vec2 point3, Color color) {
	return {};
}
Shape::Shape Shape::TriangleLines(glm::vec2 point1, glm::vec2 point2, glm::vec2 point3, Color color) {
	return {};
}
Shape::Shape Shape::TriangleStrip(std::vector<glm::vec2> points, Color color) {
	return {};
}
Shape::Shape Shape::Polygon(std::vector<glm::vec2> points, Color color) {
	return {};
}

void Draw::Pixel(glm::vec2 position, Color color) {
}

void Draw::Line(glm::vec2 start, glm::vec2 end, Color color) {
}

void Draw::Line(glm::vec2 start, glm::vec2 end, float thickness, Color color) {
}

void Draw::LineStrip(const std::vector<glm::vec2>& points, Color color) {
}

void Draw::LineStrip(const std::vector<glm::vec2>& points, float thickness, Color color) {
}

void Draw::LineBezier(glm::vec2 start, glm::vec2 end, Color color) {
}

void Draw::Circle(glm::vec2 center, float radius, Color color) {
}

void Draw::CircleSector(glm::vec2 center, float radius, float startAngle, float endAngle, Color color) {
}

void Draw::CircleLines(glm::vec2 center, float radius, Color color) {
	DrawCall draw;
	draw.m_Vertices = Shape::CircleLines(center, radius, color);
	draw.textureId = g_RenderMan.GetShapeTexture();
	g_RenderMan.PushDraw(std::move(draw));
}

void Draw::CircleLinesSector(glm::vec2 center, float radius, Color color) {
}

void Draw::Ellipse(glm::vec2 center, float radiusH, float radiusV, Color color) {
}

void Draw::EllipseLines(glm::vec2 center, float radiusH, float radiusV, Color color) {
}
void Draw::Ring(glm::vec2 center, float innerRadius, float outerRadius, float startAngle, float endAngle, Color color) {
}
void Draw::RingLines(glm::vec2 center, float innerRadius, float outerRadius, float startAngle, float endAngle, Color color) {
}
void Draw::Rectangle(Box rect, Color color) {
}
void Draw::RectangleLines(Box rect, Color color) {
}
void Draw::RoundedRectangle(Box rect, float cornerRadius, Color color) {
}
void Draw::RoundedRectangleLines(Box rect, float cornerRadius, Color color) {
}
void Draw::RoundedRectangleLines(Box rect, float cornerRadius, float thickness, Color color) {
}
void Draw::Triangle(glm::vec2 point1, glm::vec2 point2, glm::vec2 point3, Color color) {
}
void Draw::TriangleLines(glm::vec2 point1, glm::vec2 point2, glm::vec2 point3, Color color) {
}
void Draw::TriangleStrip(std::vector<glm::vec2> points, Color color) {
}
void Draw::Polygon(std::vector<glm::vec2> points, Color color) {
}
