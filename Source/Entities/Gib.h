#pragma once

#include "Vector.h"

namespace RTE {

	class MovableObject;

	/// Something to bundle the properties of Gib piece together.
	class Gib : public Serializable {
		friend class GibEditor;
		friend struct EntityLuaBindings;

	public:
		SerializableClassNameGetter;
		SerializableOverrideMethods;

		/// Different types of logic for the Gib to use when applying velocity to its GibParticles.
		enum SpreadMode {
			SpreadRandom,
			SpreadEven,
			SpreadSpiral
		};

#pragma region Creation
		/// Constructor method used to instantiate a Gib object in system memory. Create() should be called before using the object.
		Gib();

		/// Creates a Gib to be identical to another, by deep copy.
		/// @param reference A reference to the Gib to deep copy.
		/// @return An error return value signaling success or any particular failure. Anything below 0 is an error signal.
		int Create(const Gib& reference);
#pragma endregion

#pragma region Destruction
		/// Destructor method used to clean up a Gib object before deletion from system memory.
		~Gib() override;

		/// Destroys and resets (through Clear()) the Gib object.
		void Destroy() { Clear(); }
#pragma endregion

#pragma region Getters and Setters
		/// Gets the reference particle to be used as a Gib. Ownership is NOT transferred!
		/// @return A pointer to the particle to be used as a Gib.
		const MovableObject* GetParticlePreset() const { return m_GibParticle; }

		/// Sets the reference particle to be used as a Gib. Ownership is NOT transferred!
		/// @param newParticlePreset A pointer to the new particle to be used as a Gib.
		void SetParticlePreset(const MovableObject* newParticlePreset) { m_GibParticle = newParticlePreset; }

		/// Gets the spawn offset of this Gib from the parent's position.
		/// @return The offset in pixels from the parent's position where this Gib gets spawned.
		Vector GetOffset() const { return m_Offset; }

		/// Gets the number of copies of the GibParticle object that will be spawned in this Gib.
		/// @return The number of copies of the GibParticle object that will be spawned in this Gib.
		unsigned int GetCount() const { return m_Count; }

		/// Gets the angle spread of the spawned GibParticle objects to each side of the parent's angle in radians.
		/// PI/2 would mean that GibParticles fly out to one side only, with the m_Rotation of the parent defining the middle of that half circle.
		/// @return The GibParticle spread in radians.
		float GetSpread() const { return m_Spread; }

		/// Gets the specified minimum velocity a GibParticle object can have when spawned.
		/// @return The minimum velocity a GibParticle can have when spawned in m/s.
		float GetMinVelocity() const { return std::min(m_MinVelocity, m_MaxVelocity); }

		/// Sets the specified minimum velocity a GibParticle object can have when spawned.
		/// @param newMinVelocity The new minimum velocity in m/s.
		void SetMinVelocity(float newMinVelocity) { m_MinVelocity = newMinVelocity; }

		/// Gets the specified maximum velocity a GibParticle object can have when spawned.
		/// @return The maximum velocity a GibParticle can have when spawned in m/s.
		float GetMaxVelocity() const { return std::max(m_MinVelocity, m_MaxVelocity); }

		/// Sets the specified maximum velocity a GibParticle object can have when spawned.
		/// @param newMaxVelocity The new maximum velocity in m/s.
		void SetMaxVelocity(float newMaxVelocity) { m_MaxVelocity = newMaxVelocity; }

		/// Gets the specified variation in Lifetime of the GibParticle objects.
		/// @return The life variation rationally expressed. 0.1 = up to 10% variation.
		float GetLifeVariation() const { return m_LifeVariation; }

		/// Gets how much of the gibbing parent's velocity this Gib's GibParticles should inherit.
		/// @return The proportion of inherited velocity as a scalar from 0 to 1.
		float InheritsVelocity() const { return m_InheritsVel; }

		/// Gets whether this Gib's GibParticles should ignore hits with the team of the gibbing parent.
		/// @return Whether this Gib's GibParticles should ignore hits with the team of the gibbing parent.
		bool IgnoresTeamHits() const { return m_IgnoresTeamHits; }

		/// Gets this Gib's spread mode, which determines how velocity angles are applied to the GibParticles.
		/// @return The spread mode of this Gib.
		SpreadMode GetSpreadMode() const { return m_SpreadMode; }

		/// Sets this Gib's spread mode, which determines how velocity angles are applied to the GibParticles.
		/// @param newSpreadMode The new spread mode of this Gib. See the SpreadMode enumeration.
		void SetSpreadMode(SpreadMode newSpreadMode) { m_SpreadMode = (newSpreadMode < SpreadMode::SpreadRandom || newSpreadMode > SpreadMode::SpreadSpiral) ? SpreadMode::SpreadRandom : newSpreadMode; }
#pragma endregion

	protected:
		const MovableObject* m_GibParticle; //!< The pointer to the preset instance that copies of will be created as this Gib. Not Owned.
		Vector m_Offset; //!< Offset spawn position from owner/parent's position.
		unsigned int m_Count; //!< The number of copies of the GibParticle that will be spawned.
		float m_Spread; //!< The angle spread of the spawned GibParticle objects to each side of the parent's angle in radians.
		float m_MinVelocity; //!< The minimum velocity a GibParticle object can have when spawned.
		float m_MaxVelocity; //!< The maximum velocity a GibParticle object can have when spawned.
		float m_LifeVariation; //!< The per-Gib variation in Lifetime, in percentage of the existing Lifetime of the gib.
		float m_InheritsVel; //!< How much of the exploding parent's velocity this Gib should inherit.
		bool m_IgnoresTeamHits; //!< Whether this Gib should ignore hits with the team of the exploding parent or not.
		SpreadMode m_SpreadMode; //!< Determines what kind of logic is used when applying velocity to the GibParticle objects.

	private:
		static const std::string c_ClassName; //!< A string with the friendly-formatted type name of this object.

		/// Clears all the member variables of this Gib, effectively resetting the members of this abstraction level only.
		void Clear();
	};
} // namespace RTE
