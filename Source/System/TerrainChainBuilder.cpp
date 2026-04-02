/**
 * TerrainChainBuilder.cpp
 *
 * Extracts terrain surface contours for Box2D chain shapes.
 */

#include "TerrainChainBuilder.h"
#include "SLTerrain.h"
#include "SceneMan.h"
#include "Box2DManager.h"
#include "Constants.h"

using namespace RTE;

std::vector<b2Vec2> TerrainChainBuilder::ExtractSurface(SLTerrain* terrain, int stepX) {
    if (!terrain) return {};

    int width = terrain->GetBitmap()->w;
    int height = terrain->GetBitmap()->h;
    float ppm = Box2DManager::PPM;

    std::vector<b2Vec2> points;
    points.reserve(width / stepX + 2);

    for (int x = 0; x < width; x += stepX) {
        // Scan from top to find first solid pixel in this column
        int surfaceY = height; // Default: bottom (no surface found)
        for (int y = 0; y < height; y++) {
            unsigned char mat = terrain->GetMaterialPixel(x, y);
            if (mat != g_MaterialAir && mat != 1 /* g_MaterialCavity */) {
                surfaceY = y;
                break;
            }
        }

        // Convert to Box2D coordinates (pixels → meters, Y negated)
        points.push_back({(float)x / ppm, -(float)surfaceY / ppm});
    }

    // Add final column if we didn't land on it
    if ((width - 1) % stepX != 0) {
        int x = width - 1;
        int surfaceY = height;
        for (int y = 0; y < height; y++) {
            unsigned char mat = terrain->GetMaterialPixel(x, y);
            if (mat != g_MaterialAir && mat != 1) {
                surfaceY = y;
                break;
            }
        }
        points.push_back({(float)x / ppm, -(float)surfaceY / ppm});
    }

    return points;
}

std::vector<b2Vec2> TerrainChainBuilder::ExtractSurfaceRegion(SLTerrain* terrain,
                                                               int left, int top, int right, int bottom,
                                                               int stepX) {
    if (!terrain) return {};
    float ppm = Box2DManager::PPM;

    std::vector<b2Vec2> points;
    int regionWidth = right - left;
    points.reserve(regionWidth / stepX + 2);

    for (int x = left; x <= right; x += stepX) {
        int surfaceY = bottom;
        for (int y = top; y < bottom; y++) {
            unsigned char mat = terrain->GetMaterialPixel(x, y);
            if (mat != g_MaterialAir && mat != 1) {
                surfaceY = y;
                break;
            }
        }
        points.push_back({(float)x / ppm, -(float)surfaceY / ppm});
    }

    return points;
}

// Douglas-Peucker line simplification
static float perpendicularDistance(const b2Vec2& point, const b2Vec2& lineStart, const b2Vec2& lineEnd) {
    float dx = lineEnd.x - lineStart.x;
    float dy = lineEnd.y - lineStart.y;
    float mag = sqrtf(dx * dx + dy * dy);
    if (mag < 0.0001f) {
        dx = point.x - lineStart.x;
        dy = point.y - lineStart.y;
        return sqrtf(dx * dx + dy * dy);
    }
    float u = ((point.x - lineStart.x) * dx + (point.y - lineStart.y) * dy) / (mag * mag);
    float ix = lineStart.x + u * dx;
    float iy = lineStart.y + u * dy;
    dx = point.x - ix;
    dy = point.y - iy;
    return sqrtf(dx * dx + dy * dy);
}

static void douglasPeuckerRecurse(const std::vector<b2Vec2>& points, int start, int end,
                                   float epsilon, std::vector<bool>& keep) {
    if (end <= start + 1) return;

    float maxDist = 0.0f;
    int maxIdx = start;
    for (int i = start + 1; i < end; i++) {
        float dist = perpendicularDistance(points[i], points[start], points[end]);
        if (dist > maxDist) {
            maxDist = dist;
            maxIdx = i;
        }
    }

    if (maxDist > epsilon) {
        keep[maxIdx] = true;
        douglasPeuckerRecurse(points, start, maxIdx, epsilon, keep);
        douglasPeuckerRecurse(points, maxIdx, end, epsilon, keep);
    }
}

std::vector<b2Vec2> TerrainChainBuilder::Simplify(const std::vector<b2Vec2>& points, float epsilon) {
    if (points.size() < 3) return points;

    std::vector<bool> keep(points.size(), false);
    keep.front() = true;
    keep.back() = true;

    douglasPeuckerRecurse(points, 0, (int)points.size() - 1, epsilon, keep);

    std::vector<b2Vec2> result;
    for (size_t i = 0; i < points.size(); i++) {
        if (keep[i]) result.push_back(points[i]);
    }
    return result;
}
