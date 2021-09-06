#include "BitmapToPolygonHelper.h"
#include "psimpl.h"
#include "RTETools.h"

namespace RTE {

	const std::vector<b2Vec2> BitmapToPolygonHelper::c_BitmapDirectionUnitVectors = {
		b2Vec2(1.0F, 1.0F),
		b2Vec2(0.0F, 1.0F),
		b2Vec2(-1.0F, 1.0F),
		b2Vec2(-1.0F, 0.0F),
		b2Vec2(-1.0F, -1.0F),
		b2Vec2(0.0F, -1.0F),
		b2Vec2(1.0F, -1.0F),
		b2Vec2(1.0F, 0.0F)
	};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	b2PolygonShape BitmapToPolygonHelper::ConvertBitmapToPolygon(BITMAP *bitmapToConvert, bool reducePolygon) {
		b2Vec2 startPoint = GetStartingPixelPosition(bitmapToConvert);
		b2Vec2 currentPoint = startPoint;
		b2Vec2 previousPoint = startPoint;

		std::vector<b2Vec2> polygonPoints = {startPoint};
		while (polygonPoints.size() == 1 || currentPoint != startPoint) {
			currentPoint = GetNextVertexPosition(bitmapToConvert, currentPoint, previousPoint);
			previousPoint = polygonPoints[polygonPoints.size() - 1];
			polygonPoints.push_back(currentPoint);
		}

		std::vector<b2Vec2> reducedPolygonPoints = GetReducedPolygon(polygonPoints);
		b2PolygonShape bitmapAsPolygon;
		bitmapAsPolygon.Set(reducedPolygonPoints.data(), reducedPolygonPoints.size());

		//Convex

		return reducePolygon ? GetReducedPolygon(polygonPoints) : polygonPoints;

		/*
		std::vector<float> polygonPointsAsFloats;
		std::for_each(polygonPoints.begin(), polygonPoints.end(), [&polygonPointsAsFloats](const Vector &polygonPoint) {
			polygonPointsAsFloats.push_back(polygonPoint.GetX());
			polygonPointsAsFloats.push_back(polygonPoint.GetY());
		});

		std::vector<float> resultPoints;
		psimpl::simplify_opheim<2>(polygonPointsAsFloats.begin(), polygonPointsAsFloats.end(), 10.0f, 20.0f, std::back_inserter(resultPoints));

		std::vector<Vector> outputPolygonPoints;
		for (int i = 0; i < resultPoints.size() - 1; i+=2) {
			outputPolygonPoints.push_back(Vector(resultPoints[i], resultPoints[i + 1]));
		}
		return reducePolygon ? outputPolygonPoints : polygonPoints;
		*/
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	b2Vec2 BitmapToPolygonHelper::GetStartingPixelPosition(BITMAP *bitmapToScan) {
		for (int x = bitmapToScan->w - 1; x >= 0; --x) {
			for (int y = 0; y < bitmapToScan->h; ++y) {
				if (getpixel(bitmapToScan, x, y) != g_MaskColor) {
					return b2Vec2(static_cast<float>(x), static_cast<float>(y));
				}
			}
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	b2Vec2 BitmapToPolygonHelper::GetNextVertexPosition(BITMAP *bitmapToScan, const b2Vec2 &currentPoint, const b2Vec2 &previousPoint) {
		int offset = 0;
		b2Vec2 nextVertexPosition;

		for (int i = 0; i < c_BitmapDirectionUnitVectors.size(); ++i) {
			nextVertexPosition = currentPoint + c_BitmapDirectionUnitVectors[i];
			if (std::floorf(nextVertexPosition.x) == std::floorf(previousPoint.x) && std::floorf(nextVertexPosition.y) == std::floorf(previousPoint.y)) {
				offset = i + 1;
				break;
			}
		}

		for (int i = 0; i < c_BitmapDirectionUnitVectors.size(); ++i) {
			nextVertexPosition = currentPoint + c_BitmapDirectionUnitVectors[(i + offset) % c_BitmapDirectionUnitVectors.size()];
			if (nextVertexPosition.x >= 0 && nextVertexPosition.x < static_cast<float>(bitmapToScan->w) && nextVertexPosition.y >= 0 && nextVertexPosition.y < static_cast<float>(bitmapToScan->h) && getpixel(bitmapToScan, static_cast<int>(std::floorf(nextVertexPosition.x)), static_cast<int>(std::floorf(nextVertexPosition.y))) != g_MaskColor) {
				return nextVertexPosition;
			}
		}
		RTEAbort("Tried to convert sprite to polygon but it's less than 3 pixels large!");
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	std::vector<b2Vec2> BitmapToPolygonHelper::GetReducedPolygon(const std::vector<b2Vec2> &polygonVertices) {
		std::vector<b2Vec2> result;
		result.reserve(polygonVertices.size());

		b2Vec2 checkStartPosition;
		b2Vec2 checkMidPosition = polygonVertices[polygonVertices.size() - 2];
		b2Vec2 checkEndPosition = polygonVertices[polygonVertices.size() - 1];
		for (int i = 0; i < polygonVertices.size(); ++i) {
			checkStartPosition = polygonVertices[i % (polygonVertices.size() - 1)];
			//if ((checkMidPosition - checkStartPosition).GetNormalized().GetFloored() != (checkEndPosition - checkStartPosition).GetNormalized().GetFloored()) {
			if (!IsInline(checkStartPosition, checkMidPosition, checkEndPosition)) {
				result.push_back(checkMidPosition);
				checkEndPosition = checkMidPosition;
			}
			checkMidPosition = checkStartPosition;
		}
		RTEAssert(result.size() > 0, "Tried to reduce bad polygon when converting sprite to polygon!");

		return result;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool BitmapToPolygonHelper::IsInline(const b2Vec2 &checkStartPosition, const b2Vec2 &checkMidPosition, const b2Vec2 &checkEndPosition) {
		//Vector midToStart = (checkMidPosition - checkStartPosition).GetFloored();
		//Vector endToMid = (checkEndPosition - checkMidPosition).GetFloored();
		//return (midToStart.YIsZero() && endToMid.YIsZero()) || (midToStart.GetNormalized().GetFloored() == endToMid.GetNormalized().GetFloored());


		int slope1 = (checkEndPosition.y - checkMidPosition.y);
		int slope2 = (checkMidPosition.y - checkStartPosition.y);
		return 0 == slope1 && 0 == slope2 ||
		   std::floorf((checkEndPosition.x - checkMidPosition.x) / slope1) == ((checkMidPosition.x - checkStartPosition.x) / slope2);
	}
}