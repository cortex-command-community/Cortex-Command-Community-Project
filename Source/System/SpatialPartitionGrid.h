#pragma once

// TODO Move Team enum into Constants so we can avoid including Activity here.
#include "Activity.h"

#include "Constants.h"

#include "tsl/hopscotch_set.h"

namespace RTE {

	class Box;
	struct IntRect;
	class MovableObject;

	/// A spatial partitioning grid, used to optimize MOID collision checks.
	class SpatialPartitionGrid {

	public:
#pragma region Creation
		/// Constructor method used to instantiate a SpatialPartitionGrid object.
		SpatialPartitionGrid() { Clear(); }

		/// Constructor method used to instantiate a SpatialPartitionGrid object.
		SpatialPartitionGrid(int width, int height, int cellSize) {
			Clear();
			Create(width, height, cellSize);
		}

		/// Makes the SpatialPartitionGrid object ready for use.
		/// @return An error return value signaling success or any particular failure. Anything below 0 is an error signal.
		int Create(int width, int height, int cellSize);

		/// Creates a SpatialPartitionGrid to be identical to another, by deep copy.
		/// @param reference A reference to the SpatialPartitionGrid to deep copy.
		/// @return An error return value signaling success or any particular failure. Anything below 0 is an error signal.
		int Create(const SpatialPartitionGrid& reference);
#pragma endregion

#pragma region Grid Management
		/// Resets the spatial partitioning grid, removing everything from it
		void Reset();

		/// Adds the given MovableObject to this SpatialPartitionGrid.
		/// @param rect A rectangle defining the space the MovableObject takes up.
		/// @param mo The MovableObject to add.
		void Add(const IntRect& rect, const MovableObject& mo);

		/// Gets a vector of pointers to all MovableObjects within the given Box, who aren't of the ignored team.
		/// @param box The Box to get MovableObjects within.
		/// @param ignoreTeam The team to ignore when getting MovableObjects.
		/// @param getsHitByMOsOnly Whether to only include MOs that have GetsHitByMOs enabled, or all MOs.
		/// @return A vector of pointers to all MovableObjects within the given Box, who aren't of the ignored team.
		std::vector<MovableObject*> GetMOsInBox(const Box& box, int ignoreTeam, bool getsHitByMOsOnly) const;

		/// Get a vector of pointers to all the MovableObjects within the specified radius of the given center point, who aren't of the ignored team.
		/// @param center The center point to get MovableObjects around.
		/// @param radius The radius to get MovableObjects within.
		/// @param ignoreTeam The team to ignore when getting MovableObjects.
		/// @param getsHitByMOsOnly Whether to only include MOs that have GetsHitByMOs enabled, or all MOs.
		/// @return A vector of pointers to all the MovableObjects within the specified radius of the given center point, who aren't of the ignored team.
		std::vector<MovableObject*> GetMOsInRadius(const Vector& center, float radius, int ignoreTeam, bool getsHitByMOsOnly) const;

		/// Gets the MOIDs that are potentially overlapping the given X and Y Scene coordinates.
		/// @param x The X coordinate to check.
		/// @param y The Y coordinate to check.
		/// @param ignoreTeam The team to ignore when getting MOIDs.
		/// @param getsHitByMOsOnly Whether to only include MOs that have GetsHitByMOs enabled, or all MOs.
		/// @return A vector of MOIDs that are potentially overlapping the x and y coordinates.
		const std::vector<int>& GetMOIDsAtPosition(int x, int y, int ignoreTeam, bool getsHitByMOsOnly) const;
#pragma endregion

	private:
		int m_Width; //!< The width of the SpatialPartitionGrid, in cells.
		int m_Height; //!< The height of the SpatialPartitionGrid, in cells.
		int m_CellSize; //!< The size of each of the SpatialPartitionGrid's cells, in pixels.

		// We store a list per team, so overlapping Actors don't waste loads of time collision checking against themselves.
		// Note that this is this list of MOIDs that are potentially colliding per team, so the list for team 1 contains the MOIDs for team 2, 3, 4, and no-team, as well as anything for team 1 that doesn't ignore team hits.
		std::array<std::vector<std::vector<int>>, Activity::MaxTeamCount + 1> m_Cells; //!< Array of cells for each team. The outside-vector is the vector of cells for the team, and each inside-vector entry contains all MOIDs in the cell's space that can collide with that team.
		std::array<std::vector<std::vector<int>>, Activity::MaxTeamCount + 1> m_PhysicsCells; //!< Same as m_Cells, but includes only objects that are GetsHitByMOs.

		tsl::hopscotch_set<int> m_UsedCellIds; //!< Set of used cell Ids, maintained to avoid wasting time looping through and clearing unused cells.

		/// Gets the Id of the cell at the given SpatialPartitionGrid coordinates, automatically accounting for wrapping.
		/// @param cellX The x coordinate of the cell to get the Id of.
		/// @param cellY The y coordinate of the cell to get the Id of.
		/// @return The Id of the cell at the given SpatialPartitionGrid coordinates.
		int GetCellIdForCellCoords(int cellX, int cellY) const;

		/// Clears all the member variables of this SpatialPartitionGrid.
		void Clear();

		// Disallow the use of an implicit method.
		SpatialPartitionGrid(const SpatialPartitionGrid& reference) = delete;
	};
} // namespace RTE
