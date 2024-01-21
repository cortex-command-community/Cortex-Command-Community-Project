#pragma once

#include "Attachable.h"

namespace RTE {

	class HeldDevice;

	/// A detachable Leg that will be controlled by LimbPaths.
	class Leg : public Attachable {

	public:
		EntityAllocation(Leg);
		SerializableOverrideMethods;
		ClassInfoGetters;

#pragma region Creation
		/// Constructor method used to instantiate a Leg object in system memory. Create() should be called before using the object.
		Leg();

		/// Makes the Leg object ready for use.
		/// @return An error return value signaling success or any particular failure. Anything below 0 is an error signal.
		int Create() override;

		/// Creates a Leg to be identical to another, by deep copy.
		/// @param reference A reference to the Leg to deep copy.
		/// @return An error return value signaling success or any particular failure. Anything below 0 is an error signal.
		int Create(const Leg& reference);
#pragma endregion

#pragma region Destruction
		/// Destructor method used to clean up a Leg object before deletion from system memory.
		~Leg() override;

		/// Destroys and resets (through Clear()) the Leg object.
		/// @param notInherited Whether to only destroy the members defined in this derived class, or to destroy all inherited members also.
		void Destroy(bool notInherited = false) override {
			if (!notInherited) {
				Attachable::Destroy();
			}
			Clear();
		}

		/// Resets the entire Leg, including its inherited members, to their default settings or values.
		void Reset() override {
			Clear();
			Attachable::Reset();
		}
#pragma endregion

#pragma region Getters and Setters
		/// Gets the foot of this Leg.
		/// @return A pointer to foot of this Leg. Ownership is NOT transferred!
		Attachable* GetFoot() const { return m_Foot; }

		/// Sets the foot for this Leg. Ownership IS transferred!
		/// @param newFoot The new foot to use.
		void SetFoot(Attachable* newFoot);

		/// Gets the min length this of Leg, the minimum allowed length from its joint to its ankle's position.
		/// @return The min length, in pixels, of this Leg.
		float GetMinLength() const { return m_MinExtension; }

		/// Gets the max length this Leg, the maximum allowed length from its joint to its ankle's position.
		/// @return The max length, in pixels, of this Leg.
		float GetMaxLength() const { return m_MaxExtension; }

		/// Gets the move speed of this Leg, where 1.0 is instant and 0.0 is no movement.
		/// @return The move speed of this Leg.
		float GetMoveSpeed() const { return m_MoveSpeed; }

		/// Sets the move speed of this Leg, where 1.0 is instant and 0.0 is no movement.
		/// @return The new move speed of this Leg.
		void SetMoveSpeed(float newMoveSpeed) { m_MoveSpeed = newMoveSpeed; }

		/// Sets the position this Leg should move towards, in absolute coordinates.
		/// @param targetPosition The position the Leg should move towards.
		void SetTargetPosition(const Vector& targetPosition) { m_TargetPosition = targetPosition; }

		/// Sets whether this Leg will go into idle offset mode if the target appears to be above the joint of the Leg.
		/// @param idle Whether to enable idling if the target offset is above the joint.
		void EnableIdle(bool idle = true) { m_WillIdle = idle; }
#pragma endregion

#pragma region Concrete Methods
		/// Gets a copy of this Leg's foot AtomGroup to be used as an Actor's FootGroup.
		/// @return A copy of this Leg's foot AtomGroup to be used as an Actor's FootGroup. OWNERSHIP IS TRANSFERRED!
		AtomGroup* GetFootGroupFromFootAtomGroup();
#pragma endregion

#pragma region Override Methods
		/// Updates this Leg. Supposed to be done every frame.
		void Update() override;
#pragma endregion

	protected:
		static Entity::ClassInfo m_sClass; //!< ClassInfo for this class.

		Attachable* m_Foot; //!< Pointer to the foot attachable of this Leg.

		Vector m_ContractedOffset; //!< The offset from the joint where the ankle contracts to in the sprite.
		Vector m_ExtendedOffset; //!< The offset from the joint where the ankle extends to in the sprite.

		float m_MinExtension; //!< Precalculated min extension of the Leg (from the joint) based on the contracted offset.
		float m_MaxExtension; //!< Precalculated max extension of the Leg (from the joint) based on the extended offset.
		float m_NormalizedExtension; //!< Normalized scalar of where the ankle offset's magnitude is between the min and max extensions.

		Vector m_TargetPosition; //!< The absolute position that this Leg's foot is moving towards.
		Vector m_IdleOffset; //!< The target offset from m_Pos that this Leg's foot is moving towards when allowed to idle and the target position is not acceptable.

		Vector m_AnkleOffset; //!< Current offset from the joint to the ankle where the foot should be.

		bool m_WillIdle; //!< Whether the Leg will go to idle position if the target position is above the Leg's joint position.
		float m_MoveSpeed; //!< How fast the Leg moves to a reach target, 0 means it doesn't and 1 means it moves instantly.

	private:
#pragma region Update Breakdown
		/// Updates the current ankle offset for this Leg. Should only be called from Update.
		/// If the Leg is attached, the current ankle offset is based on the target offset and move speed, and whether the Leg should idle or not, otherwise it puts it in a reasonable position.
		void UpdateCurrentAnkleOffset();

		/// Updates the rotation of the Leg. Should only be called from Update.
		/// If the Leg is attached, this applies extra rotation to line up the Leg's sprite with its extension line, otherwise it does nothing.
		void UpdateLegRotation();

		/// Updates the frame and rotation of the Leg's foot Attachable. Should only be called from Update.
		/// If the Leg is attached, the foot's rotation and frame depend on the ankle offset, otherwise the foot's rotation is set to be perpendicular to the Leg's rotation.
		void UpdateFootFrameAndRotation();
#pragma endregion

		/// Clears all the member variables of this Leg, effectively resetting the members of this abstraction level only.
		void Clear();

		// Disallow the use of some implicit methods.
		Leg(const Leg& reference) = delete;
		Leg& operator=(const Leg& rhs) = delete;
	};
} // namespace RTE
