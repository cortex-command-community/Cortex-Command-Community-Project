#ifndef _RTEPRIMITIVES_
#define _RTEPRIMITIVES_
#include "Interface/ShapeInterface.h"
#include "Interface/RendererInterface.h"

namespace RTE {
	/// <summary>
	/// Draws a line from start to end with color
	/// </summary>
	/// <param name="renderer">The renderer to draw with</param>
	/// <param name="start">The start point of the line</param>
	/// <param name="end">The end point of the line</param>
	/// <param name="color">The color of the line</param>
	extern void DrawLine(Renderer* renderer, glm::vec2 start, glm::vec2 end, glm::vec4 color);

	/// <summary>
	/// Draws a rectangle from topLeft to bottomRight with color
	/// </summary>
	/// <param name="renderer">The renderer to draw with</param>
	/// <param name="topLeft">The top left point of the rectangle</param>
	/// <param name="bottomRight">The bottom right point of the rectangle</param>
	/// <param name="color">The color of the rectangle</param>
	/// <param name="fill">Whether or not to fill the rectangle</param>
	extern void DrawRectangle(Renderer* renderer, glm::vec2 topLeft, glm::vec2 bottomRight, glm::vec4 color, bool fill = false);

	/// <summary>
	/// Draws a rectangle from topLeft with width and height with color
	/// </summary>
	/// <param name="renderer">The renderer to draw with</param>
	/// <param name="topLeft">The top left point of the rectangle</param>
	/// <param name="width">The width of the rectangle</param>
	/// <param name="height">The height of the rectangle</param>
	/// <param name="color">The color of the rectangle</param>
	/// <param name="fill">Whether or not to fill the rectangle</param>
	extern void DrawRectangle(Renderer* renderer, glm::vec2 topLeft, float width, float height, glm::vec4 color, bool fill = false);

	/// <summary>
	/// Draws a circle with center and radius with color
	/// </summary>
	/// <param name="renderer">The renderer to draw with</param>
	/// <param name="center">The center point of the circle</param>
	/// <param name="radius">The radius of the circle</param>
	/// <param name="color">The color of the circle</param>
	/// <param name="fill">Whether or not to fill the circle</param>
	extern void DrawCircle(Renderer* renderer, glm::vec2 center, float radius, glm::vec4 color, bool fill = false);

	/// <summary>
	/// Draws an arc with center, radius, startAngle, endAngle, and color.
	/// </summary>
	/// <param name="renderer">The renderer to draw with</param>
	/// <param name="center">The center point of the arc</param>
	/// <param name="radius">The radius of the arc</param>
	/// <param name="startAngle">The start angle of the arc</param>
	/// <param name="endAngle">The end angle of the arc</param>
	/// <param name="color">The color of the arc</param>
	extern void DrawArc(Renderer* renderer, glm::vec2 center, float radius, float startAngle, float endAngle, glm::vec4 color);

	/// <summary>
	/// Draws a polygon with start, shape, and color.
	/// </summary>
	/// <param name="renderer">The renderer to draw with</param>
	/// <param name="start">The start point of the polygon</param>
	/// <param name="shape">The shape of the polygon</param>
	/// <param name="color">The color of the polygon</param>
	/// <param name="fill">Whether or not to fill the polygon</param>
	extern void DrawPolygon(Renderer* renderer, glm::vec2 start, Shape* shape, glm::vec4 color, bool fill = false);

	/// <summary>
	/// Draws a triangle with p1, p2, p3, and color.
	/// </summary>
	/// <param name="renderer">The renderer to draw with.</param>
	/// <param name="p1">The first point of the triangle.</param>
	/// <param name="p2">The second point of the triangle.</param>
	/// <param name="p3">The third point of the triangle.</param>
	/// <param name="color">The color of the triangle.</param>
	/// <param name="fill">Whether or not to fill the triangle.</param>
	extern void DrawTriangle(Renderer* renderer, glm::vec2 p1, glm::vec2 p2, glm::vec2 p3, glm::vec4 color, bool fill = false);

	/// <summary>
	/// Draws a regular polygon with center, radius, sides, and color.
	/// </summary>
	/// <param name="renderer">The renderer to draw with.</param>
	/// <param name="center">The center point of the polygon.</param>
	/// <param name="radius">The radius of the polygon.</param>
	/// <param name="sides">The number of sides of the polygon.</param>
	/// <param name="color">The color of the polygon.</param>
	/// <param name="fill">Whether or not to fill the polygon.</param>
	extern void DrawRegularPolygon(Renderer* renderer, glm::vec2 center, float radius, int sides, glm::vec4 color, bool fill = false);
}
#endif