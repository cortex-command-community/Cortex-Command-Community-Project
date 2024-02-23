#pragma once

#include "Serializable.h"
#include "MovableObject.h"

namespace RTE {

	/// Something to bundle the properties of an emission together.
	class Emission : public Entity {
		friend class AEmitter;
		friend class PEmitter;

	public:
		// Concrete allocation and cloning definitions
		EntityAllocation(Emission);
		SerializableOverrideMethods;
		ClassInfoGetters;

		/// Constructor method used to instantiate a Emission object in system
		/// memory. Create() should be called before using the object.
		Emission();

		/// Creates a Emission to be identical to another, by deep copy.
		/// @param reference A reference to the Emission to deep copy.
		/// @return An error return value signaling sucess or any particular failure.
		/// Anything below 0 is an error signal.
		int Create(const Emission& reference);

		/// Resets the entire Serializable, including its inherited members, to their
		/// default settings or values.
		void Reset() override;

		/// Gets the reference particle to be emitted. Owenership is NOT transferred!
		/// @return A pointer to the particle to be emitted. Not transferred!
		const MovableObject* GetEmissionParticlePreset();

		/// Gets the rate at which these emissions are made, in particles per minute.
		/// @return The emission rate in PPM.
		float GetRate() const;

		/// Sets the rate at which these emissions are made, in particles per minute.
		/// @param newPPM The emission rate in PPM.
		void SetRate(float newPPM);

		/// Gets the number of extra particles that are bursted at the beginning of
		/// emission.
		/// @return The burst size.
		int GetBurstSize() const;

		/// Sets the number of extra particles that are bursted at the beginning of
		/// emission.
		/// @param newSize The burst size.
		void SetBurstSize(int newSize);

		/// Gets the angle spread of velocity of the emitted MO's to each side of
		/// the m_EmitAngle angle. in radians. PI/2 would mean that MO's fly out to
		/// one side only, with the m_Rotation defining the middle of that half circle.
		/// @return The emission spread in radians.
		float GetSpread() const;

		/// Sets the angle spread of velocity of the emitted MO's to each side of
		/// the m_EmitAngle angle. in radians. PI/2 would mean that MO's fly out to
		/// one side only, with the m_Rotation defining the middle of that half circle.
		/// @param newSpread The emission spread in radians.
		void SetSpread(float newSpread);

		/// Gets the specified minimum velocity an emitted MO can have when emitted.
		/// @return The min emission velocity in m/s.
		float GetMinVelocity() const;

		/// Sets the specified minimum velocity an emitted MO can have when emitted.
		/// @param newVel The min emission velocity in m/s.
		void SetMinVelocity(float newVel);

		/// Gets the specified maximum velocity an emitted MO can have when emitted.
		/// @return The max emission velocity in m/s.
		float GetMaxVelocity() const;

		/// Gets the specified maximum velocity an emitted MO can have when emitted.
		/// @param newVel The max emission velocity in m/s.
		void SetMaxVelocity(float newVel);

		/// Gets the specified variation in lifetime of the emitted particles.
		/// @return The life variation rationally expressed.. 0.1 = up to 10% varitaion.
		float GetLifeVariation() const;

		/// Sets the specified variation in lifetime of the emitted particles.
		/// @param newVariation The life variation rationally expressed.. 0.1 = up to 10% varitaion.
		void SetLifeVariation(float newVariation);

		/// Indicates whether this emission is supposed to push its emitter back
		/// because of recoil.
		/// @return Whether recoil pushing is enabled or not for this emitter.
		bool PushesEmitter() const;

		/// Sets whether this emission is supposed to push its emitter back
		/// because of recoil.
		/// @param newValue Whether recoil pushing is enabled or not for this emitter.
		void SetPushesEmitter(bool newValue);

		/// Shows whether this should be emitting now or not, based on what its
		/// start and end timers are set to.
		/// @return Whether this should be emitting right now.
		bool IsEmissionTime();

		/// Resets the emission timers so they start counting time as to wheter
		/// emissions are clearer.
		void ResetEmissionTimers();

		/// How much of the root parent's velocity this emission inherit
		/// @return The proportion of the velocity inherited. 0.1 = 10% inheritance.
		float InheritsVelocity();

		/// Gets offset of the emission point from Emitter's sprite center, which gets rotated with owner Emitter
		/// @return Returns emission offset.
		Vector GetOffset() const;

		/// Sets offset of the emission point from Emitter's sprite center, which gets rotated with owner Emitter
		/// @param offset New offset value.
		void SetOffset(Vector offset);

	protected:
		// Member variables
		static Entity::ClassInfo m_sClass;

		// The pointer to the preset instance, that copies of which will be emitted
		const MovableObject* m_pEmission;
		// Emission rate in Particles Per Minute
		float m_PPM;
		// The number of particles in the first initial burst of emissions
		// that this AEmitter will generate upon emitting. 0 means none (duh).
		int m_BurstSize;
		// The accumulator for decoupling emission rate from the physics update rate.
		double m_Accumulator;
		// The angle spread of velocity of the emitted MO's to each
		// side of the m_EmitAngle angle. in radians.
		// PI/2 would mean that MO's fly out to one side only, with the
		// m_Rotation defining the middle of that half circle.
		float m_Spread;
		// The minimum velocity an emitted MO can have when emitted
		float m_MinVelocity;
		// The maximum velocity an emitted MO can have when emitted
		float m_MaxVelocity;
		// The variation in life time of each emitted aprticle, in percentage of the existing life time of the partilcle
		float m_LifeVariation;
		// Whether these emissions push the emitter around with recoil or not.
		bool m_PushesEmitter;
		// How much of the parents velocity this emission inherits
		float m_InheritsVel;
		// Timers for measuring when to start and stop this emission the actual times are the set time limits of these
		Timer m_StartTimer;
		Timer m_StopTimer;
		// Offset of the emission point from Emitter's sprite center, which gets rotated with owner Emitter
		Vector m_Offset;

	private:
		/// Clears all the member variables of this Emission, effectively
		/// resetting the members of this abstraction level only.
		void Clear();
	};

} // namespace RTE
