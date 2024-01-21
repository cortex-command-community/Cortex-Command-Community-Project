#pragma once

#include "HeldDevice.h"

namespace RTE {

	/// A device that is carried and thrown by Actors.
	class ThrownDevice : public HeldDevice {

	public:
		EntityAllocation(ThrownDevice);
		SerializableOverrideMethods;
		ClassInfoGetters;

#pragma region Creation
		/// Constructor method used to instantiate a ThrownDevice object in system memory. Create should be called before using the object.
		ThrownDevice();

		/// Makes the ThrownDevice object ready for use.
		/// @return An error return value signaling success or any particular failure. Anything below 0 is an error signal.
		int Create() override;

		/// Creates a ThrownDevice to be identical to another, by deep copy.
		/// @param reference A reference to the ThrownDevice to deep copy.
		/// @return An error return value signaling success or any particular failure. Anything below 0 is an error signal.
		int Create(const ThrownDevice& reference);
#pragma endregion

#pragma region Destruction
		/// Destructor method used to clean up a ThrownDevice object before deletion from system memory.
		~ThrownDevice() override;

		/// Destroys and resets (through Clear()) the SceneLayer object.
		/// @param notInherited Whether to only destroy the members defined in this derived class, or to destroy all inherited members also.
		void Destroy(bool notInherited = false) override {
			if (!notInherited) {
				HeldDevice::Destroy();
			}
			Clear();
		}

		/// Resets the entire ThrownDevice, including its inherited members, to their default settings or values.
		void Reset() override {
			Clear();
			Attachable::Reset();
		}
#pragma endregion

#pragma region Getters and Setters
		/// Gets the start throw offset of this ThrownDevice's joint relative from the parent Actor's position, if attached.
		/// @return A const reference to the current start throw parent offset.
		Vector GetStartThrowOffset() const { return m_StartThrowOffset; }

		/// Sets the start throw offset for this ThrownDevice.
		/// @param startOffset The new start throw offset.
		void SetStartThrowOffset(Vector startOffset) { m_StartThrowOffset = startOffset; }

		/// Gets the end throw offset of this ThrownDevice's joint relative from the parent Actor's position, if attached.
		/// @return A const reference to the current end throw parent offset.
		Vector GetEndThrowOffset() const { return m_EndThrowOffset; }

		/// Sets the end throw offset for this ThrownDevice.
		/// @param endOffset The new end throw offset.
		void SetEndThrowOffset(Vector endOffset) { m_EndThrowOffset = endOffset; }

		/// Gets the minimum throw velocity of this when thrown.
		/// @return The minimum throw velocity of this, in m/s.
		float GetMinThrowVel() const { return m_MinThrowVel; }

		/// Sets the minimum throw velocity of this when thrown.
		/// @param minThrowVel The minimum throw velocity of this, in m/s.
		void SetMinThrowVel(float minThrowVel) { m_MinThrowVel = minThrowVel; }

		/// Gets the maximum throw velocity of this when thrown.
		/// @return The maximum throw velocity of this, in m/s.
		float GetMaxThrowVel() const { return m_MaxThrowVel; }

		/// Sets the maximum throw velocity of this when thrown.
		/// @param maxThrowVel The maximum throw velocity of this, in m/s.
		void SetMaxThrowVel(float maxThrowVel) { m_MaxThrowVel = maxThrowVel; }

		/// Ugly method to deal with lua AI bullshit, by pulling the max throwvel calculation based on arm strength into here.
		/// If throw velocity is decided by the Arm and not by the ThrownDevice, then the mass of the ThrownDevice and the angular velocity of the root parent Actor will be taken into account.
		/// @return The max throw vel to use.
		float GetCalculatedMaxThrowVelIncludingArmThrowStrength();

		/// If true then the explosive will not activate until it's released.
		/// @return Whether this ThrownDevice is supposed to only activate when it's released.
		bool ActivatesWhenReleased() const { return m_ActivatesWhenReleased; }
#pragma endregion

#pragma region Virtual Override Methods
		/// Resets all the timers used by this (e.g. emitters, etc). This is to prevent backed up emissions from coming out all at once while this has been held dormant in an inventory.
		void ResetAllTimers() override;

		/// Activates this Device as long as it's not set to activate when released or it has no parent.
		void Activate() override;

		/// Does the calculations necessary to detect whether this ThrownDevice is at rest or not. IsAtRest() retrieves the answer.
		void RestDetection() override {
			HeldDevice::RestDetection();
			if (m_Activated) {
				m_RestTimer.Reset();
			}
		}
#pragma endregion

	protected:
		static Entity::ClassInfo m_sClass; //!< ClassInfo for this class.

		std::shared_ptr<SoundContainer> m_ActivationSound; //!< Activation sound.

		Vector m_StartThrowOffset; //!< The position offset at which a throw of this Device begins.
		Vector m_EndThrowOffset; //!< The position offset at which a throw of this Device ends.
		float m_MinThrowVel; //!< The minimum throw velocity this gets when thrown.
		float m_MaxThrowVel; //!< The maximum throw velocity this gets when thrown.
		long m_TriggerDelay; //!< Time in millisecs from the time of being thrown to triggering whatever it is that this ThrownDevice does.
		bool m_ActivatesWhenReleased; //!< Whether this activates when its throw is started, or waits until it is released from the arm that is throwing it.
		const MovableObject* m_StrikerLever; //!< Striker lever particle MovableObject preset instance.

	private:
		/// Clears all the member variables of this ThrownDevice, effectively resetting the members of this abstraction level only.
		void Clear();

		// Disallow the use of some implicit methods.
		ThrownDevice(const ThrownDevice& reference) = delete;
		ThrownDevice& operator=(const ThrownDevice& rhs) = delete;
	};
} // namespace RTE
