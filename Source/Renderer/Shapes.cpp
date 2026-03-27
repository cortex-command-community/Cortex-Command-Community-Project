#include "Shapes.h"
#include "DrawCall.h"
#include "RenderMan.h"
#include "tracy/Tracy.hpp"

using namespace RTE;

Shape::Shape Shape::Pixel(glm::vec2 position, Color color) {
	return Rectangle(FloatRect(position.x, position.y, 1.0f, 1.0f), color);
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
	ZoneScoped;
	Shape circle;
	circle.m_Vertices.emplace_back(center, color);
	int segments = 180;
	float angle = 0.0f;
	for (int i = 0; i < segments; ++i) {
		if (i % 2 == 0) {
			circle.m_Indices.emplace_back(0);
			circle.m_Indices.emplace_back(i - 1);
		}
		glm::vec2 vertex = center + glm::vec2(std::cos(angle), std::sin(angle)) * radius;
		circle.m_Vertices.emplace_back(vertex, color);
		circle.m_Indices.emplace_back(i + 1);
		angle += DegreesToRadians(360.0f / segments);
	}
	return circle;
}
Shape::Shape Shape::CircleSector(glm::vec2 center, float radius, float startAngle, float endAngle, Color color) {
	return {};
}
Shape::Shape Shape::CircleLines(glm::vec2 center, float radius, Color color) {
	ZoneScoped;
	Shape circle;
	int segments = std::ceil(PI * radius);
	circle.m_Vertices.emplace_back(center + glm::vec2(radius - 0.5f, 0.0f), color);
	circle.m_Vertices.emplace_back(center + glm::vec2(radius + 0.5f, 0.0f), color);
	float angle = DegreesToRadians(360.0f / segments);
	for (int i = 0; i < segments * 2; i+=2) {
		circle.m_Vertices.emplace_back(center + glm::vec2(std::cos(angle), std::sin(angle)) * (radius - 0.5f), color);
		circle.m_Vertices.emplace_back(center + glm::vec2(std::cos(angle), std::sin(angle)) * (radius + 0.5f), color);
		circle.m_Indices.insert(
		    circle.m_Indices.end(),
		    {i, i + 1, i + 2,
		     i + 1, i + 3, i + 2});
		angle += DegreesToRadians(360.0f / segments);
	}
	return circle;
}
Shape::Shape Shape::CircleLinesSector(glm::vec2 center, float radius, float startAngle, float endAngle, Color color) {
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
Shape::Shape Shape::Rectangle(FloatRect rect, Color color) {
	return Rectangle(rect, FloatRect(0.0f, 0.0f, 1.0f, 1.0f), color);
}

Shape::Shape Shape::Rectangle(FloatRect rect, FloatRect uv, Color color) {
	ZoneScoped;
	Shape rectangle;
	rectangle.m_Vertices = {
	    Vertex(glm::vec2(rect.x, rect.y), glm::vec2(uv.x, uv.y), color),
	    Vertex(glm::vec2(rect.x, rect.y) + glm::vec2(rect.w, rect.h), {uv.x + uv.w, uv.y + uv.h}, color),
	    Vertex(glm::vec2(rect.x, rect.y) + glm::vec2(rect.w, 0.0f), {uv.x + uv.w, uv.y}, color),
	    Vertex(glm::vec2(rect.x, rect.y) + glm::vec2(0.0f, rect.h), {uv.x, uv.y + uv.h}, color)};

	rectangle.m_Indices = {
		0, 1, 2,
		0, 3, 1
	};

	return rectangle;
}

Shape::Shape Shape::RectangleLines(FloatRect rect, Color color) {
	ZoneScoped;
	Shape rectangle;
	rectangle.m_Vertices = {
		Vertex(glm::vec2(rect.x, rect.y), {0.0f, 0.0f}, color),
		Vertex(glm::vec2(rect.x, rect.y) + glm::vec2(rect.w, rect.h), {1.0f, 1.0f}, color),
		Vertex(glm::vec2(rect.x, rect.y) + glm::vec2(rect.w, 0.0f), {1.0f, 0.0f}, color),
		Vertex(glm::vec2(rect.x, rect.y) + glm::vec2(0.0f, rect.h), {0.0f, 1.0f}, color),
		Vertex(glm::vec2(rect.x, rect.y) + glm::vec2(1.0f, 1.0f), {0.0f, 0.0f}, color),
		Vertex(glm::vec2(rect.x, rect.y) + glm::vec2(rect.w - 1.0f, rect.h - 1.0f), {1.0f, 1.0f}, color),
		Vertex(glm::vec2(rect.x, rect.y) + glm::vec2(rect.w - 1.0f, 0.0f), {1.0f, 0.0f}, color),
		Vertex(glm::vec2(rect.x, rect.y) + glm::vec2(0.0f, rect.h - 1.0f), {0.0f, 1.0f}, color)
	};

	rectangle.m_Indices = {
		0, 4, 2,
		4, 6, 2,
		2, 6, 1,
		1, 6, 5,
		5, 3, 1,
		5, 7, 3,
		0, 3, 7,
		0, 7, 4
	};

	return rectangle;
}
Shape::Shape Shape::RoundedRectangle(FloatRect rect, float cornerRadius, Color color) {
	return {};
}
Shape::Shape Shape::RoundedRectangleLines(FloatRect rect, float cornerRadius, Color color) {
	return {};
}
Shape::Shape Shape::RoundedRectangleLines(FloatRect rect, float cornerRadius, float thickness, Color color) {
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

Shape::Shape Shape::Lines::VectorArrow(glm::vec2 pos, glm::vec2 vector, Color color) {
	ZoneScoped;
	Shape arrow;
	arrow.m_Vertices = {
	    Vertex(pos, color),
	    Vertex(pos + vector, color),
	    Vertex(pos + vector - (std::sqrt(2.0f) / 10.0f * glm::vec2(vector.x - vector.y, vector.x + vector.y)), color),
	    Vertex(pos + vector - (std::sqrt(2.0f) / 10.0f * glm::vec2(vector.x + vector.y, -vector.x + vector.y)), color)};

	arrow.m_Indices = {0, 1, 1, 2, 1, 3};

	return arrow;
}

Shape::Shape Shape::Lines::Rectangle(const FloatRect& rect, Color color) {
	Shape rectangle;
	rectangle.m_Vertices = {
	    Vertex(glm::vec2(rect.x, rect.y), color),
	    Vertex(glm::vec2(rect.x + rect.w, rect.y), color),
	    Vertex(glm::vec2(rect.x + rect.w, rect.y + rect.h), color),
	    Vertex(glm::vec2(rect.x, rect.y + rect.h), color),
	};

	rectangle.m_Indices = {0, 1, 2, 3};
	return rectangle;
}

std::shared_ptr<DrawCall> Draw::Pixel(glm::vec2 position, Color color) {
	return nullptr;
}

std::shared_ptr<DrawCall> Draw::Line(glm::vec2 start, glm::vec2 end, Color color) {
	return nullptr;
}

std::shared_ptr<DrawCall> Draw::Line(glm::vec2 start, glm::vec2 end, float thickness, Color color) {
	return nullptr;
}

std::shared_ptr<DrawCall> Draw::LineStrip(const std::vector<glm::vec2>& points, Color color) {
	return nullptr;
}

std::shared_ptr<DrawCall> Draw::LineStrip(const std::vector<glm::vec2>& points, float thickness, Color color) {
	return nullptr;
}

std::shared_ptr<DrawCall> Draw::LineBezier(glm::vec2 start, glm::vec2 end, Color color) {
	return nullptr;
}

std::shared_ptr<DrawCall> Draw::Circle(glm::vec2 center, float radius, Color color) {
	return nullptr;
}

std::shared_ptr<DrawCall> Draw::CircleSector(glm::vec2 center, float radius, float startAngle, float endAngle, Color color) {
	return nullptr;
}

std::shared_ptr<DrawCall> Draw::CircleLines(glm::vec2 center, float radius, Color color) {
	ZoneScoped;
	std::shared_ptr<DrawCall> draw = g_RenderMan.BeginDraw();
	Shape::Shape circle = Shape::CircleLines(center, radius, color);
	draw->m_Vertices = std::move(circle.m_Vertices);
	draw->m_Indices = std::move(circle.m_Indices);
	draw->m_TextureId = g_RenderMan.GetShapeTexture();
	draw->m_Indexed = false;
	return draw;
}

std::shared_ptr<DrawCall> Draw::CircleLinesSector(glm::vec2 center, float radius, Color color) {
	return nullptr;
}

std::shared_ptr<DrawCall> Draw::Ellipse(glm::vec2 center, float radiusH, float radiusV, Color color) {
	return nullptr;
}

std::shared_ptr<DrawCall> Draw::EllipseLines(glm::vec2 center, float radiusH, float radiusV, Color color) {
	return nullptr;
}

std::shared_ptr<DrawCall> Draw::Ring(glm::vec2 center, float innerRadius, float outerRadius, float startAngle, float endAngle, Color color) {
	return nullptr;
}

std::shared_ptr<DrawCall> Draw::RingLines(glm::vec2 center, float innerRadius, float outerRadius, float startAngle, float endAngle, Color color) {
	return nullptr;
}

std::shared_ptr<DrawCall> Draw::Rectangle(FloatRect rect, Color color) {
	ZoneScoped;
	std::shared_ptr<DrawCall> draw = g_RenderMan.BeginDraw();
	Shape::Shape rectangle = Shape::Rectangle(rect, color);
	draw->m_Vertices = std::move(rectangle.m_Vertices);
	draw->m_Indices = std::move(rectangle.m_Indices);
	draw->m_TextureId = g_RenderMan.GetShapeTexture();
	draw->m_Indexed = false;
	return draw;
}

std::shared_ptr<DrawCall> Draw::RectangleLines(FloatRect rect, Color color) {
	ZoneScoped;
	std::shared_ptr<DrawCall> draw = g_RenderMan.BeginDraw();
	Shape::Shape rectangle = Shape::RectangleLines(rect, color);
	draw->m_Vertices = std::move(rectangle.m_Vertices);
	draw->m_Indices = std::move(rectangle.m_Indices);
	draw->m_TextureId = g_RenderMan.GetShapeTexture();
	draw->m_Indexed = false;
	return draw;
}

std::shared_ptr<DrawCall> Draw::RoundedRectangle(FloatRect rect, float cornerRadius, Color color) {
	return nullptr;
}

std::shared_ptr<DrawCall> Draw::RoundedRectangleLines(FloatRect rect, float cornerRadius, Color color) {
	return nullptr;
}

std::shared_ptr<DrawCall> Draw::RoundedRectangleLines(FloatRect rect, float cornerRadius, float thickness, Color color) {
	return nullptr;
}

std::shared_ptr<DrawCall> Draw::Triangle(glm::vec2 point1, glm::vec2 point2, glm::vec2 point3, Color color) {
	return nullptr;
}

std::shared_ptr<DrawCall> Draw::TriangleLines(glm::vec2 point1, glm::vec2 point2, glm::vec2 point3, Color color) {
	return nullptr;
}

std::shared_ptr<DrawCall> Draw::TriangleStrip(std::vector<glm::vec2> points, Color color) {
	return nullptr;
}

std::shared_ptr<DrawCall> Draw::Polygon(std::vector<glm::vec2> points, Color color) {
	return nullptr;
}

std::shared_ptr<DrawCall> Draw::Lines::VectorArrow(glm::vec2 pos, glm::vec2 vector, Color color) {
	ZoneScoped;
	std::shared_ptr<DrawCall> draw = g_RenderMan.BeginDraw();
	Shape::Shape arrow = Shape::Lines::VectorArrow(pos, vector, color);
	draw->m_DrawMode = GL_LINES;
	draw->m_Vertices = std::move(arrow.m_Vertices);
	draw->m_Indices = std::move(arrow.m_Indices);
	draw->m_Indexed = false;
	return draw;
}

std::shared_ptr<DrawCall> Draw::Lines::Rectangle(const FloatRect& rect, Color color) {
	std::shared_ptr<DrawCall> draw = g_RenderMan.BeginDraw();
	Shape::Shape rectangle = Shape::Lines::Rectangle(rect, color);
	draw->m_DrawMode = GL_LINE_LOOP;
	draw->m_Vertices = std::move(rectangle.m_Vertices);
	draw->m_Indices = std::move(rectangle.m_Indices);
	draw->m_Indexed = false;
	return draw;
}
