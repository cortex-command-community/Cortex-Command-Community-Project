#pragma once
#include "Color.h"
#include "glm/fwd.hpp"
#include <vector>
#include "Vertex.h"

namespace RTE {
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
		Shape Rectangle(Box rect, Color color);
		Shape Rectangle(Box rect, float angle, Color color);
		Shape RectangleLines(Box rect, Color color);
		Shape RoundedRectangle(Box rect, float cornerRadius, Color color);
		Shape RoundedRectangleLines(Box rect, float cornerRadius, Color color);
		Shape RoundedRectangleLines(Box rect, float cornerRadius, float thickness, Color color);
		Shape Triangle(glm::vec2 point1, glm::vec2 point2, glm::vec2 point3, Color color);
		Shape TriangleLines(glm::vec2 point1, glm::vec2 point2, glm::vec2 point3, Color color);
		Shape TriangleStrip(std::vector<glm::vec2> points, Color color);
		Shape Polygon(std::vector<glm::vec2> points, Color color);
	} // namespace Shape
	namespace Draw {
		void Pixel(glm::vec2 position, Color color);
		void Line(glm::vec2 start, glm::vec2 end, Color color);
		void Line(glm::vec2 start, glm::vec2 end, float thickness, Color color);
		void LineStrip(const std::vector<glm::vec2>& points, Color color);
		void LineStrip(const std::vector<glm::vec2>& points, float thickness, Color color);
		void LineBezier(glm::vec2 start, glm::vec2 end, Color color);
		void Circle(glm::vec2 center, float radius, Color color);
		void CircleSector(glm::vec2 center, float radius, float startAngle, float endAngle, Color color);
		void CircleLines(glm::vec2 center, float radius, Color color);
		void CircleLinesSector(glm::vec2 center, float radius, Color color);
		void Ellipse(glm::vec2 center, float radiusH, float radiusV, Color color);
		void EllipseLines(glm::vec2 center, float radiusH, float radiusV, Color color);
		void Ring(glm::vec2 center, float innerRadius, float outerRadius, float startAngle, float endAngle, Color color);
		void RingLines(glm::vec2 center, float innerRadius, float outerRadius, float startAngle, float endAngle, Color color);
		void Rectangle(Box rect, Color color);
		void RectangleLines(Box rect, Color color);
		void RoundedRectangle(Box rect, float cornerRadius, Color color);
		void RoundedRectangleLines(Box rect, float cornerRadius, Color color);
		void RoundedRectangleLines(Box rect, float cornerRadius, float thickness, Color color);
		void Triangle(glm::vec2 point1, glm::vec2 point2, glm::vec2 point3, Color color);
		void TriangleLines(glm::vec2 point1, glm::vec2 point2, glm::vec2 point3, Color color);
		void TriangleStrip(std::vector<glm::vec2> points, Color color);
		void Polygon(std::vector<glm::vec2> points, Color color);
	} // namespace Draw
} // namespace RTE
