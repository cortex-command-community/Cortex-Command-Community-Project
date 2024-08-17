#pragma once

/// Header file for the AHuman class.
/// @author Daniel Tabar
/// data@datarealms.com
/// http://www.datarealms.com
/// Inclusions of header files
#include "Actor.h"
#include "Arm.h"
#include "Leg.h"
#include "LimbPath.h"

#include <array>

struct BITMAP;

namespace RTE {

	class AEJetpack;

	/// A humanoid actor.
	class AHuman : public Actor {
		friend struct EntityLuaBindings;

		enum UpperBodyState {
			WEAPON_READY = 0,
			AIMING_SHARP,
			HOLSTERING_BACK,
			HOLSTERING_BELT,
			DEHOLSTERING_BACK,
			DEHOLSTERING_BELT,
			THROWING_PREP,
			THROWING_RELEASE
		};

		enum ProneState {
			NOTPRONE = 0,
			GOPRONE,
			PRONE,
			PRONESTATECOUNT
		};

		enum Layer {
			FGROUND = 0,
			BGROUND
		};

		/// Public member variable, method and friend function declarations
	public:
		// Concrete allocation and cloning definitions
		EntityAllocation(AHuman);
		AddScriptFunctionNames(Actor, "OnStride");
		SerializableOverrideMethods;
		ClassInfoGetters;
		DefaultPieMenuNameGetter("Default Human Pie Menu");

		/// Constructor method used to instantiate a AHuman object in system
		/// memory. Create() should be called before using the object.
		AHuman();

		/// Destructor method used to clean up a AHuman object before deletion
		/// from system memory.
		~AHuman() override;

		/// Makes the AHuman object ready for use.
		/// @return An error return value signaling sucess or any particular failure.
		/// Anything below 0 is an error signal.
		int Create() override;

		/// Creates a AHuman to be identical to another, by deep copy.
		/// @param reference A reference to the AHuman to deep copy.
		/// @return An error return value signaling sucess or any particular failure.
		/// Anything below 0 is an error signal.
		int Create(const AHuman& reference);

		/// Resets the entire AHuman, including its inherited members, to their
		/// default settings or values.
		void Reset() override {
			Clear();
			Actor::Reset();
		}

		/// Destroys and resets (through Clear()) the SceneLayer object.
		/// @param notInherited Whether to only destroy the members defined in this derived class, or (default: false)
		/// to destroy all inherited members also.
		void Destroy(bool notInherited = false) override;

		/// Gets the total liquidation value of this Actor and all its carried
		/// gold and inventory.
		/// @param nativeModule If this is supposed to be adjusted for a specific Tech's subjective (default: 0)
		/// value, then pass in the native DataModule ID of that tech. 0 means
		/// no Tech is specified and the base value is returned.
		/// @param foreignMult How much to multiply the value if this happens to be a foreign Tech. (default: 1.0)
		/// @return The current value of this Actor and all his carried assets.
		float GetTotalValue(int nativeModule = 0, float foreignMult = 1.0, float nativeMult = 1.0) const override;

		/// Shows whether this is or carries a specifically named object in its
		/// inventory. Also looks through the inventories of potential passengers,
		/// as applicable.
		/// @param objectName The Preset name of the object to look for.
		/// @return Whetehr the object was found carried by this.
		bool HasObject(std::string objectName) const override;

		/// Shows whether this is or carries a specifically grouped object in its
		/// inventory. Also looks through the inventories of potential passengers,
		/// as applicable.
		/// @param groupName The name of the group to look for.
		/// @return Whetehr the object in the group was found carried by this.
		bool HasObjectInGroup(std::string groupName) const override;

		/// Gets the absoltue position of this' brain, or equivalent.
		/// @return A Vector with the absolute position of this' brain.
		Vector GetCPUPos() const override;

		/// Gets the absoltue position of this' eye, or equivalent, where look
		/// vector starts from.
		/// @return A Vector with the absolute position of this' eye or view point.
		Vector GetEyePos() const override;

		/// Gets the head of this AHuman.
		/// @return A pointer to the head of this AHuman. Ownership is NOT transferred.
		Attachable* GetHead() const { return m_pHead; }

		/// Sets the head for this AHuman.
		/// @param newHead The new head to use.
		void SetHead(Attachable* newHead);

		/// Gets the jetpack of this AHuman.
		/// @return A pointer to the jetpack of this AHuman. Ownership is NOT transferred.
		AEJetpack* GetJetpack() const { return m_pJetpack; }

		/// Sets the jetpack for this AHuman.
		/// @param newJetpack The new jetpack to use.
		void SetJetpack(AEJetpack* newJetpack);

		/// Gets the foreground Arm of this AHuman.
		/// @return A pointer to the foreground Arm of this AHuman. Ownership is NOT transferred.
		Arm* GetFGArm() const { return m_pFGArm; }

		/// Sets the foreground Arm for this AHuman.
		/// @param newArm The new Arm to use.
		void SetFGArm(Arm* newArm);

		/// Gets the background arm of this AHuman.
		/// @return A pointer to the background arm of this AHuman. Ownership is NOT transferred.
		Arm* GetBGArm() const { return m_pBGArm; }

		/// Sets the background Arm for this AHuman.
		/// @param newArm The new Arm to use.
		void SetBGArm(Arm* newArm);

		/// Gets the foreground Leg of this AHuman.
		/// @return A pointer to the foreground Leg of this AHuman. Ownership is NOT transferred.
		Leg* GetFGLeg() const { return m_pFGLeg; }

		/// Sets the foreground Leg for this AHuman.
		/// @param newLeg The new Leg to use.
		void SetFGLeg(Leg* newLeg);

		/// Gets the background Leg of this AHuman.
		/// @return A pointer to the background Leg of this AHuman. Ownership is NOT transferred.
		Leg* GetBGLeg() const { return m_pBGLeg; }

		/// Sets the background Leg for this AHuman.
		/// @param newLeg The new Leg to use.
		void SetBGLeg(Leg* newLeg);

		/// Gets the foot Attachable of this AHuman's foreground Leg.
		/// @return A pointer to the foot Attachable of this AHuman's foreground Leg. Ownership is NOT transferred!
		Attachable* GetFGFoot() const { return m_pFGLeg ? m_pFGLeg->GetFoot() : nullptr; }

		/// Sets the foot Attachable of this AHuman's foreground Leg.
		/// @param newFoot The new foot for this AHuman's foreground Leg to use.
		void SetFGFoot(Attachable* newFoot) {
			if (m_pFGLeg && m_pFGLeg->IsAttached()) {
				m_pFGLeg->SetFoot(newFoot);
			}
		}

		/// Gets the foot Attachable of this AHuman's background Leg.
		/// @return A pointer to the foot Attachable of this AHuman's background Leg. Ownership is NOT transferred!
		Attachable* GetBGFoot() const { return m_pBGLeg ? m_pBGLeg->GetFoot() : nullptr; }

		/// Sets the foot Attachable of this AHuman's background Leg.
		/// @param newFoot The new foot for this AHuman's background Leg to use.
		void SetBGFoot(Attachable* newFoot) {
			if (m_pBGLeg && m_pBGLeg->IsAttached()) {
				m_pBGLeg->SetFoot(newFoot);
			}
		}

		/// Gets this AHuman's UpperBodyState.
		/// @return This AHuman's UpperBodyState.
		UpperBodyState GetUpperBodyState() const { return m_ArmsState; }

		/// Sets this AHuman's UpperBodyState to the new state.
		/// @param newUpperBodyState This AHuman's new UpperBodyState.
		void SetUpperBodyState(UpperBodyState newUpperBodyState) { m_ArmsState = newUpperBodyState; }

		/// Gets this AHuman's ProneState.
		/// @return This AHuman's ProneState.
		ProneState GetProneState() const { return m_ProneState; }

		/// Sets this AHuman's ProneState to the new state.
		/// @param newProneState This AHuman's new ProneState.
		void SetProneState(ProneState newProneState) { m_ProneState = newProneState; }

		/// Tries to handle the activated PieSlice in this object's PieMenu, if there is one, based on its SliceType.
		/// @param pieSliceType The SliceType of the PieSlice being handled.
		/// @return Whether or not the activated PieSlice SliceType was able to be handled.
		bool HandlePieCommand(PieSliceType pieSliceType) override;

		/// Adds an inventory item to this AHuman. This also puts that item
		/// directly in the hands of this if they are empty.
		/// @param pItemToAdd An pointer to the new item to add. Ownership IS TRANSFERRED!
		void AddInventoryItem(MovableObject* pItemToAdd) override;

		/// Swaps the next MovableObject carried by this AHuman and puts one not currently carried into the back of the inventory of this.
		/// For safety reasons, this will dump any non-HeldDevice inventory items it finds into MovableMan, ensuring the returned item is a HeldDevice (but not casted to one, for overload purposes).
		/// @param inventoryItemToSwapIn A pointer to the external MovableObject to swap in. Ownership IS transferred.
		/// @param muteSound Whether or not to mute the sound on this event.
		/// @return The next HeldDevice in this AHuman's inventory, if there are any.
		MovableObject* SwapNextInventory(MovableObject* inventoryItemToSwapIn = nullptr, bool muteSound = false) override;

		/// Swaps the previous MovableObject carried by this AHuman and puts one not currently carried into the back of the inventory of this.
		/// For safety reasons, this will dump any non-HeldDevice inventory items it finds into MovableMan, ensuring the returned item is a HeldDevice (but not casted to one, for overload purposes).
		/// @param inventoryItemToSwapIn A pointer to the external MovableObject to swap in. Ownership IS transferred.
		/// @return The previous HeldDevice in this AHuman's inventory, if there are any.
		MovableObject* SwapPrevInventory(MovableObject* inventoryItemToSwapIn = nullptr) override;

		/// Switches the currently held device (if any) to the first found firearm
		/// in the inventory. If the held device already is a firearm, or no
		/// firearm is in inventory, nothing happens.
		/// @param doEquip Whether to actually equip any matching item found in the inventory, (default: true)
		/// or just report that it's there or not.
		/// @return Whether a firearm was successfully switched to, or already held.
		bool EquipFirearm(bool doEquip = true);

		/// Switches the currently held device (if any) to the first found device
		/// of the specified group in the inventory. If the held device already
		/// is of that group, or no device is in inventory, nothing happens.
		/// @param group The group the device must belong to.
		/// @param doEquip Whether to actually equip any matching item found in the inventory, (default: true)
		/// or just report that it's there or not.
		/// @return Whether a firearm was successfully switched to, or already held.
		bool EquipDeviceInGroup(std::string group, bool doEquip = true);

		/// Switches the currently held device (if any) to the first loaded HDFirearm
		/// of the specified group in the inventory. If no such weapon is in the
		/// inventory, nothing happens.
		/// @param group The group the HDFirearm must belong to. "Any" for all groups.
		/// @param exludeGroup The group the HDFirearm must *not* belong to. "None" for no group.
		/// @param doEquip Whether to actually equip any matching item found in the inventory, (default: true)
		/// or just report that it's there or not.
		/// @return Whether a firearm was successfully switched to, or already held.
		bool EquipLoadedFirearmInGroup(std::string group, std::string exludeGroup, bool doEquip = true);

		/// Switches the equipped HeldDevice (if any) to the first found device with the specified preset name in the inventory.
		/// If the equipped HeldDevice is of that module and preset name, nothing happens.
		/// @param presetName The preset name of the HeldDevice to equip.
		/// @param doEquip Whether to actually equip any matching item found in the inventory, or just report whether or not it's there.
		/// @return Whether a matching HeldDevice was successfully found/switched -o, or already held.
		bool EquipNamedDevice(const std::string& presetName, bool doEquip) { return EquipNamedDevice("", presetName, doEquip); }

		/// Switches the equipped HeldDevice (if any) to the first found device with the specified module and preset name in the inventory.
		/// If the equipped HeldDevice is of that module and preset name, nothing happens.
		/// @param moduleName The module name of the HeldDevice to equip.
		/// @param presetName The preset name of the HeldDevice to equip.
		/// @param doEquip Whether to actually equip any matching item found in the inventory, or just report whether or not it's there.
		/// @return Whether a matching HeldDevice was successfully found/switched -o, or already held.
		bool EquipNamedDevice(const std::string& moduleName, const std::string& presetName, bool doEquip);

		/// Switches the currently held device (if any) to the first found ThrownDevice
		/// in the inventory. If the held device already is a ThrownDevice, or no
		/// ThrownDevice  is in inventory, nothing happens.
		/// @param doEquip Whether to actually equip any matching item found in the inventory, (default: true)
		/// or just report that it's there or not.
		/// @return Whether a ThrownDevice was successfully switched to, or already held.
		bool EquipThrowable(bool doEquip = true);

		/// Switches the currently held device (if any) to the strongest digging tool in the inventory.
		/// @param doEquip Whether to actually equip the strongest digging tool, or just report whether a digging tool was found.
		/// @return Whether or not the strongest digging tool was successfully equipped.
		bool EquipDiggingTool(bool doEquip = true);

		/// Estimates what material strength any digger this AHuman is carrying can penetrate.
		/// @return The maximum material strength this AHuman's digger can penetrate, or a default dig strength if they don't have a digger.
		float EstimateDigStrength() const override;

		/// Switches the currently held device (if any) to the first found shield
		/// in the inventory. If the held device already is a shield, or no
		/// shield is in inventory, nothing happens.
		/// @return Whether a shield was successfully switched to, or already held.
		bool EquipShield();

		/// Tries to equip the first shield in inventory to the background arm;
		/// this only works if nothing is held at all, or the FG arm holds a
		/// one-handed device, or we're in inventory mode.
		/// @return Whether a shield was successfully equipped in the background arm.
		bool EquipShieldInBGArm();

		/// Tries to equip the first dual-wieldable in inventory to the background arm;
		/// this only works if nothing is held at all, or the FG arm holds a
		/// one-handed device, or we're in inventory mode.
		/// @return Whether a shield was successfully equipped in the background arm.
		//	bool EquipDualWieldableInBGArm();

		/// Gets the throw chargeup progress of this AHuman.
		/// @return The throw chargeup progress, as a scalar from 0 to 1.
		float GetThrowProgress() const { return m_ThrowPrepTime > 0 ? static_cast<float>(std::min(m_ThrowTmr.GetElapsedSimTimeMS() / static_cast<double>(m_ThrowPrepTime), 1.0)) : 1.0F; }

		/// Unequips whatever is in the FG arm and puts it into the inventory.
		/// @return Whether there was anything to unequip.
		bool UnequipFGArm();

		/// Unequips whatever is in the BG arm and puts it into the inventory.
		/// @return Whether there was anything to unequip.
		bool UnequipBGArm();

		/// Unequips whatever is in either of the arms and puts them into the inventory.
		void UnequipArms() {
			UnequipBGArm();
			UnequipFGArm();
		}

		/// Gets the FG Arm's HeldDevice. Ownership is NOT transferred.
		/// @return The FG Arm's HeldDevice.
		HeldDevice* GetEquippedItem() const { return m_pFGArm ? m_pFGArm->GetHeldDevice() : nullptr; }

		/// Gets the BG Arm's HeldDevice. Ownership is NOT transferred.
		/// @return The BG Arm's HeldDevice.
		HeldDevice* GetEquippedBGItem() const { return m_pBGArm ? m_pBGArm->GetHeldDevice() : nullptr; }

		/// Gets the total mass of this AHuman's currently equipped devices.
		/// @return The mass of this AHuman's equipped devices.
		float GetEquippedMass() const;

		/// Indicates whether the currently held HDFirearm's is ready for use, and has
		/// ammo etc.
		/// @return Whether a currently HDFirearm (if any) is ready for use.
		bool FirearmIsReady() const;

		/// Indicates whether the currently held ThrownDevice's is ready to go.
		/// @return Whether a currently held ThrownDevice (if any) is ready for use.
		bool ThrowableIsReady() const;

		/// Indicates whether the currently held HDFirearm's is out of ammo.
		/// @return Whether a currently HDFirearm (if any) is out of ammo.
		bool FirearmIsEmpty() const;

		/// Indicates whether any currently held HDFirearms are almost out of ammo.
		/// @return Whether a currently HDFirearm (if any) has less than half of ammo left.
		bool FirearmNeedsReload() const;

		/// Indicates whether currently held HDFirearms are reloading. If the parameter is true, it will only return true if all firearms are reloading, otherwise it will return whether any firearm is reloading.
		/// @return Whether or not currently held HDFirearms are reloading.
		bool FirearmsAreReloading(bool onlyIfAllFirearmsAreReloading) const;

		/// Indicates whether the currently held HDFirearm's is semi or full auto.
		/// @return Whether a currently HDFirearm (if any) is a semi auto device.
		bool FirearmIsSemiAuto() const;

		/// Returns the currently held device's delay between pulling the trigger
		/// and activating.
		/// @return Delay in ms or zero if not a HDFirearm.
		int FirearmActivationDelay() const;

		/// Reloads the currently held firearm, if any. Will only reload the BG Firearm if the FG one is full already, to support reloading guns one at a time.
		/// @param onlyReloadEmptyFirearms Whether or not to only reload empty fireams. (default: false)
		void ReloadFirearms(bool onlyReloadEmptyFirearms = false);

		/// Tells whether a point on the scene is within close range of the currently
		/// used device and aiming status, if applicable.
		/// @param point A Vector with the aboslute coordinates of a point to check.
		/// @return Whether the point is within close range of this.
		bool IsWithinRange(Vector& point) const override;

		/// Casts an unseen-revealing ray in the direction of where this is facing.
		/// @param FOVSpread The degree angle to deviate from the current view point in the ray
		/// casting. A random ray will be chosen out of this +-range.
		/// @param range The range, in pixels, beyond the actors sharp aim that the ray will have.
		/// @return Whether any unseen pixels were revealed by this look.
		bool Look(float FOVSpread, float range) override;

		/// Casts a material detecting ray in the direction of where this is facing.
		/// @param FOVSpread The degree angle to deviate from the current view point in the ray
		/// casting. A random ray will be chosen out of this +-range.
		/// @param range The range, in pixels, that the ray will have.
		/// @param foundLocation A Vector which will be filled with the absolute coordinates of any
		/// found gold. It will be unaltered if false is returned.
		/// @return Whether gold was spotted by this ray cast. If so, foundLocation
		/// has been filled out with the absolute location of the gold.
		bool LookForGold(float FOVSpread, float range, Vector& foundLocation) const;

		/// Casts an MO detecting ray in the direction of where the head is looking
		/// at the time. Factors including head rotation, sharp aim mode, and
		/// other variables determine how this ray is cast.
		/// @param FOVSpread The degree angle to deviate from the current view point in the ray (default: 45)
		/// casting. A random ray will be chosen out of this +-range.
		/// @param ignoreMaterial A specific material ID to ignore (see through) (default: 0)
		/// @param ignoreAllTerrain Whether to ignore all terrain or not (true means 'x-ray vision'). (default: false)
		/// @return A pointer to the MO seen while looking.
		MovableObject* LookForMOs(float FOVSpread = 45, unsigned char ignoreMaterial = 0, bool ignoreAllTerrain = false);

		/// Gets the GUI representation of this AHuman, only defaulting to its Head or body if no GraphicalIcon has been defined.
		/// @return The graphical representation of this AHuman as a BITMAP.
		BITMAP* GetGraphicalIcon() const override;

		/// Resest all the timers used by this. Can be emitters, etc. This is to
		/// prevent backed up emissions to come out all at once while this has been
		/// held dormant in an inventory.
		void ResetAllTimers() override;

		/// Detects slopes in terrain and updates the walk path rotation for the corresponding Layer accordingly.
		/// @param whichLayer The Layer in question.
		void UpdateWalkAngle(AHuman::Layer whichLayer);

		/// Detects overhead ceilings and crouches for them.
		void UpdateCrouching();

		/// Gets the walk path rotation for the specified Layer.
		/// @param whichLayer The Layer in question.
		/// @return The walk angle in radians.
		float GetWalkAngle(AHuman::Layer whichLayer) const { return m_WalkAngle[whichLayer].GetRadAngle(); }

		/// Sets the walk path rotation for the specified Layer.
		/// @param whichLayer The Layer in question.
		/// @param angle The angle to set.
		void SetWalkAngle(AHuman::Layer whichLayer, float angle) { m_WalkAngle[whichLayer] = Matrix(angle); }

		/// Gets whether this AHuman has just taken a stride this frame.
		/// @return Whether this AHuman has taken a stride this frame or not.
		bool StrideFrame() const { return m_StrideFrame; }

		/// Gets whether this AHuman is currently attempting to climb something, using arms.
		/// @return Whether this AHuman is currently climbing or not.
		bool IsClimbing() const { return m_ArmClimbing[FGROUND] || m_ArmClimbing[BGROUND]; }

		/// Update called prior to controller update. Ugly hack. Supposed to be done every frame.
		void PreControllerUpdate() override;

		/// Updates this MovableObject. Supposed to be done every frame.
		void Update() override;

		/// Draws this AHuman's current graphical representation to a
		/// BITMAP of choice.
		/// @param pTargetBitmap A pointer to a BITMAP to draw on.
		/// @param targetPos The absolute position of the target bitmap's upper left corner in the Scene. (default: Vector())
		/// @param mode In which mode to draw in. See the DrawMode enumeration for the modes. (default: g_DrawColor)
		/// @param onlyPhysical Whether to not draw any extra 'ghost' items of this MovableObject, (default: false)
		/// indicator arrows or hovering HUD text and so on.
		void Draw(BITMAP* pTargetBitmap, const Vector& targetPos = Vector(), DrawMode mode = g_DrawColor, bool onlyPhysical = false) const override;

		/// Draws this Actor's current graphical HUD overlay representation to a
		/// BITMAP of choice.
		/// @param pTargetBitmap A pointer to a BITMAP to draw on.
		/// @param targetPos The absolute position of the target bitmap's upper left corner in the Scene. (default: Vector())
		/// @param whichScreen Which player's screen this is being drawn to. May affect what HUD elements (default: 0)
		/// get drawn etc.
		void DrawHUD(BITMAP* pTargetBitmap, const Vector& targetPos = Vector(), int whichScreen = 0, bool playerControlled = false) override;

		/// Gets the LimbPath corresponding to the passed in Layer and MovementState values.
		/// @param layer Whether to get foreground or background LimbPath.
		/// @param movementState Which movement state to get the LimbPath for.
		/// @return The LimbPath corresponding to the passed in Layer and MovementState values.
		LimbPath* GetLimbPath(Layer layer, MovementState movementState) { return &m_Paths[layer][movementState]; }

		/// Get walking limb path speed for the specified preset.
		/// @param speedPreset Speed preset to set 0 = LimbPath::SLOW, 1 = Limbpath::NORMAL, 2 = LimbPath::FAST
		/// @return Limb path speed for the specified preset in m/s.
		float GetLimbPathSpeed(int speedPreset) const;

		/// Set walking limb path speed for the specified preset.
		/// @param speedPreset Speed preset to set 0 = LimbPath::SLOW, 1 = Limbpath::NORMAL, 2 = LimbPath::FAST. New speed value in m/s.
		void SetLimbPathSpeed(int speedPreset, float speed);

		/// Gets the default force that a limb traveling walking LimbPath can push against
		/// stuff in the scene with.
		/// @return The default set force maximum, in kg * m/s^2.
		float GetLimbPathPushForce() const;

		/// Sets the default force that a limb traveling walking LimbPath can push against
		/// stuff in the scene with.
		/// @param force The default set force maximum, in kg * m/s^2.
		void SetLimbPathPushForce(float force);

		/// Gets the target rot angle for the given MovementState.
		/// @param movementState The MovementState to get the rot angle target for.
		/// @return The target rot angle for the given MovementState.
		float GetRotAngleTarget(MovementState movementState) { return m_RotAngleTargets[movementState]; }

		/// Sets the target rot angle for the given MovementState.
		/// @param movementState The MovementState to get the rot angle target for.
		/// @param newRotAngleTarget The new rot angle target to use.
		void SetRotAngleTarget(MovementState movementState, float newRotAngleTarget) { m_RotAngleTargets[movementState] = newRotAngleTarget; }

		/// Gets the duration it takes this AHuman to fully charge a throw.
		/// @return The duration it takes to fully charge a throw in MS.
		long GetThrowPrepTime() const { return m_ThrowPrepTime; }

		/// Sets the duration it takes this AHuman to fully charge a throw.
		/// @param newPrepTime New duration to fully charge a throw in MS.
		void SetThrowPrepTime(long newPrepTime) { m_ThrowPrepTime = newPrepTime; }

		/// Gets the rate at which this AHuman's Arms will swing with Leg movement, if they're not holding or supporting a HeldDevice.
		/// @return The arm swing rate of this AHuman.
		float GetArmSwingRate() const { return m_ArmSwingRate; }

		/// Sets the rate at which this AHuman's Arms will swing with Leg movement, if they're not holding or supporting a HeldDevice.
		/// @param newValue The new arm swing rate for this AHuman.
		void SetArmSwingRate(float newValue) { m_ArmSwingRate = newValue; }

		/// Gets the rate at which this AHuman's Arms will sway with Leg movement, if they're holding or supporting a HeldDevice.
		/// @return The device arm sway rate of this AHuman.
		float GetDeviceArmSwayRate() const { return m_DeviceArmSwayRate; }

		/// Sets the rate at which this AHuman's Arms will sway with Leg movement, if they're holding or supporting a HeldDevice.
		/// @param newValue The new device arm sway rate for this AHuman.
		void SetDeviceArmSwayRate(float newValue) { m_DeviceArmSwayRate = newValue; }

		/// Gets this AHuman's max walkpath adjustment upwards to crouch below low ceilings.
		/// @return This AHuman's max walkpath adjustment.
		float GetMaxWalkPathCrouchShift() const { return m_MaxWalkPathCrouchShift; }

		/// Sets this AHuman's max walkpath adjustment upwards to crouch below low ceilings.
		/// @param newValue The new value for this AHuman's max walkpath adjustment.
		void SetMaxWalkPathCrouchShift(float newValue) { m_MaxWalkPathCrouchShift = newValue; }

		/// Gets this AHuman's max crouch rotation to duck below low ceilings.
		/// @return This AHuman's max crouch rotation adjustment.
		float GetMaxCrouchRotation() const { return m_MaxCrouchRotation; }

		/// Sets this AHuman's max crouch rotation to duck below low ceilings.
		/// @param newValue The new value for this AHuman's max crouch rotation adjustment.
		void SetMaxCrouchRotation(float newValue) { m_MaxCrouchRotation = newValue; }

		/// Gets this AHuman's current crouch amount. 0.0 == fully standing, 1.0 == fully crouched.
		/// @return This AHuman's current crouch amount.
		float GetCrouchAmount() const { return (m_WalkPathOffset.m_Y * -1.0F) / m_MaxWalkPathCrouchShift; }

		/// Gets this AHuman's current crouch amount override. 0.0 == fully standing, 1.0 == fully crouched, -1 == no override.
		/// @return This AHuman's current crouch amount override.
		float GetCrouchAmountOverride() const { return m_CrouchAmountOverride; }

		/// Sets this AHuman's current crouch amount override.
		/// @param newValue The new value for this AHuman's current crouch amount override.
		void SetCrouchAmountOverride(float newValue) { m_CrouchAmountOverride = newValue; }

		/// Gets this AHuman's stride sound. Ownership is NOT transferred!
		/// @return The SoundContainer for this AHuman's stride sound.
		SoundContainer* GetStrideSound() const { return m_StrideSound; }

		/// Sets this AHuman's stride sound. Ownership IS transferred!
		/// @param newSound The new SoundContainer for this AHuman's stride sound.
		void SetStrideSound(SoundContainer* newSound) { m_StrideSound = newSound; }

		/// Protected member variable and method declarations
	protected:
		/// Function that is called when we get a new movepath.
		/// This processes and cleans up the movepath.
		void OnNewMovePath() override;

		/// Draws an aiming aid in front of this AHuman for throwing.
		/// @param targetBitmap A pointer to a BITMAP to draw on.
		/// @param targetPos The absolute position of the target bitmap's upper left corner in the Scene.
		/// @param progressScalar A normalized scalar that determines the magnitude of the reticle, to indicate force in the throw.
		void DrawThrowingReticle(BITMAP* targetBitmap, const Vector& targetPos = Vector(), float progressScalar = 1.0F) const;

		// Member variables
		static Entity::ClassInfo m_sClass;
		// Articulated head.
		Attachable* m_pHead;
		// Ratio at which the head's rotation follows the aim angle
		float m_LookToAimRatio;
		// Foreground arm.
		Arm* m_pFGArm;
		// Background arm.
		Arm* m_pBGArm;
		// Foreground leg.
		Leg* m_pFGLeg;
		// Background leg.
		Leg* m_pBGLeg;
		// Limb AtomGroups.
		AtomGroup* m_pFGHandGroup;
		AtomGroup* m_pBGHandGroup;
		AtomGroup* m_pFGFootGroup;
		AtomGroup* m_BackupFGFootGroup;
		AtomGroup* m_pBGFootGroup;
		AtomGroup* m_BackupBGFootGroup;
		// The sound of the actor taking a step (think robot servo)
		SoundContainer* m_StrideSound;
		// Jetpack booster.
		AEJetpack* m_pJetpack;
		bool m_CanActivateBGItem; //!< A flag for whether or not the BG item is waiting to be activated separately. Used for dual-wielding. TODO: Should this be able to be toggled off per actor, device, or controller?
		bool m_TriggerPulled; //!< Internal flag for whether this AHuman is currently holding down the trigger of a HDFirearm. Used for dual-wielding.
		bool m_WaitingToReloadOffhand; //!< A flag for whether or not the offhand HeldDevice is waiting to be reloaded.
		// Blink timer
		Timer m_IconBlinkTimer;
		// Current upper body state.
		UpperBodyState m_ArmsState;
		// Current movement state.
		MovementState m_MoveState;
		// Whether the guy is currently lying down on the ground, rotational spring pulling him that way
		// This is engaged if the player first crouches (still upright spring), and then presses left/right
		// It is disengaged as soon as the crouch button/direction is released
		ProneState m_ProneState;
		// Timer for the going prone procedural animation
		Timer m_ProneTimer;
		// The maximum amount our walkpath can be shifted upwards to crouch and avoid ceilings above us
		float m_MaxWalkPathCrouchShift;
		// The maximum amount we will duck our head down to avoid obstacles above us.
		float m_MaxCrouchRotation;
		// The script-set forced crouching amount. 0.0 == fully standing, 1.0 == fully crouched, -1 == no override.
		float m_CrouchAmountOverride;
		// Limb paths for different movement states.
		// [0] is for the foreground limbs, and [1] is for BG.
		LimbPath m_Paths[2][MOVEMENTSTATECOUNT];
		std::array<float, MOVEMENTSTATECOUNT> m_RotAngleTargets; //!< An array of rot angle targets for different movement states.
		// Whether was aiming during the last frame too.
		bool m_Aiming;
		// Whether the BG Arm is helping with locomotion or not.
		bool m_ArmClimbing[2];
		// Whether a stride was taken this frame or not.
		bool m_StrideFrame = false;
		// Controls the start of leg synch.
		bool m_StrideStart;
		// Times the stride to see if it is taking too long and needs restart
		Timer m_StrideTimer;
		// For timing throws
		Timer m_ThrowTmr;
		// The duration it takes this AHuman to fully charge a throw.
		long m_ThrowPrepTime;
		Timer m_SharpAimRevertTimer; //!< For timing the transition from sharp aim back to regular aim.
		float m_FGArmFlailScalar; //!< The rate at which this AHuman's FG Arm follows the the bodily rotation. Best to keep this at 0 so it doesn't complicate aiming.
		float m_BGArmFlailScalar; //!< The rate at which this AHuman's BG Arm follows the the bodily rotation. Set to a negative value for a "counterweight" effect.
		Timer m_EquipHUDTimer; //!< Timer for showing the name of any newly equipped Device.
		std::array<Matrix, 2> m_WalkAngle; //!< An array of rot angle targets for different movement states.
		Vector m_WalkPathOffset;
		float m_ArmSwingRate; //!< Controls the rate at which this AHuman's Arms follow the movement of its Legs while they're not holding device(s).
		float m_DeviceArmSwayRate; //!< Controls the rate at which this AHuman's Arms follow the movement of its Legs while they're holding device(s). One-handed devices sway half as much as two-handed ones. Defaults to three quarters of Arm swing rate.

		////////////////
		// AI States

		enum DeviceHandlingState {
			STILL = 0,
			POINTING,
			SCANNING,
			AIMING,
			FIRING,
			THROWING,
			DIGGING
		};

		enum SweepState {
			NOSWEEP = 0,
			SWEEPINGUP,
			SWEEPUPPAUSE,
			SWEEPINGDOWN,
			SWEEPDOWNPAUSE
		};

		enum DigState {
			NOTDIGGING = 0,
			PREDIG,
			STARTDIG,
			TUNNELING,
			FINISHINGDIG,
			PAUSEDIGGER
		};

		enum JumpState {
			NOTJUMPING = 0,
			FORWARDJUMP,
			PREUPJUMP,
			UPJUMP,
			APEXJUMP,
			LANDJUMP
		};

		// What the AI is doing with its held devices
		DeviceHandlingState m_DeviceState;
		// What we are doing with a device sweeping
		SweepState m_SweepState;
		// The current digging state
		DigState m_DigState;
		// The current jumping state
		JumpState m_JumpState;
		// Jumping target, overshoot this and the jump is completed
		Vector m_JumpTarget;
		// Jumping left or right
		bool m_JumpingRight;
		// AI is crawling
		bool m_Crawling;
		// The position of the end of the current tunnel being dug. When it is reached, digging can stop.
		Vector m_DigTunnelEndPos;
		// The center angle (in rads) for the sweeping motion done duing scannign and digging
		float m_SweepCenterAimAngle;
		// The range to each direction of the center that the sweeping motion will be done in
		float m_SweepRange;
		// The absolute coordinates of the last detected gold deposits
		Vector m_DigTarget;
		// Timer for how long to be shooting at a seen enemy target
		Timer m_FireTimer;
		// Timer for how long to be shooting at a seen enemy target
		Timer m_SweepTimer;
		// Timer for how long to be patrolling in a direction
		Timer m_PatrolTimer;
		// Timer for how long to be firing the jetpack in a direction
		Timer m_JumpTimer;

#pragma region Event Handling
		/// Event listener to be run while this AHuman's PieMenu is opened.
		/// @param pieMenu The PieMenu this event listener needs to listen to. This will always be this' m_PieMenu and only exists for std::bind.
		/// @return An error return value signaling success or any particular failure. Anything below 0 is an error signal.
		int WhilePieMenuOpenListener(const PieMenu* pieMenu) override;
#pragma endregion

		/// Private member variable and method declarations
	private:
		/// Clears all the member variables of this AHuman, effectively
		/// resetting the members of this abstraction level only.
		void Clear();

		// Disallow the use of some implicit methods.
		AHuman(const AHuman& reference) = delete;
		AHuman& operator=(const AHuman& rhs) = delete;
	};

} // namespace RTE
