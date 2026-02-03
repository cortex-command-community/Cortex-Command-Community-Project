#pragma once
#include "Color.h"
#include "glm/fwd.hpp"
#include <vector>
#include <memory>
#include "Vertex.h"

namespace RTE {
	class DrawCall;
	namespace Shape {
		struct Shape {
			std::vector<Vertex> m_Vertices{};
			std::vector<int> m_Indices{};
		};
		Shape Pixel(glm::vec2 position, Color color);
		Shape Line(glm::vec2 start, glm::vec2 end, Color color);
		Shape Line(glm::vec2 start, glm::vec2 end, float thickness, Color color);
		Shape LineStrip(const std::vector<glm::vec2>& points, Color color);
		Shape LineStrip(const std::vector<glm::vec2>& points, float thickness, Color color);
		Shape LineBezier(glm::vec2 start, glm::vec2 end, Color color);
		Shape Circle(glm::vec2 center, float radius, Color color);
		Shape CircleSector(glm::vec2 center, float radius, float startAngle, float endAngle, Color color);
		Shape CircleLines(glm::vec2 center, float radius, Color color);
		Shape CircleLinesSector(glm::vec2 center, float radius, float startAngle, float endAngle, Color color);
		Shape Ellipse(glm::vec2 center, float radiusH, float radiusV, Color color);
		Shape EllipseLines(glm::vec2 center, float radiusH, float radiusV, Color color);
		Shape Ring(glm::vec2 center, float innerRadius, float outerRadius, float startAngle, float endAngle, Color color);
		Shape RingLines(glm::vec2 center, float innerRadius, float outerRadius, float startAngle, float endAngle, Color color);
		Shape Rectangle(FloatRect rect, Color color);
		Shape Rectangle(FloatRect rect, float angle, Color color);
		Shape Rectangle(FloatRect rect, FloatRect uv, Color color);
		Shape RectangleLines(FloatRect rect, Color color);
		Shape RoundedRectangle(FloatRect rect, float cornerRadius, Color color);
		Shape RoundedRectangleLines(FloatRect rect, float cornerRadius, Color color);
		Shape RoundedRectangleLines(FloatRect rect, float cornerRadius, float thickness, Color color);
		Shape Triangle(glm::vec2 point1, glm::vec2 point2, glm::vec2 point3, Color color);
		Shape TriangleLines(glm::vec2 point1, glm::vec2 point2, glm::vec2 point3, Color color);
		Shape TriangleStrip(std::vector<glm::vec2> points, Color color);
		Shape Polygon(std::vector<glm::vec2> points, Color color);
		namespace Lines {
			Shape VectorArrow(glm::vec2 pos, glm::vec2 vector, Color color);
		}
	} // namespace Shape
	namespace Draw {
		std::shared_ptr<DrawCall> Pixel(glm::vec2 position, Color color);
		std::shared_ptr<DrawCall> Line(glm::vec2 start, glm::vec2 end, Color color);
		std::shared_ptr<DrawCall> Line(glm::vec2 start, glm::vec2 end, float thickness, Color color);
		std::shared_ptr<DrawCall> LineStrip(const std::vector<glm::vec2>& points, Color color);
		std::shared_ptr<DrawCall> LineStrip(const std::vector<glm::vec2>& points, float thickness, Color color);
		std::shared_ptr<DrawCall> LineBezier(glm::vec2 start, glm::vec2 end, Color color);
		std::shared_ptr<DrawCall> Circle(glm::vec2 center, float radius, Color color);
		std::shared_ptr<DrawCall> CircleSector(glm::vec2 center, float radius, float startAngle, float endAngle, Color color);
		std::shared_ptr<DrawCall> CircleLines(glm::vec2 center, float radius, Color color);
		std::shared_ptr<DrawCall> CircleLinesSector(glm::vec2 center, float radius, Color color);
		std::shared_ptr<DrawCall> Ellipse(glm::vec2 center, float radiusH, float radiusV, Color color);
		std::shared_ptr<DrawCall> EllipseLines(glm::vec2 center, float radiusH, float radiusV, Color color);
		std::shared_ptr<DrawCall> Ring(glm::vec2 center, float innerRadius, float outerRadius, float startAngle, float endAngle, Color color);
		std::shared_ptr<DrawCall> RingLines(glm::vec2 center, float innerRadius, float outerRadius, float startAngle, float endAngle, Color color);
		std::shared_ptr<DrawCall> Rectangle(FloatRect rect, Color color);
		std::shared_ptr<DrawCall> RectangleLines(FloatRect rect, Color color);
		std::shared_ptr<DrawCall> RoundedRectangle(FloatRect rect, float cornerRadius, Color color);
		std::shared_ptr<DrawCall> RoundedRectangleLines(FloatRect rect, float cornerRadius, Color color);
		std::shared_ptr<DrawCall> RoundedRectangleLines(FloatRect rect, float cornerRadius, float thickness, Color color);
		std::shared_ptr<DrawCall> Triangle(glm::vec2 point1, glm::vec2 point2, glm::vec2 point3, Color color);
		std::shared_ptr<DrawCall> TriangleLines(glm::vec2 point1, glm::vec2 point2, glm::vec2 point3, Color color);
		std::shared_ptr<DrawCall> TriangleStrip(std::vector<glm::vec2> points, Color color);
		std::shared_ptr<DrawCall> Polygon(std::vector<glm::vec2> points, Color color);
		namespace Lines {
			std::shared_ptr<DrawCall> VectorArrow(glm::vec2 pos, glm::vec2 vector, Color color);
		}
	} // namespace Draw
} // namespace RTE
