#pragma once

#include "SoundContainer.h"

namespace RTE {

	class MovableObject;

	/// A round containing a number of projectile particles and one shell.
	class Round : public Entity {

	public:
		EntityAllocation(Round);
		SerializableOverrideMethods;
		ClassInfoGetters;

#pragma region Creation
		/// Constructor method used to instantiate a Round object in system memory. Create() should be called before using the object.
		Round();

		/// Makes the Round object ready for use.
		/// @return An error return value signaling success or any particular failure. Anything below 0 is an error signal.
		int Create() override;

		/// Creates a Round to be identical to another, by deep copy.
		/// @param reference A reference to the Round to deep copy.
		/// @return An error return value signaling success or any particular failure. Anything below 0 is an error signal.
		int Create(const Round& reference);
#pragma endregion

#pragma region Destruction
		/// Destructor method used to clean up a Round object before deletion from system memory.
		~Round() override;

		/// Destroys and resets (through Clear()) the Round object.
		/// @param notInherited Whether to only destroy the members defined in this derived class, or to destroy all inherited members also.
		void Destroy(bool notInherited = false) override {
			if (!notInherited) {
				Entity::Destroy();
			}
			Clear();
		}

		/// Resets the entire Round, including its inherited members, to their default settings or values.
		void Reset() override {
			Clear();
			Entity::Reset();
		}
#pragma endregion

#pragma region Getters and Setters
		/// Returns how many particles are contained within this Round, not counting the shell.
		/// @return The number of particles.
		int ParticleCount() const { return m_ParticleCount; }

		/// Returns whether this Round is out of particles or not.
		/// @return Whether this Round is out of particles or not.
		bool IsEmpty() const { return m_ParticleCount <= 0; }

		/// Gets the next particle contained in this Round. Ownership is NOT transferred!
		/// @return A pointer to the next particle, or 0 if this Round is empty.
		const MovableObject* GetNextParticle() const { return (m_ParticleCount > 0) ? m_Particle : 0; }

		/// Gets the next particle contained in this Round, and removes it from the stack. Ownership IS transferred!
		/// @return A pointer to the next particle, or 0 if this Round is empty.
		MovableObject* PopNextParticle();

		/// Gets the velocity at which this round is to be fired.
		/// @return A float with the velocity in m/s.
		float GetFireVel() const { return m_FireVel; }

		/// Gets whether or not this Round should inherit velocity from its firer.
		/// @return Whether or not this Round should inherit velocity from its firer.
		bool GetInheritsFirerVelocity() const { return m_InheritsFirerVelocity; }

		/// Gets the separation of particles in this round.
		/// @return A float with the separation range in pixels.
		float GetSeparation() const { return m_Separation; }

		/// Gets the variation in lifetime of the fired particles in this Round.
		/// @return A float with the life variation scalar.
		float GetLifeVariation() const { return m_LifeVariation; }

		/// Gets the shell casing preset of this Round. Ownership IS NOT transferred!
		/// @return A pointer to the shell casing preset, or 0 if this Round has no shell.
		const MovableObject* GetShell() const { return m_Shell; }

		/// Gets the maximum velocity at which this round's shell is to be ejected.
		/// @return A float with the maximum shell velocity in m/s.
		float GetShellVel() const { return m_ShellVel; }

		/// Shows whether this Round has an extra sound sample to play when fired.
		/// @return Whether the firing Sound of this has been loaded, or the firing Device will make the noise alone.
		bool HasFireSound() const { return m_FireSound.HasAnySounds(); }

		/// Gets the extra firing sound of this Round, which can be played in addition to the weapon's own firing sound. OWNERSHIP IS NOT TRANSFERRED!
		/// @return A sound with the firing sample of this round.
		SoundContainer* GetFireSound() { return &m_FireSound; }
#pragma endregion

#pragma region AI Properties
		/// Returns the lifetime of the projectile used by the AI when executing the shooting scripts.
		/// @return The life time in MS used by the AI.
		unsigned long GetAILifeTime() const { return m_AILifeTime; }

		/// Returns the FireVelocity of the projectile used by the AI when executing the shooting scripts.
		/// @return The FireVelocity in m/s used by the AI.
		int GetAIFireVel() const { return m_AIFireVel; }

		/// Returns the bullet's ability to penetrate material when executing the AI shooting scripts.
		/// @return A value equivalent to Mass * Sharpness * Vel.
		int GetAIPenetration() const { return m_AIPenetration; }
#pragma endregion

	protected:
		static Entity::ClassInfo m_sClass; //!< ClassInfo for this class.

		const MovableObject* m_Particle; //!< Round particle MovableObject preset instance.
		int m_ParticleCount; //!< How many particle copies there are in this Round.
		float m_FireVel; //!< The velocity with which this Round is fired.
		bool m_InheritsFirerVelocity; //!< Whether or not this Round should inherit velocity from its firer.
		float m_Separation; //!< The range of separation between particles in this Round, in pixels.
		float m_LifeVariation; //!< The random variation in life time of each fired particle, in percentage of their life time.

		const MovableObject* m_Shell; //!< Shell particle MovableObject preset instance.
		float m_ShellVel; //!< The maximum velocity with which this Round's shell/casing is launched.

		SoundContainer m_FireSound; //!< The extra firing audio of this Round being fired.

		unsigned long m_AILifeTime; //!< For overriding LifeTime when executing the AI shooting scripts.
		int m_AIFireVel; //!< For overriding FireVel when executing the AI shooting scripts.
		int m_AIPenetration; //!< For overriding the bullets ability to penetrate material when executing the AI shooting scripts.

	private:
		/// Clears all the member variables of this Round, effectively resetting the members of this abstraction level only.
		void Clear();

		// Disallow the use of some implicit methods.
		Round(const Round& reference) = delete;
		Round& operator=(const Round& rhs) = delete;
	};
} // namespace RTE
