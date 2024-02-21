#pragma once

#include "AEmitter.h"

namespace RTE {

	/// A jetpack MO, which can be used to generate thrust
	class AEJetpack : public AEmitter {
		friend struct EntityLuaBindings;

	public:
		// Concrete allocation and cloning definitions
		EntityAllocation(AEJetpack);
		SerializableOverrideMethods;
		ClassInfoGetters;

		enum JetpackType {
			Standard, // Can be intermittently tapped to produce small amounts of thrust
			JumpPack // Spends all of it's fuel until empty, and cannot fire again until recharged
		};

#pragma region Creation
		/// Constructor method used to instantiate a AEJetpack object in system memory. Create() should be called before using the object.
		AEJetpack();

		/// Makes the AEJetpack object ready for use.
		/// @return An error return value signaling success or any particular failure. Anything below 0 is an error signal.
		int Create() override;

		/// Creates a AEJetpack to be identical to another, by deep copy.
		/// @param reference A reference to the AEJetpack to deep copy.
		/// @return An error return value signaling success or any particular failure. Anything below 0 is an error signal.
		int Create(const AEJetpack& reference);
#pragma endregion

#pragma region Destruction
		/// Destructor method used to clean up a AEJetpack object before deletion from system memory.
		~AEJetpack() override;

		/// Resets the entire AEJetpack, including its inherited members, to their default settings or values.
		void Reset() override;
#pragma endregion

		/// Updates this AEJetpack from our parent actor.
		void UpdateBurstState(Actor& parentActor);

		/// Returns whether or not this jetpack is fully fueled.
		/// @return Whether or not this jetpack is fully fueled.
		bool IsFullyFueled() const;

		/// Returns whether or not this jetpack is out of fuel.
		/// @return Whether or not this jetpack is out of fuel.
		bool IsOutOfFuel() const;

		/// Gets the amount of time this jetpack can fire when filled, in ms.
		/// @return The amount of time this jetpack can fire when it's at max.
		float GetJetTimeTotal() const;

		/// Sets the amount of time this' jetpack can fire when filled, in ms.
		/// @param newValue The amount of time this jetpack can fire when it's at max.
		void SetJetTimeTotal(float newValue);

		/// Gets the amount of time this jetpack can still fire until out, in ms.
		/// @return The amount of time this jetpack can still fire before running out.
		float GetJetTimeLeft() const;

		/// Sets the amount of time this' jetpack can still fire until out, in ms.
		/// @param newValue The amount of time this' jetpack can still fire before running out.
		void SetJetTimeLeft(float newValue);

		/// Gets the ratio of jetpack time that is left.
		/// @return The ratio of jetpack time that is left.
		float GetJetTimeRatio();

		/// Gets the rate at which this AHuman's jetpack is replenished during downtime.
		/// @return The rate at which the jetpack is replenished.
		float GetJetReplenishRate() const;

		/// Sets the rate at which this AHuman's jetpack is replenished during downtime.
		/// @param newValue The rate at which the jetpack is replenished.
		void SetJetReplenishRate(float newValue);

		/// Gets the rate at which this AHuman's jetpack is replenished during downtime.
		/// @return The rate at which the jetpack is replenished.
		float GetMinimumFuelRatio() const;

		/// Sets the rate at which this AHuman's jetpack is replenished during downtime.
		/// @param newValue The rate at which the jetpack is replenished.
		void SetMinimumFuelRatio(float newValue);

		/// Gets the scalar ratio at which this jetpack's thrust angle follows the aim angle of the user.
		/// @return The ratio at which this jetpack follows the aim angle of the user.
		float GetJetAngleRange() const;

		/// Sets the scalar ratio at which this jetpack's thrust angle follows the aim angle of the user.
		/// @param newValue The ratio at which this jetpack follows the aim angle of the user.
		void SetJetAngleRange(float newValue);

		/// Gets the type of this jetpack.
		/// @return The type of this jetpack.
		JetpackType GetJetpackType() const;

		/// Sets the type of this jetpack.
		/// @param newType The new type of this jetpack.
		void SetJetpackType(JetpackType newType);

		/// Returns whether the angle of this jetpack can adjust while firing, or if it can only be aimed while off.
		/// @return Whether the angle of this jetpack can adjust while firing.
		bool GetCanAdjustAngleWhileFiring() const;

		/// Sets whether the angle of this can adjust while firing, or if it can only be aimed while off.
		/// @param newValue The new value for whether the angle of this jetpack can adjust while firing.
		void SetCanAdjustAngleWhileFiring(bool newValue);

		/// Returns whether this jetpack adjusts it's throttle to balance for extra weight.
		/// @return Whether this jetpack adjusts it's throttle to balance for extra weight.
		bool GetAdjustsThrottleForWeight() const;

		/// Sets whether this jetpack adjusts it's throttle to balance for extra weight.
		/// @param newValue The new value for whether this jetpack adjusts it's throttle to balance for extra weight.
		void SetAdjustsThrottleForWeight(bool newValue);

	protected:
		static Entity::ClassInfo m_sClass;

		JetpackType m_JetpackType; //!< The type of jetpack
		float m_JetTimeTotal; //!< The max total time, in ms, that the jetpack can be used without pause
		float m_JetTimeLeft; //!< How much time left the jetpack can go, in ms
		float m_JetThrustBonusMultiplier; //!< A multiplier bonus to our produced thrust, which doesn't cost extra fuel. Used for AI buffs.
		float m_JetReplenishRate; //!< A multiplier affecting how fast the jetpack fuel will replenish when not in use. 1 means that jet time replenishes at 2x speed in relation to depletion.
		float m_MinimumFuelRatio; // Minimum ratio of current fuel to max fuel to be able to initiate the jetpack.
		float m_JetAngleRange; //!< Ratio at which the jetpack angle follows aim angle
		bool m_CanAdjustAngleWhileFiring; //!< Whether or not the angle of the thrust can change while firing, or if it can only be adjusted while the jetpack is off
		bool m_AdjustsThrottleForWeight; //!< Whether or not the jetpack throttle auto-adjusts for weight, at the cost of fuel usage.

	private:
		/// The logic to run when bursting.
		/// @param parentActor The parent actor using this jetpack.
		/// @param fuelUseMultiplier The multiplier to fuel usage rate.
		void Burst(Actor& parentActor, float fuelUseMultiplier);

		/// The logic to run when thrusting.
		/// @param parentActor The parent actor using this jetpack.
		/// @param fuelUseMultiplier The multiplier to fuel usage rate.
		void Thrust(Actor& parentActor, float fuelUseMultiplier);

		/// The logic to run when recharging.
		/// @param parentActor The parent actor using this jetpack.
		void Recharge(Actor& parentActor);

		/// Clears all the member variables of this AEJetpack, effectively resetting the members of this abstraction level only.
		void Clear();

		// Disallow the use of some implicit methods.
		AEJetpack(const AEJetpack& reference) = delete;
		AEJetpack& operator=(const AEJetpack& rhs) = delete;
	};
} // namespace RTE
