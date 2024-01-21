#pragma once

#include "Attachable.h"

namespace RTE {

	class HeldDevice;

	/// An Attachable Turret pod that can hold HeldDevices.
	class Turret : public Attachable {

	public:
		EntityAllocation(Turret);
		SerializableOverrideMethods;
		ClassInfoGetters;

#pragma region Creation
		/// Constructor method used to instantiate a Turret object in system memory. Create() should be called before using the object.
		Turret();

		/// Creates a Turret to be identical to another, by deep copy.
		/// @param reference A reference to the Turret to deep copy.
		/// @return An error return value signaling success or any particular failure. Anything below 0 is an error signal.
		int Create(const Turret& reference);
#pragma endregion

#pragma region Destruction
		/// Destructor method used to clean up a Turret object before deletion from system memory.
		~Turret() override;

		/// Destroys and resets (through Clear()) the Turret object.
		/// @param notInherited Whether to only destroy the members defined in this derived class, or to destroy all inherited members also.
		void Destroy(bool notInherited = false) override;

		/// Resets the entire Turret, including its inherited members, to their default settings or values.
		void Reset() override {
			Clear();
			Attachable::Reset();
		}
#pragma endregion

#pragma region Getters and Setters
		/// Indicates whether a HeldDevice is mounted or not.
		/// @return Whether or not a HeldDevice is mounted on this Turret.
		bool HasMountedDevice() const { return !m_MountedDevices.empty(); }

		/// Gets the first mounted HeldDevice of this Turret, mostly here for Lua convenience.
		/// @return A pointer to mounted HeldDevice of this Turret. Ownership is NOT transferred!
		HeldDevice* GetFirstMountedDevice() const { return m_MountedDevices[0]; }

		/// Sets the first mounted HeldDevice for this Turret, mostly here for Lua convenience. Ownership IS transferred!
		/// The current first mounted HeldDevice (if there is one) will be dropped and added to MovableMan.
		/// @param newMountedDevice The new HeldDevice to use.
		void SetFirstMountedDevice(HeldDevice* newMountedDevice);

		/// Gets the vector of mounted HeldDevices for this Turret.
		/// @return The vector of mounted HeldDevices for this Turret.
		const std::vector<HeldDevice*>& GetMountedDevices() const { return m_MountedDevices; }

		/// Adds a HeldDevice to be mounted on this Turret. Ownership IS transferred!
		/// Will not remove any other HeldDevices mounted on this Turret.
		/// @param newMountedDevice The new HeldDevice to be mounted on this Turret.
		void AddMountedDevice(HeldDevice* newMountedDevice);

		/// Gets the current rotational offset of the mounted HeldDevice from the rest of the Turret.
		/// @return The current rotational offset of the mounted HeldDevice from the rest of the Turret.
		float GetMountedDeviceRotationOffset() const { return m_MountedDeviceRotationOffset; }

		/// Sets the current rotational offset of the mounted HeldDevice from the rest of the Turret.
		/// @param newOffsetAngle The new offset angle in radians, relative from the rest of the Turret.
		void SetMountedDeviceRotationOffset(float newOffsetAngle) { m_MountedDeviceRotationOffset = newOffsetAngle; }
#pragma endregion

#pragma region Override Methods
		/// Updates this MovableObject. Supposed to be done every frame.
		void Update() override;

		/// Draws this Turret's current graphical representation to a BITMAP of choice.
		/// @param pTargetBitmap A pointer to a BITMAP to draw on.
		/// @param targetPos The absolute position of the target bitmap's upper left corner in the Scene.
		/// @param mode In which mode to draw in. See the DrawMode enumeration for the modes.
		/// @param onlyPhysical Whether to not draw any extra 'ghost' items of this MovableObject, indicator arrows or hovering HUD text and so on.
		void Draw(BITMAP* pTargetBitmap, const Vector& targetPos = Vector(), DrawMode mode = g_DrawColor, bool onlyPhysical = false) const override;
#pragma endregion

	protected:
		/// Sets this Attachable's parent MOSRotating, and also sets its Team based on its parent and, if the Attachable is set to collide, adds/removes Atoms to its new/old parent.
		/// Additionally, deactivates all MountedDevices.
		/// @param newParent A pointer to the MOSRotating to set as the new parent. Ownership is NOT transferred!
		void SetParent(MOSRotating* newParent) override;

		static Entity::ClassInfo m_sClass; //!< ClassInfo for this class.

	private:
		// TODO I think things would be cleaner if this (and all hardcoded attachable pointers) used weak_ptrs. It would solve some weird ownership stuff, particularly with this. However, for that to be possible, m_Attachables has to be shared_ptrs though.
		std::vector<HeldDevice*> m_MountedDevices; //!< Vector of pointers to the mounted HeldDevices of this Turret, if any. Owned here.
		float m_MountedDeviceRotationOffset; //!< The relative offset angle (in radians) of the mounted HeldDevice from this Turret's rotation.

		/// Removes the HeldDevice from this turret's vector of mounted devices if it's in there. This releases the unique_ptr for it, leaving the caller to take care of it.
		/// @param mountedDeviceToRemove A pointer to the mounted device to remove.
		void RemoveMountedDevice(const HeldDevice* mountedDeviceToRemove);

		/// Clears all the member variables of this Turret, effectively resetting the members of this abstraction level only.
		void Clear();

		// Disallow the use of some implicit methods.
		Turret(const Turret& reference) = delete;
		Turret& operator=(const Turret& rhs) = delete;
	};
} // namespace RTE
