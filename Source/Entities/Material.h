#pragma once

#include "Entity.h"
#include "ContentFile.h"
#include "Color.h"

#include <cmath>

namespace RTE {

	/// Represents a material and holds all the relevant data.
	class Material : public Entity {

	public:
		EntityAllocation(Material);
		SerializableOverrideMethods;
		ClassInfoGetters;

#pragma region Creation
		/// Constructor method used to instantiate a Material object in system memory. Create() should be called before using the object.
		Material() { Clear(); }

		/// Copy constructor method used to instantiate a Material object identical to an already existing one.
		/// @param reference A Material object which is passed in by reference.
		Material(const Material& reference) {
			if (this != &reference) {
				Clear();
				Create(reference);
			}
		}

		/// Creates a Material to be identical to another, by deep copy.
		/// @param reference A reference to the Material to deep copy.
		/// @return An error return value signaling success or any particular failure. Anything below 0 is an error signal.
		int Create(const Material& reference);
#pragma endregion

#pragma region Destruction
		/// Resets the entire Material, including its inherited members, to it's default settings or values.
		void Reset() override {
			Clear();
			Entity::Reset();
		}
#pragma endregion

#pragma region Getters and Setters
		/// Gets the foreground texture bitmap of this Material, if any is associated with it.
		/// @return Pointer to the foreground texture bitmap of this Material.
		BITMAP* GetFGTexture() const { return m_TerrainFGTexture; }

		/// Gets the background texture bitmap of this Material, if any is associated with it.
		/// @return Pointer to the background texture bitmap of this Material.
		BITMAP* GetBGTexture() const { return m_TerrainBGTexture; }

		/// Gets the index of this Material in the material palette.
		/// @return The index of this Material in the material palette. 0 - 255.
		unsigned char GetIndex() const { return m_Index; }

		/// Sets the index of this Material in the material palette to the next specified value.
		/// @param newIndex The new index of this Material in the material palette. 0 - 255.
		void SetIndex(unsigned char newIndex) { m_Index = newIndex; }

		/// Gets the drawing priority of this Material. The higher the number, the higher chances that a pixel of this material will be drawn on top of others. Will default to Integrity if no Priority has been defined.
		/// @return The drawing priority of this Material.
		int GetPriority() const { return m_Priority < 0 ? static_cast<int>(std::ceil(m_Integrity)) : m_Priority; }

		/// Gets the amount of times a dislodged pixel of this Material will attempt to relocate to an open position.
		/// @return The amount of attempts at relocating.
		int GetPiling() const { return m_Piling; }

		/// The impulse force that a particle needs to knock loose a terrain pixel of this material. In kg * m/s.
		/// @return The impulse force that a particle needs to knock loose a terrain pixel of this material.
		float GetIntegrity() const { return m_Integrity; }

		/// Gets the scalar value that defines the restitution of this Material. 1.0 = no kinetic energy is lost in a collision, 0.0 = all energy is lost (plastic).
		/// @return A float scalar value that defines the restitution of this Material.
		float GetRestitution() const { return m_Restitution; }

		/// Gets the scalar value that defines the friction of this Material. 1.0 = will snag onto everything, 0.0 = will glide with no friction.
		/// @return A float scalar value that defines the friction of this Material.
		float GetFriction() const { return m_Friction; }

		/// Gets the scalar value that defines the stickiness of this Material. 1.0 = will stick to everything, 0.0 = will never stick to anything.
		/// @return A float scalar value that defines the stickiness of this Material.
		float GetStickiness() const { return m_Stickiness; }

		/// Gets the density of this Material in Kg/L.
		/// @return The density of this Material.
		float GetVolumeDensity() const { return m_VolumeDensity; }

		/// Gets the density of this Material in kg/pixel, usually calculated from the KG per Volume L property.
		/// @return The pixel density of this Material.
		float GetPixelDensity() const { return m_PixelDensity; }

		/// If this material transforms into something else when settling into the terrain, this will return that different material index. If not, it will just return the regular index of this material.
		/// @return The settling material index of this or the regular index.
		unsigned char GetSettleMaterial() const { return (m_SettleMaterialIndex != 0) ? m_SettleMaterialIndex : m_Index; }

		/// Gets the material index to spawn instead of this one for special effects.
		/// @return The material index to spawn instead of this one for special effects. 0 means to spawn the same material as this.
		unsigned char GetSpawnMaterial() const { return m_SpawnMaterialIndex; }

		/// Whether this material is scrap material made from gibs of things that have already been blown apart.
		/// @return Whether this material is scrap material.
		bool IsScrap() const { return m_IsScrap; }

		/// Gets the Color of this Material.
		/// @return The color of this material.
		Color GetColor() const { return m_Color; }

		/// Indicates whether or not to use the Material's own color when a pixel of this Material is knocked loose from the terrain.
		/// @return Whether the Material's color, or the terrain pixel's color should be applied.
		bool UsesOwnColor() const { return m_UseOwnColor; }
#pragma endregion

#pragma region Operator Overloads
		/// An assignment operator for setting one Material equal to another.
		/// @param rhs A Material reference.
		/// @return A reference to the changed Material.
		Material& operator=(const Material& rhs) {
			if (this != &rhs) {
				Destroy();
				Create(rhs);
			}
			return *this;
		}
#pragma endregion

	protected:
		static Entity::ClassInfo m_sClass; //!< ClassInfo for this class.

		unsigned char m_Index; //!< Index of this in the material palette. 0 - 255.
		int m_Priority; //!< The priority that a pixel of this material has to be displayed. The higher the number, the higher chances that a pixel of this material will be drawn on top of others.
		int m_Piling; //! The amount of times a dislodged pixel of this Material will attempt to relocate upwards, when intersecting a terrain pixel of the same Material. TODO: Better property name?

		float m_Integrity; //!< The impulse force that a particle needs to knock loose a terrain pixel of this material. In kg * m/s.
		float m_Restitution; //!< A scalar value that defines the restitution (elasticity). 1.0 = no kinetic energy is lost in a collision, 0.0 = all energy is lost (plastic).
		float m_Friction; //!< A scalar value that defines the friction coefficient. 1.0 = will snag onto everything, 0.0 = will glide with no friction.
		float m_Stickiness; //!< A scalar value that defines the stickiness coefficient (no sticky 0.0 - 1.0 max). Determines the likelihood of something of this material sticking to terrain when a collision occurs.

		float m_VolumeDensity; //!< Density in Kg/L.
		float m_PixelDensity; //!< Density in kg/pixel, usually calculated from the KG per Volume L property.

		// TODO: Implement these properties maybe? They aren't referenced anywhere or do anything as of now.
		float m_GibImpulseLimitPerLiter; //!< How much impulse gib limit of an object increases per liter of this material.
		float m_GibWoundLimitPerLiter; //!< How much wound gib limit of an object increases per liter of this material.

		unsigned char m_SettleMaterialIndex; //!< The material to turn particles of this into when they settle on the terrain. 0 here means to spawn this material.
		unsigned char m_SpawnMaterialIndex; //!< The material to spawn instead of this one for special effects, etc. 0 here means to spawn this material.
		bool m_IsScrap; //!< Whether this material is scrap material made from gibs of things that have already been blown apart.

		Color m_Color; //!< The natural color of this material.
		bool m_UseOwnColor; //!< Whether or not to use the own color when a pixel of this material is knocked loose from the terrain. If 0, then the terrain pixel's color will be applied instead.

		ContentFile m_FGTextureFile; //!< The file pointing to the terrain foreground texture of this Material.
		ContentFile m_BGTextureFile; //!< The file pointing to the terrain background texture of this Material.
		BITMAP* m_TerrainFGTexture; //!< The foreground texture of this Material, used when building an SLTerrain. Not owned.
		BITMAP* m_TerrainBGTexture; //!< The background texture of this Material, used when building an SLTerrain. Not owned.

	private:
		/// Clears all the member variables of this Material, effectively resetting the members of this abstraction level only.
		void Clear();
	};
} // namespace RTE
