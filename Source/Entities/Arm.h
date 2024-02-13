#pragma once

#include "Attachable.h"

#include <queue>

namespace RTE {

	class HeldDevice;

	/// A detachable arm that can hold HeldDevices.
	class Arm : public Attachable {

	public:
		EntityAllocation(Arm);
		SerializableOverrideMethods;
		ClassInfoGetters;

#pragma region Creation
		/// Constructor method used to instantiate an Arm object in system memory. Create() should be called before using the object.
		Arm();

		/// Makes the Arm object ready for use.
		/// @return An error return value signaling success or any particular failure. Anything below 0 is an error signal.
		int Create() override;

		/// Creates an Arm to be identical to another, by deep copy.
		/// @param reference A reference to the Arm to deep copy.
		/// @return An error return value signaling success or any particular failure. Anything below 0 is an error signal.
		int Create(const Arm& reference);
#pragma endregion

#pragma region Destruction
		/// Destructor method used to clean up an Arm object before deletion from system memory.
		~Arm() override;

		/// Destroys and resets (through Clear()) the Arm object.
		/// @param notInherited Whether to only destroy the members defined in this derived class, or to destroy all inherited members also.
		void Destroy(bool notInherited = false) override {
			if (!notInherited) {
				Attachable::Destroy();
			}
			Clear();
		}

		/// Resets the entire Arm, including its inherited members, to their default settings or values.
		void Reset() override {
			Clear();
			Attachable::Reset();
		}
#pragma endregion

#pragma region Getters and Setters
		/// Gets the max length of this Arm when fully extended, i.e. the farthest possible length from its joint position to the hand.
		/// @return The max length of this Arm.
		float GetMaxLength() const { return m_MaxLength; }

		/// Gets the move speed of this Arm, where 1.0 is instant and 0.0 is none.
		/// @return The move speed of this Arm.
		float GetMoveSpeed() const { return m_MoveSpeed; }

		/// Sets the move speed of this Arm, where 1.0 is instant and 0.0 is none.
		/// @return The new move speed of this Arm.
		void SetMoveSpeed(float newMoveSpeed) { m_MoveSpeed = newMoveSpeed; }

		/// Gets the default idle offset of this Arm's hand, i.e. the default offset from the joint position that this Arm will try to move to when not moving towards a position.
		/// @return The idle offset of this Arm's hand.
		Vector GetHandIdleOffset() const { return m_HandIdleOffset; }

		/// Sets the default idle offset of this Arm's hand, i.e. the default offset from the joint position that this Arm will try to move to when not moving towards a position.
		/// @param newDefaultIdleOffset The new idle offset of this Arm's hand.
		void SetHandIdleOffset(const Vector& newDefaultIdleOffset) { m_HandIdleOffset = newDefaultIdleOffset; }

		/// Gets the rotation that is being applied to this Arm's hand, if it's using an idle offset.
		/// @return The idle rotation of this Arm's hand.
		float GetHandIdleRotation() const { return m_HandIdleRotation; }

		/// Sets the rotation that is being applied to this Arm's hand, if it's using an idle offset. Note that this value is reset to 0 every update.
		/// @param newHandIdleRotation The new idle rotation of this Arm's hand.
		void SetHandIdleRotation(float newHandIdleRotation) { m_HandIdleRotation = newHandIdleRotation; }

		/// Gets the current offset of this Arm's hand, i.e. its distance from the joint position.
		/// @return This current offset of this Arm's hand.
		Vector GetHandCurrentOffset() const { return m_HandCurrentOffset; }

		/// Sets the current offset of this Arm's hand, i.e. its distance from the joint position. The value is capped to the max length of the Arm.
		/// @param newHandOffset The new current offset of this Arm's hand.
		// TODO maybe don't want this in favor of SetHandPos?
		void SetHandCurrentOffset(const Vector& newHandOffset) {
			m_HandCurrentOffset = newHandOffset;
			m_HandCurrentOffset.CapMagnitude(m_MaxLength);
		}

		/// Gets the current position of this Arm's hand in absolute Scene coordinates.
		/// @return The current position of this Arm's hand in absolute Scene coordinates.
		Vector GetHandPos() const { return m_JointPos + m_HandCurrentOffset; }

		/// Sets the current position of this Arm's hand to an absolute scene coordinate. If needed, the set position is modified so its distance from the joint position of the Arm is capped to the max length of the Arm.
		/// @param newHandPos The new current position of this Arm's hand as absolute scene coordinate.
		void SetHandPos(const Vector& newHandPos);

		/// Gets the the strength with which this Arm will grip its HeldDevice.
		/// @return The grip strength of this Arm.
		float GetGripStrength() const { return m_GripStrength; }

		/// Sets the strength with which this Arm will grip its HeldDevice.
		/// @param newGripStrength The new grip strength for this Arm to use.
		void SetGripStrength(float newGripStrength) { m_GripStrength = newGripStrength; }

		/// Gets the the strength with which this Arm will throw a ThrownDevice.
		/// @return The throw strength of this Arm.
		float GetThrowStrength() const { return m_ThrowStrength; }

		/// Sets the strength with which this Arm will throw a ThrownDevice.
		/// @param newThrowStrength The new throw strength for this Arm to use.
		void SetThrowStrength(float newThrowStrength) { m_ThrowStrength = newThrowStrength; }
#pragma endregion

#pragma region Hand Animation Handling
		/// Adds a HandTarget position, in absolute scene coordinates, to the queue for the Arm to move its hand towards. Target positions are removed from the queue when they're reached (or as close to reached as is possible).
		/// @param description The description of this HandTarget, for easy identification.
		/// @param handTargetPositionToAdd The position, in absolute scene coordinates, to add the queue of hand targets.
		void AddHandTarget(const std::string& description, const Vector& handTargetPositionToAdd) { AddHandTarget(description, handTargetPositionToAdd, 0); }

		/// Adds a HandTarget position, in absolute scene coordinates, to the queue for the Arm to move its hand towards. Target positions are removed from the queue when they're reached (or as close to reached as is possible).
		/// If the target position is very close to the last element in the queue, or has the same name as it, the last element in the queue is updated to avoid filling the queue with similar values.
		/// @param description The description of this HandTarget, for easy identification.
		/// @param handTargetPositionToAdd The position, in absolute scene coordinates, to add the queue of hand targets.
		/// @param delayAtTarget The amount of time, in MS, that the hand should wait when it reaches the newly added HandTarget.
		void AddHandTarget(const std::string& description, const Vector& handTargetPositionToAdd, float delayAtTarget);

		/// Removes this Arm's next HandTarget, if there is one.
		void RemoveNextHandTarget() {
			if (!m_HandTargets.empty()) {
				m_HandTargets.pop();
				m_HandHasReachedCurrentTarget = false;
			}
		}

		/// Gets whether or not this Arm has any HandTargets.
		/// @return Whether or not this Arm has any HandTargets.
		bool HasAnyHandTargets() const { return !m_HandTargets.empty(); }

		/// Gets the number of HandTargets this Arm has.
		/// @return The number of HandTargets this Arm has.
		int GetNumberOfHandTargets() const { return m_HandTargets.size(); }

		/// Gets the name of this Arm's next HandTarget.
		/// @return The name of this Arm's next HandTarget.
		std::string GetNextHandTargetDescription() const { return m_HandTargets.empty() ? "" : m_HandTargets.front().Description; }

		/// Gets the position, in absolute scene coordinates, of this Arm's next HandTarget.
		/// @return The position of this Arm's next HandTarget.
		Vector GetNextHandTargetPosition() const { return m_HandTargets.empty() ? Vector() : m_HandTargets.front().TargetOffset + m_JointPos; }

		/// Gets whether or not the hand has reached its current target. This is either the front of the HandTarget queue, or the offset it's currently trying to move to to when it has no HandTargets specified.
		/// @return Whether or not the hand has reached its current target.
		bool GetHandHasReachedCurrentTarget() const { return m_HandHasReachedCurrentTarget; }

		/// Empties the queue of HandTargets. With the queue empty, the hand will move to its appropriate idle offset.
		void ClearHandTargets() { m_HandTargets = {}; }
#pragma endregion

#pragma region HeldDevice Management
		/// Gets the HeldDevice currently held by this Arm.
		/// @return The HeldDevice currently held by this Arm. Ownership is NOT transferred.
		HeldDevice* GetHeldDevice() const { return m_HeldDevice; }

		/// Sets the HeldDevice held by this Arm.
		/// @param newHeldDevice The new HeldDevice to be held by this Arm. Ownership IS transferred.
		void SetHeldDevice(HeldDevice* newHeldDevice);

		/// Gets the HeldDevice this Arm is trying to support.
		/// @return The HeldDevice this Arm is trying to support. Ownership is NOT transferred.
		HeldDevice* GetHeldDeviceThisArmIsTryingToSupport() const { return m_HeldDeviceThisArmIsTryingToSupport; }

		/// Sets the HeldDevice being this Arm is trying to support.
		/// @param newHeldDeviceForThisArmToTryToSupport The new HeldDevice this Arm should try to support. Ownership is NOT transferred.
		void SetHeldDeviceThisArmIsTryingToSupport(HeldDevice* newHeldDeviceThisArmShouldTryToSupport) { m_HeldDeviceThisArmIsTryingToSupport = newHeldDeviceThisArmShouldTryToSupport; }

		/// Replaces the HeldDevice currently held by this Arm with a new one, and returns the old one. Ownership IS transferred both ways.
		/// @param newHeldDevice The new HeldDevice to be held by this Arm. Ownership IS transferred.
		/// @return The HeldDevice that was held by this Arm. Ownership IS transferred.
		HeldDevice* SwapHeldDevice(HeldDevice* newHeldDevice);
#pragma endregion

#pragma region Concrete Methods
		/// Draws this Arm's hand's graphical representation to a BITMAP of choice.
		/// @param targetBitmap A pointer to a BITMAP to draw on.
		/// @param targetPos The absolute position of the target bitmap's upper left corner in the Scene.
		/// @param mode Which mode to draw in. See the DrawMode enumeration for available modes.
		void DrawHand(BITMAP* targetBitmap, const Vector& targetPos = Vector(), DrawMode mode = g_DrawColor) const;
#pragma endregion

#pragma region Override Methods
		/// Does stuff that needs to be done after Update().
		void PostTravel() override {
			if (IsAttached()) {
				m_AngularVel = 0;
			}
			MOSRotating::PostTravel();
		}

		/// Updates this Arm. Supposed to be done every frame.
		void Update() override;

		/// Draws this Arm's current graphical representation to a BITMAP of choice.
		/// @param targetBitmap A pointer to a BITMAP to draw on.
		/// @param targetPos The absolute position of the target bitmap's upper left corner in the Scene.
		/// @param mode Which mode to draw in. See the DrawMode enumeration for the modes.
		/// @param onlyPhysical Whether to not draw any extra 'ghost' items of this Arm. In this case, that means the hand sprite.
		void Draw(BITMAP* targetBitmap, const Vector& targetPos = Vector(), DrawMode mode = g_DrawColor, bool onlyPhysical = false) const override;
#pragma endregion

	private:
		/// Struct for storing data about each target in the Arm's queue of HandTargets.
		struct HandTarget {
			/// Constructor method used to instantiate a HandTarget object in system memory.
			HandTarget(const std::string_view& description, const Vector& targetOffset, float delayAtTarget, bool hFlippedWhenTargetWasCreated) :
			    Description(description), TargetOffset(targetOffset), DelayAtTarget(delayAtTarget), HFlippedWhenTargetWasCreated(hFlippedWhenTargetWasCreated) {}

			std::string Description = "";
			Vector TargetOffset;
			float DelayAtTarget = 0;
			bool HFlippedWhenTargetWasCreated = false;
		};

		static Entity::ClassInfo m_sClass; //!< ClassInfo for this class.

		float m_MaxLength; //!< The maximum length of this Arm when fully extended, i.e. the length of the straight Arm sprite.
		float m_MoveSpeed; //!< How quickly this Arm moves between targets. 0.0 means it doesn't move at all, 1.0 means it moves instantly.

		Vector m_HandIdleOffset; //!< The default offset that this Arm's hand should move to when not moving towards anything else, relative to its joint position. Other offsets are used under certain circumstances.
		float m_HandIdleRotation; //!< The rotation to be applied to the idle offset, when it's being used. Resets every update to avoid locking it.

		Vector m_HandCurrentOffset; //!< The current offset of this Arm's hand, relative to its joint position.

		std::queue<HandTarget> m_HandTargets; // A queue of target positions this Arm's hand is reaching towards. If it's empty, the Arm isn't reaching towards anything.
		Timer m_HandMovementDelayTimer; //!< A Timer for making the hand wait at its current HandTarget.
		bool m_HandHasReachedCurrentTarget; //!< A flag for whether or not the hand has reached its current target. The target is either the front of the HandTarget queue, or the appropriate target to move to if the queue is empty.

		ContentFile m_HandSpriteFile; //!< The ContentFile containing this Arm's hand bitmap.
		BITMAP* m_HandSpriteBitmap; //!< An unowned pointer to the Bitmap held by the hand sprite ContentFile.

		float m_GripStrength; //!< The strength with which this Arm will grip its HeldDevice. Effectively supersedes the HeldDevice's JointStrength.
		float m_ThrowStrength; //!< The strength with which this Arm will throw a ThrownDevice. Effectively supersedes the ThrownDevice's ThrowVelocity values.

		HeldDevice* m_HeldDevice; //!< A pointer to the HeldDevice this Arm is currently holding. Owned in the MOSRotating Attachables list, kept here for convenience.
		HeldDevice* m_HeldDeviceThisArmIsTryingToSupport; //!< A pointer to the HeldDevice being supported by this Arm (i.e. this is the background Arm for another HeldDevice).

		/// Gets whether or not the hand is close to the given offset.
		/// @param targetOffset The offset to check for closeness to the hand.
		/// @return Whether or not the hand is close to the given offset.
		bool HandIsCloseToTargetOffset(const Vector& targetOffset) const;

#pragma region Update Breakdown
		/// Updates the current hand offset for this Arm. Should only be called from Update.
		/// If the Arm is attached, the current hand offset is based on the target offset and move speed, and whether the Arm should idle or not, otherwise it puts it in a reasonable position.
		/// @param hasParent Whether or not this Arm has a parent. Passed in for convenient reuse.
		/// @param heldDeviceIsAThrownDevice Whether or not this Arm's HeldDevice is a ThrownDevice. Passed in for convenient reuse.
		void UpdateHandCurrentOffset(bool armHasParent, bool heldDeviceIsAThrownDevice);

		/// To be used in UpdateHandCurrentOffset. Applies any recoil accumulated on the HeldDevice to the passed in target offset, so it can be used to affect the hand's target position.
		/// @param heldDevice The held MO as a HeldDevice, for convenience.
		/// @param targetOffset The target offset to have recoil applied to it.
		void AccountForHeldDeviceRecoil(const HeldDevice* heldDevice, Vector& targetOffset);

		/// To be used in UpdateHandCurrentOffset. Ensures the HeldDevice won't clip through terrain by modifying the passed in target offset.
		/// @param heldDevice The held MO as a HeldDevice, for convenience.
		/// @param targetOffset The target offset to be modified to avoid any terrain clipping.
		void AccountForHeldDeviceTerrainClipping(const HeldDevice* heldDevice, Vector& targetOffset) const;

		/// Updates the frame for this Arm. Should only be called from Update.
		void UpdateArmFrame();
#pragma endregion

		/// Clears all the member variables of this Arm, effectively resetting the members of this abstraction level only.
		void Clear();

		// Disallow the use of some implicit methods.
		Arm(const Arm& reference) = delete;
		Arm& operator=(const Arm& rhs) = delete;
	};
} // namespace RTE
