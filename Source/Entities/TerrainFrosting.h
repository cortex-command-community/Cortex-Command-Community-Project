#pragma once

#include "Material.h"

namespace RTE {

	class SLTerrain;

	/// A layer of material on top of another material on the terrain.
	class TerrainFrosting : public Serializable {

	public:
		SerializableClassNameGetter
		SerializableOverrideMethods

#pragma region Creation
		/// Constructor method used to instantiate a TerrainFrosting object in system memory and make it ready for use.
		TerrainFrosting() {
			Clear();
		}
#pragma endregion

#pragma region Concrete Methods
		/// Draws the frosting layer to the specified SLTerrain according to the read-in parameters.
		/// @param terrain The SLTerrain to frost. Ownership is NOT transferred!
		void FrostTerrain(SLTerrain* terrain) const;
#pragma endregion

	private:
		static const std::string c_ClassName; //!< A string with the friendly-formatted type name of this object.

		Material m_FrostingMaterial; //!< The Material this frosting is made of.
		Material m_TargetMaterial; //!< The Material this frosting will be placed on top of in the terrain.
		int m_MinThickness; //!< The minimum height above the target Material, in pixels.
		int m_MaxThickness; //!< The maximum height above the target Material, in pixels.
		bool m_InAirOnly; //!< Whether the frosting only appears where there is air (i.e. does not appear where the terrain background layer is showing).

		/// Clears all the member variables of this TerrainFrosting, effectively resetting the members of this abstraction level only.
		void Clear();

		// Disallow the use of some implicit methods.
		TerrainFrosting(const TerrainFrosting& reference) = delete;
		void operator=(const TerrainFrosting& rhs) = delete;
	};
} // namespace RTE
