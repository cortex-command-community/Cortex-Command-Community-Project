#ifndef _BITMAPTOPOLYGONHELPER_
#define _BITMAPTOPOLYGONHELPER_

#include "box2d/box2d.h"

struct BITMAP;

namespace RTE {

	/// <summary>
	/// A helper for converting Bitmaps to polygons for colliders.
	/// </summary>
	class BitmapToPolygonHelper {
	public:
		b2PolygonShape ConvertBitmapToPolygon(BITMAP *bitmapToConvert, bool reducePolygon = true);

	private:
		b2Vec2 GetStartingPixelPosition(BITMAP *bitmapToScan);
		b2Vec2 GetNextVertexPosition(BITMAP *bitmapToScan, const b2Vec2 &currentPoint, const b2Vec2 &previousPoint);
		std::vector<b2Vec2> GetReducedPolygon(const std::vector<b2Vec2> &polygonVertices);
		bool IsInline(const b2Vec2 &v1, const b2Vec2 &v2, const b2Vec2 &v3);

		static const std::vector<b2Vec2> c_BitmapDirectionUnitVectors;
	};
}
#endif