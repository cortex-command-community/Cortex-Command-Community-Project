#pragma once

#include "SceneObject.h"
#include "ContentFile.h"

namespace RTE {

	class SLTerrain;

	/// A feature of the terrain, which includes foreground color layer, material layer and optional background layer.
	class TerrainObject : public SceneObject {

	public:
		EntityAllocation(TerrainObject);
		SerializableOverrideMethods;
		ClassInfoGetters;

#pragma region Creation
		/// Constructor method used to instantiate a TerrainObject object in system memory. Create() should be called before using the object.
		TerrainObject();

		/// Makes the TerrainObject object ready for use.
		/// @return An error return value signaling success or any particular failure. Anything below 0 is an error signal.
		int Create() override;

		/// Creates a TerrainObject to be identical to another, by deep copy.
		/// @param reference A reference to the TerrainObject to deep copy.
		/// @return An error return value signaling success or any particular failure. Anything below 0 is an error signal.
		int Create(const TerrainObject& reference);
#pragma endregion

#pragma region Destruction
		/// Destructor method used to clean up a TerrainObject object before deletion from system memory.
		~TerrainObject() override;

		/// Destroys and resets (through Clear()) the TerrainObject object.
		/// @param notInherited Whether to only destroy the members defined in this derived class, or to destroy all inherited members also.
		void Destroy(bool notInherited = false) override {
			if (!notInherited) {
				SceneObject::Destroy();
			}
			Clear();
		}
#pragma endregion

#pragma region Getters and Setters
		/// Returns whether this TerrainObject has any foreground color data.
		/// @return Whether this TerrainOBject has any foreground color data.
		bool HasFGColorBitmap() const { return m_FGColorBitmap != nullptr; }

		/// Gets the BITMAP that this TerrainObject uses for its foreground color representation.
		/// @return A pointer to the foreground color BITMAP object. Ownership is NOT transferred!
		BITMAP* GetFGColorBitmap() const { return m_FGColorBitmap; }

		/// Returns whether this TerrainObject has any background color data.
		/// @return Whether this TerrainOBject has any background color data.
		bool HasBGColorBitmap() const { return m_BGColorBitmap != nullptr; }

		/// Gets the BITMAP that this TerrainObject uses for its background color representation, if any.
		/// @return A pointer to the background color BITMAP object. This may be nullptr if there is no BG bitmap. Ownership is NOT transferred!
		BITMAP* GetBGColorBitmap() const { return m_BGColorBitmap; }

		/// Returns whether this TerrainObject has any material data.
		/// @return Whether this TerrainOBject has any material data.
		bool HasMaterialBitmap() const { return m_MaterialBitmap != nullptr; }

		/// Gets the BITMAP that this TerrainObject uses for its material representation, if any.
		/// @return A pointer to the material BITMAP object. Ownership is NOT transferred!
		BITMAP* GetMaterialBitmap() const { return m_MaterialBitmap; }

		/// Gets the offset from the position to the upper left corner of this TerrainObject's BITMAPs.
		/// @return A Vector describing the bitmap offset, in pixels.
		const Vector& GetBitmapOffset() const { return m_BitmapOffset; }

		/// Gets the width of the widest BITMAP of this TerrainObject's layers.
		/// @return The width of this TerrainObject.
		int GetBitmapWidth() const { return std::max({m_MaterialBitmap ? m_MaterialBitmap->w : 0, m_BGColorBitmap ? m_BGColorBitmap->w : 0, m_FGColorBitmap ? m_FGColorBitmap->w : 0}); }

		/// Gets the height of the highest BITMAP of this TerrainObject's layers.
		/// @return The height of this TerrainObject.
		int GetBitmapHeight() const { return std::max({m_MaterialBitmap ? m_MaterialBitmap->h : 0, m_BGColorBitmap ? m_BGColorBitmap->h : 0, m_FGColorBitmap ? m_FGColorBitmap->h : 0}); }

		/// Gets the list of child objects that should be placed when this TerrainObject is placed.
		/// @return A reference to the list of child objects. Ownership of the list is NOT transferred!
		const std::vector<SceneObject::SOPlacer>& GetChildObjects() const { return m_ChildObjects; }

		/// Gets a BITMAP showing a good identifiable icon of this, for use in GUI lists.
		/// @return A good identifiable graphical representation of this in a BITMAP, if available. If not, nullptr is returned. Ownership is NOT transferred!
		BITMAP* GetGraphicalIcon() const override;

		/// Sets which team this TerrainObject belongs to.
		/// @param team The assigned team number.
		void SetTeam(int team) override;
#pragma endregion

#pragma region Concrete Methods
		/// Places this TerrainObject and all its children on the specified SLTerrain.
		/// @param terrain The SLTerrain to place this TerrainObject on. Ownership is NOT transferred!
		/// @return Whether the object was successfully placed on the terrain.
		bool PlaceOnTerrain(SLTerrain* terrain);
#pragma endregion

#pragma region Virtual Override Methods
		/// Indicates whether this TerrainObject's current graphical representation overlaps a point in absolute scene coordinates.
		/// @param scenePoint The point in absolute scene coordinates.
		/// @return Whether this' graphical rep overlaps the scene point.
		bool IsOnScenePoint(Vector& scenePoint) const override;

		/// Draws this TerrainObject's current graphical representation to a BITMAP of choice.
		/// @param targetBitmap A pointer to a BITMAP to draw on.
		/// @param targetPos The absolute position of the target bitmap's upper left corner in the Scene.
		/// @param drawMode In which mode to draw in. See the DrawMode enumeration for the modes.
		/// @param onlyPhysical Whether to not draw any extra 'ghost' items of this TerrainObject, like indicator arrows or hovering HUD text and so on.
		void Draw(BITMAP* targetBitmap, const Vector& targetPos = Vector(), DrawMode drawMode = DrawMode::g_DrawColor, bool onlyPhysical = false) const override;
#pragma endregion

	protected:
		static Entity::ClassInfo m_sClass; //!< ClassInfo for this class.

		ContentFile m_FGColorFile; //!< ContentFile containing the path to this TerrainObject's foreground color layer representation.
		BITMAP* m_FGColorBitmap; //!< Foreground color BITMAP of this TerrainObject.

		ContentFile m_BGColorFile; //!< ContentFile containing the path to this TerrainObject's background color layer representation.
		BITMAP* m_BGColorBitmap; //!< Background color BITMAP of this TerrainObject.

		ContentFile m_MaterialFile; //!< ContentFile containing the path to this TerrainObject's Material layer representation.
		BITMAP* m_MaterialBitmap; //!< Material BITMAP of this TerrainObject.

		Vector m_BitmapOffset; //!< Offset from the position of this to the top left corner of the bitmap. The inversion of this should point to a corner or pattern in the bitmaps which will snap well with a 24px grid.
		bool m_OffsetDefined; //!< Whether the offset has been defined and shouldn't be automatically set.

		std::vector<SceneObject::SOPlacer> m_ChildObjects; //!< The objects that are placed along with this TerrainObject on the Scene.

	private:
		/// Draws this TerrainObject's graphical and material representations to the specified SLTerrain's respective layers.
		/// @param terrain The SLTerrain to draw this TerrainObject to. Ownership is NOT transferred!
		void DrawToTerrain(SLTerrain* terrain);

		/// Clears all the member variables of this TerrainObject, effectively resetting the members of this abstraction level only.
		void Clear();

		// Disallow the use of some implicit methods.
		TerrainObject(const TerrainObject& reference) = delete;
		void operator=(const TerrainObject& rhs) = delete;
	};
} // namespace RTE
