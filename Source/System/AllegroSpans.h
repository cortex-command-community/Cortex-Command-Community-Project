#pragma once

#include "allegro.h"

#include <algorithm>
#include <array>
#include <cstdlib>
#include <limits>
#include <utility>

/// Walkers over the exact pixels that allegro's own fills cover, for filling bitmaps in ways allegro can't, like taking the max of what's already there.
namespace RTE {
	/// Calls spanFunction(y, startX, endX) for each row of pixels that allegro's rectfill() fills on a bitmap, clipped to the bitmap the same way. Both ends of a span are included.
	template <typename SpanFunction>
	void ForEachRectSpan(const BITMAP* bitmap, int x1, int y1, int x2, int y2, SpanFunction&& spanFunction) {
		if (x1 > x2) {
			std::swap(x1, x2);
		}
		if (y1 > y2) {
			std::swap(y1, y2);
		}
		x1 = std::max(x1, 0);
		x2 = std::min(x2, bitmap->w - 1);
		y1 = std::max(y1, 0);
		y2 = std::min(y2, bitmap->h - 1);
		if (x1 > x2) {
			return;
		}
		for (int y = y1; y <= y2; ++y) {
			spanFunction(y, x1, x2);
		}
	}

	/// Calls spanFunction(y, startX, endX) for each run of pixels that allegro's triangle() fills on a bitmap, clipped to the bitmap the same way. Both ends of a span are included.
	/// This is allegro's _soft_polygon() with the drawing taken out, so reveals cover exactly the pixels they did when they were drawn with triangle().
	/// That matters for see rays: allegro always fills a triangle's edge pixels, so a ray still reveals a line where its triangles are flat.
	template <typename SpanFunction>
	void ForEachTriangleSpan(const BITMAP* bitmap, int x1, int y1, int x2, int y2, int x3, int y3, SpanFunction&& spanFunction) {
		// Allegro's fixed point precision for polygon edges
		constexpr int fixShift = 18;

		struct Edge {
			int top;
			int bottom;
			int x;
			int dx;
			int w;
			Edge* prev;
			Edge* next;
		};

		// Inserts an edge into a list sorted by x or by top, before any edges equal to it, and returns the new head of the list. Same as allegro's _add_edge().
		auto addEdge = [](Edge* list, Edge* edge, bool sortByX) {
			Edge* pos = list;
			Edge* prev = nullptr;
			while (pos && (sortByX ? pos->x < edge->x : pos->top < edge->top)) {
				prev = pos;
				pos = pos->next;
			}
			edge->next = pos;
			edge->prev = prev;
			if (pos) {
				pos->prev = edge;
			}
			if (prev) {
				prev->next = edge;
				return list;
			}
			return edge;
		};

		// Removes an edge from a list, and returns the new head of the list. Same as allegro's _remove_edge().
		auto removeEdge = [](Edge* list, Edge* edge) {
			if (edge->next) {
				edge->next->prev = edge->prev;
			}
			if (edge->prev) {
				edge->prev->next = edge->next;
				return list;
			}
			return edge->next;
		};

		// Clips a span to the bitmap the same way allegro's hfill() does
		auto fillSpan = [&bitmap, &spanFunction](int y, int startX, int endX) {
			startX = std::max(startX, 0);
			endX = std::min(endX, bitmap->w - 1);
			if (startX <= endX && y >= 0 && y < bitmap->h) {
				spanFunction(y, startX, endX);
			}
		};

		// Set up an edge from each point to the one before it. Same as allegro's fill_edge_structure().
		const int points[6] = {x1, y1, x2, y2, x3, y3};
		std::array<Edge, 3> edges;
		Edge* inactiveEdges = nullptr;
		Edge* activeEdges = nullptr;
		int top = std::numeric_limits<int>::max();
		int bottom = std::numeric_limits<int>::min();
		const int* i1 = points;
		const int* i2 = points + 4;
		for (Edge& edge: edges) {
			const int* edgeTop = i2[1] < i1[1] ? i2 : i1;
			const int* edgeBottom = i2[1] < i1[1] ? i1 : i2;
			edge.top = edgeTop[1];
			edge.bottom = edgeBottom[1];
			edge.x = (edgeTop[0] << fixShift) + (1 << (fixShift - 1));
			if (edgeBottom[1] != edgeTop[1]) {
				edge.dx = ((edgeBottom[0] - edgeTop[0]) << fixShift) / (edgeBottom[1] - edgeTop[1]);
			} else {
				edge.dx = ((edgeBottom[0] - edgeTop[0]) << fixShift) << 1;
			}
			edge.w = std::max(std::abs(edge.dx) - 1, 0);
			edge.prev = nullptr;
			edge.next = nullptr;
			if (edge.dx < 0) {
				edge.x += edge.dx / 2;
			}

			top = std::min(top, edge.top);
			bottom = std::max(bottom, edge.bottom);
			inactiveEdges = addEdge(inactiveEdges, &edge, false);

			i2 = i1;
			i1 += 2;
		}
		bottom = std::min(bottom, bitmap->h - 1);

		for (int y = top; y <= bottom; ++y) {
			int hid = 0;
			int b1 = 0;
			int e1 = 0;
			int up = 0;
			int draw = 0;

			// Check for newly active edges
			Edge* edge = inactiveEdges;
			while (edge && edge->top == y) {
				Edge* nextEdge = edge->next;
				inactiveEdges = removeEdge(inactiveEdges, edge);
				activeEdges = addEdge(activeEdges, edge, true);
				edge = nextEdge;
			}

			// Find the horizontal spans
			for (edge = activeEdges; edge; edge = edge->next) {
				int e = edge->w;
				if (edge->bottom != y) {
					up = 1 - up;
				} else {
					e = edge->w >> 1;
				}
				if (edge->top == y) {
					e = edge->w >> 1;
				}

				if (draw < 1 && up >= 1) {
					b1 = (edge->x + e) >> fixShift;
				} else if (draw >= 1) {
					// Filling the triangle
					e1 = edge->x >> fixShift;
					hid = std::max(hid, b1 + 1);
					if (hid <= e1 - 1) {
						fillSpan(y, hid, e1 - 1);
					}
					b1 = (edge->x + e) >> fixShift;
				}

				// Filling the edge
				hid = std::max(hid, edge->x >> fixShift);
				if (hid <= ((edge->x + e) >> fixShift)) {
					fillSpan(y, hid, (edge->x + e) >> fixShift);
					hid = 1 + ((edge->x + e) >> fixShift);
				}

				draw = up;
			}

			// Update the edges, keeping them sorted and removing finished ones
			edge = activeEdges;
			while (edge) {
				Edge* nextEdge = edge->next;
				if (y >= edge->bottom) {
					activeEdges = removeEdge(activeEdges, edge);
				} else {
					edge->x += edge->dx;
					if (edge->top == y && edge->dx > 0) {
						edge->x -= edge->dx / 2;
					}
					if (edge->bottom == y + 1 && edge->dx < 0) {
						edge->x -= edge->dx / 2;
					}
					while (edge->prev && edge->x < edge->prev->x) {
						if (edge->next) {
							edge->next->prev = edge->prev;
						}
						edge->prev->next = edge->next;
						edge->next = edge->prev;
						edge->prev = edge->prev->prev;
						edge->next->prev = edge;
						if (edge->prev) {
							edge->prev->next = edge;
						} else {
							activeEdges = edge;
						}
					}
				}
				edge = nextEdge;
			}
		}
	}
} // namespace RTE
