#pragma once

/// Header file for the HeldDevice class.
/// @author Daniel Tabar
/// data@datarealms.com
/// http://www.datarealms.com
/// Inclusions of header files
#include "Attachable.h"
#include "Actor.h"

#include <array>
#include <unordered_map>

namespace RTE {

	enum HeldDeviceType {
		WEAPON = 0,
		TOOL,
		SHIELD,
		BOMB,
	};

	/// An articulated device that can be weilded by an Actor.
	/// 01/31/2007 Made concrete so Shields can be jsut HeldDevice:s
	class HeldDevice : public Attachable {

		/// Public member variable, method and friend function declarations
	public:
		// Concrete allocation and cloning definitions
		EntityAllocation(HeldDevice);
		SerializableOverrideMethods;
		ClassInfoGetters;

		/// Constructor method used to instantiate a HeldDevice object in system
		/// memory. Create() should be called before using the object.
		HeldDevice();

		/// Destructor method used to clean up a HeldDevice object before deletion
		/// from system memory.
		~HeldDevice() override;

		/// Makes the HeldDevice object ready for use.
		/// @return An error return value signaling sucess or any particular failure.
		/// Anything below 0 is an error signal.
		int Create() override;

		/// Creates a HeldDevice to be identical to another, by deep copy.
		/// @param reference A reference to the HeldDevice to deep copy.
		/// @return An error return value signaling sucess or any particular failure.
		/// Anything below 0 is an error signal.
		int Create(const HeldDevice& reference);

		/// Resets the entire HeldDevice, including its inherited members, to their
		/// default settings or values.
		void Reset() override {
			Clear();
			Attachable::Reset();
			m_MOType = MovableObject::TypeHeldDevice;
		}

		/// Destroys and resets (through Clear()) the SceneLayer object.
		/// @param notInherited Whether to only destroy the members defined in this derived class, or (default: false)
		/// to destroy all inherited members also.
		void Destroy(bool notInherited = false) override;

		/// Gets the absoltue position of the top of this' HUD stack.
		/// @return A Vector with the absolute position of this' HUD stack top point.
		Vector GetAboveHUDPos() const override { return m_Pos + Vector(0, -32); }

		/// Gets the absolute position of the support handhold that this HeldDevice
		/// offers.
		/// @return A vector describing the absolute world coordinates for the support
		/// position of this HeldDevice.
		Vector GetSupportPos() const;

		/// Gets the absolute position of the magazine or other equivalent point of
		/// this.
		/// @return A vector describing the absolute world coordinates for the magazine
		/// attachment point of this
		virtual Vector GetMagazinePos() const;

		/// Gets the absolute position of the muzzle or other equivalent point of
		/// this.
		/// @return A vector describing the absolute world coordinates for the muzzle point
		/// of this
		virtual Vector GetMuzzlePos() const { return m_Pos; }

		/// Gets the unrotated relative offset from the position to the muzzle or
		/// other equivalent point of this.
		/// @return A unrotated vector describing the relative for the muzzle point of
		/// this from this' position.
		virtual Vector GetMuzzleOffset() const { return Vector(); }

		/// Sets the unrotated relative offset from the position to the muzzle or
		/// other equivalent point of this.
		/// @param newOffset Bew ofsset value.
		virtual void SetMuzzleOffset(Vector newOffset) { /* Actually does something in inherited classes */
		}

		/// Gets the current position offset of this HeldDevice's joint relative
		/// from the parent Actor's position, if attached.
		/// @return A const reference to the current stance parent offset.
		virtual Vector GetStanceOffset() const;

		/// Sets the current position offset of this HeldDevice's joint relative
		/// from the parent Actor's position, if attached.
		/// @param newValue New value.
		void SetStanceOffset(Vector newValue) { m_StanceOffset = newValue; }

		/// Sets the current position offset of this HeldDevice's joint relative
		/// from the parent Actor's position, if attached.
		/// @param New value.
		Vector GetSharpStanceOffset() const { return m_SharpStanceOffset; }

		/// Sets the current position offset of this HeldDevice's joint relative
		/// from the parent Actor's position, if attached.
		/// @param newValue New value.
		void SetSharpStanceOffset(Vector newValue) { m_SharpStanceOffset = newValue; }

		/// Gets how much farther an Actor which holds this device can see when
		/// aiming this HeldDevice sharply.
		/// @return The length in world pixel units.
		float GetSharpLength() const { return m_MaxSharpLength; }

		/// Sets how much farther an Actor which holds this device can see when
		/// aiming this HeldDevice sharply.
		/// @param newLength The length in world pixel units.
		void SetSharpLength(float newLength) { m_MaxSharpLength = newLength; }

		/// Gets whether this HeldDevice can be supported when held.
		/// @return Whether this HeldDevice can be supported when held.
		bool IsSupportable() const { return m_Supportable; }

		/// Sets whether this HeldDevice can be supported when held.
		/// @param shouldBeSupportable Whether this HeldDevice can be supported when held.
		void SetSupportable(bool shouldBeSupportable) { m_Supportable = shouldBeSupportable; }

		/// Gets whether this HeldDevice is currently supported by a second Arm.
		/// @return Whether this HeldDevice is supported or not.
		bool GetSupported() const { return m_Supportable && m_Supported; }

		/// Sets whether this HeldDevice is currently supported by a second Arm.
		/// @param supported Whether this HeldDevice is being supported.
		void SetSupported(bool supported) { m_Supported = m_Supportable && supported; }

		/// Gets whether this HeldDevice's parent has a second Arm available to provide support (or this is on a Turret).
		/// @return Whether this HeldDevice's parent has a second Arm available to provide support (or this is on a Turret).
		bool GetSupportAvailable() const { return m_Supportable && m_SupportAvailable; }

		/// Sets whether this HeldDevice's parent has a second Arm available to provide support (or this is on a Turret).
		/// @param supported Whether this HeldDevice's parent has a second Arm available to provide support (or this is on a Turret).
		void SetSupportAvailable(bool supportAvailable) { m_SupportAvailable = m_Supportable && supportAvailable; }

		/// Gets whether this HeldDevice while be held at the support offset with the off-hand when reloading.
		/// @return Whether this HeldDevice while be held at the support offset with the off-hand when reloading.
		bool GetUseSupportOffsetWhileReloading() const { return m_UseSupportOffsetWhileReloading; }

		/// Sets whether this HeldDevice while be held at the support offset with the off-hand when reloading.
		/// @param value Whether this HeldDevice while be held at the support offset with the off-hand when reloading.
		void SetUseSupportOffsetWhileReloading(bool value) { m_UseSupportOffsetWhileReloading = value; }

		/// Returns support offset.
		/// @return Support offset value.
		Vector GetSupportOffset() const { return m_SupportOffset; }

		/// Sets support offset.
		/// @param newOffset New support offset value.
		void SetSupportOffset(Vector newOffset) { m_SupportOffset = newOffset; }

		/// Gets whether this HeldDevice has any limitations on what can pick it up.
		/// @return Whether this HeldDevice has any limitations on what can pick it up.
		bool HasPickupLimitations() const { return IsUnPickupable() || !m_PickupableByPresetNames.empty(); }

		/// Gets whether this HeldDevice cannot be picked up at all.
		/// @return Whether this HeldDevice cannot be picked up at all.
		bool IsUnPickupable() const { return m_IsUnPickupable; }

		/// Sets whether this HeldDevice cannot be picked up at all.
		/// @param shouldBeUnPickupable Whether this HeldDevice cannot be picked up at all. True means it cannot, false means any other limitations will apply normally.
		void SetUnPickupable(bool shouldBeUnPickupable) { m_IsUnPickupable = shouldBeUnPickupable; }

		/// Checks whether the given Actor can pick up this HeldDevice.
		/// @param actor The Actor to check. Ownership is NOT transferred.
		/// @return Whether the given Actor can pick up this HeldDevice.
		bool IsPickupableBy(const Actor* actor) const { return !HasPickupLimitations() || m_PickupableByPresetNames.find(actor->GetPresetName()) != m_PickupableByPresetNames.end(); }

		/// Specify that objects with the given PresetName can pick up this HeldDevice.
		/// @param presetName The PresetName of an object that should be able to pick up this HeldDevice.
		void AddPickupableByPresetName(const std::string& presetName) {
			SetUnPickupable(false);
			m_PickupableByPresetNames.insert(presetName);
		}

		/// Remove allowance for objects with the given PresetName to pick up this HeldDevice.
		/// Note that if the last allowance is removed, the HeldDevice will no longer have pickup limitations, rather than setting itself as unpickupable.
		/// @param actorPresetName The PresetName of an object that should no longer be able to pick up this HeldDevice.
		void RemovePickupableByPresetName(const std::string& actorPresetName);

		/// Gets the multiplier for how well this HeldDevice can be gripped by Arms.
		/// @return The grip strength multiplier for this HeldDevice.
		float GetGripStrengthMultiplier() const { return m_GripStrengthMultiplier; }

		/// Sets the multiplier for how well this HeldDevice can be gripped by Arms.
		/// @param gripStrengthMultiplier The new grip strength multiplier for this HeldDevice.
		void SetGripStrengthMultiplier(float gripStrengthMultiplier) { m_GripStrengthMultiplier = gripStrengthMultiplier; }

		/// Gets whether this can get hit by MOs when held.
		/// @return Whether this can get hit by MOs when held.
		bool GetsHitByMOsWhenHeld() const { return m_GetsHitByMOsWhenHeld; }

		/// Sets whether this can get hit by MOs when held.
		/// @param value Whether this can get hit by MOs when held.
		void SetGetsHitByMOsWhenHeld(bool value) { m_GetsHitByMOsWhenHeld = value; }

		/// Gets whether this HeldDevice is currently being held or not.
		/// @return Whether this HeldDevice is currently being held or not.
		bool IsBeingHeld() const;

		/// Gets the visual recoil multiplier.
		/// @return A float with the scalar value.
		float GetVisualRecoilMultiplier() const { return m_VisualRecoilMultiplier; }

		/// Sets the visual recoil multiplier.
		/// @param value The new recoil multiplier scalar.
		void SetVisualRecoilMultiplier(float value) { m_VisualRecoilMultiplier = value; }

		/// Sets the degree to which this is being aimed sharp. This will
		/// affect the accuracy and what GetParentOffset returns.
		/// @param sharpAim A normalized scalar between 0 (no sharp aim) to 1.0 (best aim).
		void SetSharpAim(float sharpAim) { m_SharpAim = sharpAim; }

		/// Indicates whether this is an offensive weapon or not.
		/// @return Offensive weapon or not.
		bool IsWeapon() { return m_HeldDeviceType == WEAPON; }

		/// Indicates whether this is a tool or not.
		/// @return Tool or not.
		bool IsTool() { return m_HeldDeviceType == TOOL; }

		/// Indicates whether this is a shield or not.
		/// @return Shield or not.
		bool IsShield() { return m_HeldDeviceType == SHIELD; }

		/// Indicates whether this is a dual wieldable weapon or not.
		/// @return Dual wieldable or not.
		bool IsDualWieldable() const { return m_DualWieldable; }

		/// Sets whether this is a dual wieldable weapon or not.
		/// @param isDualWieldable Dual wieldable or not.
		void SetDualWieldable(bool isDualWieldable) { m_DualWieldable = isDualWieldable; }

		/// Indicates whether this can be held and operated effectively with one
		/// hand or not.
		/// @return One handed device or not.
		bool IsOneHanded() const { return m_OneHanded; }

		/// Sets whether this can be held and operated effectively with one
		/// hand or not.
		/// @param newValue New value.
		void SetOneHanded(bool newValue) { m_OneHanded = newValue; }

		/// Calculates the collision response when another MO's Atom collides with
		/// this MO's physical representation. The effects will be applied
		/// directly to this MO, and also represented in the passed in HitData.
		/// @param hitData Reference to the HitData struct which describes the collision. This
		/// will be modified to represent the results of the collision.
		/// @return Whether the collision has been deemed valid. If false, then disregard
		/// any impulses in the Hitdata.
		bool CollideAtPoint(HitData& hitData) override;

		/// Activates one of this HDFirearm's features. Analogous to 'pulling
		/// the trigger'.
		virtual void Activate();

		/// Deactivates one of this HDFirearm's features. Analogous to 'releasing
		/// the trigger'.
		virtual void Deactivate();

		/// Throws out the currently used Magazine, if any, and puts in a new one
		/// after the reload delay is up.
		virtual void Reload() {}

		/// Tells whether the device is curtrently being activated.
		/// @return Whether being activated.
		virtual bool IsActivated() const { return m_Activated; }

		/// Gets the activation Timer for this HeldDevice.
		/// @return The activation Timer for this HeldDevice.
		const Timer& GetActivationTimer() const { return m_ActivationTimer; }

		/// Tells whether the device is curtrently being reloaded.
		/// @return Whetehr being reloaded.
		virtual bool IsReloading() const { return false; }

		/// Tells whether the device just finished reloading this frame.
		/// @return Whether just done reloading this frame.
		virtual bool DoneReloading() const { return false; }

		/// Tells whether the device is curtrently in need of being reloaded.
		/// @return Whetehr in need of reloading (ie not full).
		virtual bool NeedsReloading() const { return false; }

		/// Tells whether the device is curtrently full and reloading won't have
		/// any effect.
		/// @return Whetehr magazine is full or not.
		virtual bool IsFull() const { return true; }

		/// Tells whether this HeldDevice is currently empty of ammo.
		/// @return Whether this HeldDevice is empty.
		virtual bool IsEmpty() const { return false; }

		/// Updates this MovableObject. Supposed to be done every frame.
		void Update() override;

		/// Draws this HeldDevice's current graphical representation to a
		/// BITMAP of choice.
		/// @param pTargetBitmap A pointer to a BITMAP to draw on.
		/// @param targetPos The absolute position of the target bitmap's upper left corner in the Scene. (default: Vector())
		/// @param mode In which mode to draw in. See the DrawMode enumeration for the modes. (default: g_DrawColor)
		/// @param onlyPhysical Whether to not draw any extra 'ghost' items of this MovableObject, (default: false)
		/// indicator arrows or hovering HUD text and so on.
		void Draw(BITMAP* pTargetBitmap, const Vector& targetPos = Vector(), DrawMode mode = g_DrawColor, bool onlyPhysical = false) const override;

		/// Draws this' current graphical HUD overlay representation to a
		/// BITMAP of choice.
		/// @param pTargetBitmap A pointer to a BITMAP to draw on.
		/// @param targetPos The absolute position of the target bitmap's upper left corner in the Scene. (default: Vector())
		/// @param whichScreen Which player's screen this is being drawn to. May affect what HUD elements (default: 0)
		/// get drawn etc.
		void DrawHUD(BITMAP* pTargetBitmap, const Vector& targetPos = Vector(), int whichScreen = 0, bool playerControlled = false) override;

		/// Resest all the timers used by this. Can be emitters, etc. This is to prevent backed up emissions to come out all at once while this has been held dormant in an inventory.
		void ResetAllTimers() override {
			Attachable::ResetAllTimers();
			m_ActivationTimer.Reset();
		}

#pragma region Force Transferral
		/// Bundles up all the accumulated impulse forces of this HeldDevice and calculates how they transfer to the joint, and therefore to the parent.
		/// If the accumulated impulse forces exceed the joint strength or gib impulse limit of this HeldDevice, the jointImpulses Vector will be filled up to that limit and false will be returned.
		/// Additionally, in this case, the HeldDevice will remove itself from its parent, destabilizing said parent if it's an Actor, and gib itself if appropriate.
		/// @param jointImpulses A vector that will have the impulse forces affecting the joint ADDED to it.
		/// @param jointStiffnessValueToUse An optional override for the HeldDevice's joint stiffness for this function call. Primarily used to allow subclasses to perform special behavior.
		/// @param jointStrengthValueToUse An optional override for the HeldDevice's joint strength for this function call. Primarily used to allow subclasses to perform special behavior.
		/// @param gibImpulseLimitValueToUse An optional override for the HeldDevice's gib impulse limit for this function call. Primarily used to allow subclasses to perform special behavior.
		/// @return False if the HeldDevice has no parent or its accumulated forces are greater than its joint strength or gib impulse limit, otherwise true.
		bool TransferJointImpulses(Vector& jointImpulses, float jointStiffnessValueToUse = -1, float jointStrengthValueToUse = -1, float gibImpulseLimitValueToUse = -1) override;
#pragma endregion

		/// Protected member variable and method declarations
	protected:
		// Member variables
		static Entity::ClassInfo m_sClass;
		// Indicates what kind of held device this is, see the HeldDeviceType enum
		int m_HeldDeviceType;
		// Is this HeldDevice that are currently activated?
		bool m_Activated;
		// Timer for timing how long a feature has been activated.
		Timer m_ActivationTimer;
		// Can be weilded well with one hand or not
		bool m_OneHanded;
		// Can be weilded with bg hand or not
		bool m_DualWieldable;
		// Position offset from the parent's own position to this HeldDevice's joint, which
		// defines the normal stance that an arm that is holding this device should have.
		Vector m_StanceOffset;
		// The alternative parent offset stance that is used when the device is carefully aimed.
		Vector m_SharpStanceOffset;
		// The point at which the other arm of the holder can support this HeldDevice.
		// Relative to the m_Pos. This is like a seconday handle position.
		Vector m_SupportOffset;
		// Whether the actor using this gun should keep hold of the support offset when reloading, instead of using their ReloadOffset/HolsterOffset
		bool m_UseSupportOffsetWhileReloading;
		// The degree as to this is being aimed carefully. 0 means no sharp aim, and 1.0 means best aim.
		float m_SharpAim;
		// How much farther the player can see when aiming this sharply.
		float m_MaxSharpLength;
		bool m_Supportable; //!< Whether or not this HeldDevice can be supported.
		bool m_Supported; //!< Whether or not this HeldDevice is currently being supported by another Arm.
		bool m_SupportAvailable; //!< Whether or not this HeldDevice's parent has a second Arm available to provide support (or this is on a Turret).
		bool m_IsUnPickupable; //!< Whether or not this HeldDevice should be able to be picked up at all.
		// TODO: move this smelly thing elsewhere
		std::array<bool, Players::MaxPlayerCount> m_SeenByPlayer; //!< An array of players that can currently see the pickup HUD of this HeldDevice.
		std::unordered_set<std::string> m_PickupableByPresetNames; //!< The unordered set of PresetNames that can pick up this HeldDevice if it's dropped. An empty set means there are no PresetName limitations.
		float m_GripStrengthMultiplier; //!< The multiplier for how well this HeldDevice can be gripped by Arms.
		// Blink timer for the icon
		Timer m_BlinkTimer;
		// How loud this device is when activated. 0 means perfectly quiet 0.5 means half of normal (normal equals audiable from ~half a screen)
		float m_Loudness;
		// If this weapon belongs to the "Explosive Weapons" group or not
		bool m_IsExplosiveWeapon;
		// If this device can be hit by MOs whenever it's held
		bool m_GetsHitByMOsWhenHeld;
		/// The multiplier for visual recoil
		float m_VisualRecoilMultiplier;

		/// Private member variable and method declarations
	private:
		/// Clears all the member variables of this HeldDevice, effectively
		/// resetting the members of this abstraction level only.
		void Clear();

		// Disallow the use of some implicit methods.
		HeldDevice(const HeldDevice& reference) = delete;
		HeldDevice& operator=(const HeldDevice& rhs) = delete;
	};

} // namespace RTE
