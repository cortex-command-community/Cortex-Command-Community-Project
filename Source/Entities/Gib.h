#ifndef _RTEGIB_
#define _RTEGIB_

#include "Vector.h"

namespace RTE {

	class MovableObject;

	/// <summary>
	/// Something to bundle the properties of Gib piece together.
	/// </summary>
	class Gib : public Serializable {
		friend class GibEditor;
		friend struct EntityLuaBindings;

	public:
		SerializableClassNameGetter;
		SerializableOverrideMethods;

		/// <summary>
		/// Different types of logic for the Gib to use when applying velocity to its GibParticles.
		/// </summary>
		enum SpreadMode {
			SpreadRandom,
			SpreadEven,
			SpreadSpiral
		};

#pragma region Creation
		/// <summary>
		/// Constructor method used to instantiate a Gib object in system memory. Create() should be called before using the object.
		/// </summary>
		Gib() { Clear(); }

		/// <summary>
		/// Creates a Gib to be identical to another, by deep copy.
		/// </summary>
		/// <param name="reference">A reference to the Gib to deep copy.</param>
		/// <returns>An error return value signaling success or any particular failure. Anything below 0 is an error signal.</returns>
		int Create(const Gib& reference);
#pragma endregion

#pragma region Destruction
		/// <summary>
		/// Destructor method used to clean up a Gib object before deletion from system memory.
		/// </summary>
		~Gib() override { Destroy(); }

		/// <summary>
		/// Destroys and resets (through Clear()) the Gib object.
		/// </summary>
		void Destroy() { Clear(); }
#pragma endregion

#pragma region Getters and Setters
		/// <summary>
		/// Gets the reference particle to be used as a Gib. Ownership is NOT transferred!
		/// </summary>
		/// <returns>A pointer to the particle to be used as a Gib.</returns>
		const MovableObject* GetParticlePreset() const { return m_GibParticle; }

		/// <summary>
		/// Sets the reference particle to be used as a Gib. Ownership is NOT transferred!
		/// </summary>
		/// <param name="newParticlePreset">A pointer to the new particle to be used as a Gib.</param>
		void SetParticlePreset(const MovableObject* newParticlePreset) { m_GibParticle = newParticlePreset; }

		/// <summary>
		/// Gets the spawn offset of this Gib from the parent's position.
		/// </summary>
		/// <returns>The offset in pixels from the parent's position where this Gib gets spawned.</returns>
		Vector GetOffset() const { return m_Offset; }

		/// <summary>
		/// Gets the number of copies of the GibParticle object that will be spawned in this Gib.
		/// </summary>
		/// <returns>The number of copies of the GibParticle object that will be spawned in this Gib.</returns>
		unsigned int GetCount() const { return m_Count; }

		/// <summary>
		/// Gets the angle spread of the spawned GibParticle objects to each side of the parent's angle in radians.
		/// PI/2 would mean that GibParticles fly out to one side only, with the m_Rotation of the parent defining the middle of that half circle.
		/// </summary>
		/// <returns>The GibParticle spread in radians.</returns>
		float GetSpread() const { return m_Spread; }

		/// <summary>
		/// Gets the specified minimum velocity a GibParticle object can have when spawned.
		/// </summary>
		/// <returns>The minimum velocity a GibParticle can have when spawned in m/s.</returns>
		float GetMinVelocity() const { return std::min(m_MinVelocity, m_MaxVelocity); }

		/// <summary>
		/// Sets the specified minimum velocity a GibParticle object can have when spawned.
		/// </summary>
		/// <param name="newMinVelocity">The new minimum velocity in m/s.</param>
		void SetMinVelocity(float newMinVelocity) { m_MinVelocity = newMinVelocity; }

		/// <summary>
		/// Gets the specified maximum velocity a GibParticle object can have when spawned.
		/// </summary>
		/// <returns>The maximum velocity a GibParticle can have when spawned in m/s.</returns>
		float GetMaxVelocity() const { return std::max(m_MinVelocity, m_MaxVelocity); }

		/// <summary>
		/// Sets the specified maximum velocity a GibParticle object can have when spawned.
		/// </summary>
		/// <param name="newMaxVelocity">The new maximum velocity in m/s.</param>
		void SetMaxVelocity(float newMaxVelocity) { m_MaxVelocity = newMaxVelocity; }

		/// <summary>
		/// Gets the specified variation in Lifetime of the GibParticle objects.
		/// </summary>
		/// <returns>The life variation rationally expressed. 0.1 = up to 10% variation.</returns>
		float GetLifeVariation() const { return m_LifeVariation; }

		/// <summary>
		/// Gets how much of the gibbing parent's velocity this Gib's GibParticles should inherit.
		/// </summary>
		/// <returns>The proportion of inherited velocity as a scalar from 0 to 1.</returns>
		float InheritsVelocity() const { return m_InheritsVel; }

		/// <summary>
		/// Gets whether this Gib's GibParticles should ignore hits with the team of the gibbing parent.
		/// </summary>
		/// <returns>Whether this Gib's GibParticles should ignore hits with the team of the gibbing parent.</returns>
		bool IgnoresTeamHits() const { return m_IgnoresTeamHits; }

		/// <summary>
		/// Gets this Gib's spread mode, which determines how velocity angles are applied to the GibParticles.
		/// </summary>
		/// <returns>The spread mode of this Gib.</returns>
		SpreadMode GetSpreadMode() const { return m_SpreadMode; }

		/// <summary>
		/// Sets this Gib's spread mode, which determines how velocity angles are applied to the GibParticles.
		/// </summary>
		/// <param name="newSpreadMode">The new spread mode of this Gib. See the SpreadMode enumeration.</param>
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

		/// <summary>
		/// Clears all the member variables of this Gib, effectively resetting the members of this abstraction level only.
		/// </summary>
		void Clear();
	};
} // namespace RTE
#endif
