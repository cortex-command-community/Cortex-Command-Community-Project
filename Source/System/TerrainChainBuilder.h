/**
 * TerrainChainBuilder.h
 *
 * Extracts terrain surface contours from the material bitmap and converts
 * them into Box2D chain shapes for broad-phase terrain collision.
 *
 * The algorithm scans columns of the terrain bitmap to find the topmost
 * solid pixel in each column, producing a simplified polyline that
 * represents the terrain surface. This is converted to a Box2D chain
 * shape on a static body.
 */

#pragma once

#include "box2d/box2d.h"
#include <vector>

namespace RTE {

class SLTerrain;

class TerrainChainBuilder {
public:
    /// Extract the terrain surface as a series of points.
    /// Scans each column to find the topmost solid pixel.
    /// @param terrain Pointer to the terrain layer.
    /// @param stepX Column step size (higher = fewer points, faster).
    /// @return Vector of b2Vec2 points in Box2D meters (Y negated for Box2D convention).
    static std::vector<b2Vec2> ExtractSurface(SLTerrain* terrain, int stepX = 4);

    /// Simplify a polyline using Douglas-Peucker algorithm.
    /// @param points Input polyline.
    /// @param epsilon Maximum distance from the simplified line (in meters).
    /// @return Simplified polyline.
    static std::vector<b2Vec2> Simplify(const std::vector<b2Vec2>& points, float epsilon = 0.1f);

    /// Extract surface for a specific rectangular region (for dirty updates).
    static std::vector<b2Vec2> ExtractSurfaceRegion(SLTerrain* terrain,
                                                     int left, int top, int right, int bottom,
                                                     int stepX = 4);
};

} // namespace RTE
