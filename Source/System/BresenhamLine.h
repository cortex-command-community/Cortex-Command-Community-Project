#pragma once

#include <functional>

namespace RTE {

/// Iterator-style Bresenham's line drawing algorithm.
/// Traverses all integer grid positions from (x0,y0) to (x1,y1) and calls
/// the callback for each position. The callback receives the current position
/// and the current dominant-axis step count. The callback can optionally update
/// x and y to modify the internal position for subsequent iterations (useful for
/// scene wrapping).
/// @param x0 Starting X coordinate (will be truncated toward zero).
/// @param y0 Starting Y coordinate (will be truncated toward zero).
/// @param x1 Ending X coordinate (will be truncated toward zero).
/// @param y1 Ending Y coordinate (will be truncated toward zero).
/// @param skip How many points to skip between callback invocations (0 = every point).
/// @param callback Function called for each visited point. Parameters: (x, y, domStep).
///                 x and y are references to the internal position - the callback can
///                 modify them to affect subsequent iterations (e.g., for scene wrapping).
///                 Return true to continue, false to stop.
/// @return The number of points for which the callback was invoked.
size_t TraverseBresenhamLine(int x0, int y0, int x1, int y1, int skip, std::function<bool(int& x, int& y, int domStep)> callback);

}
