#pragma once

/// Header file for the HDFirearm class.
/// @author Daniel Tabar
/// data@datarealms.com
/// http://www.datarealms.com
/// Inclusions of header files
#include "HeldDevice.h"

namespace RTE {

	class Magazine;

	/// A firearm device that fires projectile MO's and discharges shell MO's.
	class HDFirearm : public HeldDevice {

		/// Public member variable, method and friend function declarations
	public:
		// Concrete allocation and cloning definitions
		EntityAllocation(HDFirearm);
		SerializableOverrideMethods;
		ClassInfoGetters;
		AddScriptFunctionNames(HeldDevice, "OnFire", "OnReload");

		/// Constructor method used to instantiate a HDFirearm object in system
		/// memory. Create() should be called before using the object.
		HDFirearm();

		/// Destructor method used to clean up a HDFirearm object before deletion
		/// from system memory.
		~HDFirearm() override;

		/// Makes the HDFirearm object ready for use.
		/// @return An error return value signaling sucess or any particular failure.
		/// Anything below 0 is an error signal.
		int Create() override;

		/// Creates a HDFirearm to be identical to another, by deep copy.
		/// @param reference A reference to the HDFirearm to deep copy.
		/// @return An error return value signaling sucess or any particular failure.
		/// Anything below 0 is an error signal.
		int Create(const HDFirearm& reference);

		/// Resets the entire HDFirearm, including its inherited members, to their
		/// default settings or values.
		void Reset() override {
			Clear();
			HeldDevice::Reset();
		}

		/// Destroys and resets (through Clear()) the SceneLayer object.
		/// @param notInherited Whether to only destroy the members defined in this derived class, or (default: false)
		/// to destroy all inherited members also.
		void Destroy(bool notInherited = false) override;

		/// Gets reload end offset, in ms. This is how early the ReloadEnd
		/// sound is played compared to actual end of reload.
		/// @return The reload end offset, in ms.
		int GetReloadEndOffset() const { return m_ReloadEndOffset; }

		/// Sets reload end offset, in ms. This is how early the ReloadEnd
		/// sound is played compared to actual end of reload.
		/// @param newRate The new reload end offset, in ms.
		void SetReloadEndOffset(int newRate) { m_ReloadEndOffset = newRate; }

		/// Gets the rate of fire of this. This applies even if semi-auto. it
		/// limits how quickly a new round can be fired after the last.
		/// @return The rate of fire, in rounds per min.
		int GetRateOfFire() const { return m_RateOfFire; }

		/// Sets the rate of fire of this. This applies even if semi-auto. it
		/// limits how quickly a new round can be fired after the last.
		/// @param newRate The new rate of fire, in rounds per min.
		void SetRateOfFire(int newRate) { m_RateOfFire = newRate; }

		/// Gets the minimum time in between shots, in MS.
		/// @return The minimum time in between shots, in MS.
		double GetMSPerRound() const { return 60000.0 / static_cast<double>(m_RateOfFire); }

		/// Gets the Magazine of this HDFirearm.
		/// @return A pointer to Magazine of this HDFirearm. Ownership is NOT transferred!
		Magazine* GetMagazine() const { return m_pMagazine; }

		/// Sets the Magazine for this HDFirearm. Ownership IS transferred!
		/// @param newMagazine The new Magazine to use.
		void SetMagazine(Magazine* newMagazine);

		/// Gets the flash of this HDFirearm.
		/// @return A pointer to flash of this HDFirearm. Ownership is NOT transferred!
		Attachable* GetFlash() const { return m_pFlash; }

		/// Sets the flash for this HDFirearm. Ownership IS transferred!
		/// @param newTurret The new flash to use.
		void SetFlash(Attachable* newFlash);

		/// Gets the preset name of the next Magazine that will be loaded into this gun.
		/// @return The preset name of the next Magazine that will be loaded into this gun.
		std::string GetNextMagazineName() const;

		/// Sets the Preset name of the next Magazine that will be loaded into
		/// this gun. This changes all future mags that will be reloaded.
		/// @param magName The preset name of the new Magazine to load into this from now on.
		/// @return Whether the specified magazine was found and successfully prepared.
		bool SetNextMagazineName(std::string magName);

		/// Gets the number of rounds still in the loaded magazine. Negative value
		/// means infinite ammo.
		/// @return An int with the number of rounds in the magazine currently in this
		/// HDFirearm. Negative means infinite ammo.
		int GetRoundInMagCount() const;

		/// Gets the maximum RoundCount a Magazine of this HDFirearm can hold.
		/// If there is no Magazine, it gets the RoundCount of the reference Magazine.
		/// @return An int with the maximum RoundCount the magazine or magazine reference of this HDFirearm can hold.
		int GetRoundInMagCapacity() const;

		/// Gets the delay before firing.
		/// @return An int with the activation delay in ms.
		int GetActivationDelay() const { return m_ActivationDelay; };

		/// Sets the delay before firing.
		/// @param delay An int with the activation delay in ms.
		void SetActivationDelay(int delay) { m_ActivationDelay = delay; };

		/// Gets the delay between release of activation and another can be started.
		/// @return An int with the delay in ms.
		int GetDeactivationDelay() const { return m_DeactivationDelay; };

		/// Sets the delay between release of activation and another can be started.
		/// @param delay An int with the delay in ms.
		void SetDeactivationDelay(int delay) { m_DeactivationDelay = delay; };

		/// Gets the base time this HDFirearm takes to reload, in milliseconds.
		/// @return The base time this HeldDevice takes to reload, in milliseconds.
		int GetBaseReloadTime() const { return m_BaseReloadTime; };

		/// Sets the base time this HDFirearm takes to reload, in milliseconds.
		/// @param delay The base time this HDFirearm should take to reload, in milliseconds.
		void SetBaseReloadTime(int newReloadTime) {
			m_BaseReloadTime = newReloadTime;
			CorrectReloadTimerForSupportAvailable();
		};

		/// Gets how long this HDFirearm currently takes to reload, in milliseconds.
		/// @return How long this HDFirearm currently takes to reload, in milliseconds.
		int GetReloadTime() const { return m_ReloadTmr.GetSimTimeLimitMS() <= 0 ? m_BaseReloadTime : static_cast<int>(std::floor(m_ReloadTmr.GetSimTimeLimitMS())); };

		/// Gets whether or not this HDFirearm allows dual-reload, i.e. if it's one-handed and dual-wieldable, it can reload at the same time as another weapon that also allows dual-reload.
		/// @return Whether or not this HDFirearm allows dual-reload.
		bool IsDualReloadable() const { return m_DualReloadable; }

		/// Sets whether or not this HDFirearm allows dual-reloading.
		/// @param newDualReloadable The new value for whether or not this HDFirearm should allow dual-reloading.
		void SetDualReloadable(bool newDualReloadable) { m_DualReloadable = newDualReloadable; }

		/// Gets the multiplier to be applied to reload time when this HDFirearm is being reloaded one-handed.
		/// @return The multiplier to be applied to reload time when this HDFirearm is being reloaded one-handed.
		float GetOneHandedReloadTimeMultiplier() const { return m_OneHandedReloadTimeMultiplier; }

		/// Sets the multiplier to be applied to reload time when this HDFirearm is being reloaded one-handed.
		/// @param newDualReloadTimeMultiplier The new multiplier to be applied to reload time when this HDFirearm is being reloaded one-handed.
		void SetOneHandedReloadTimeMultiplier(float newOneHandedReloadTimeMultiplier) { m_OneHandedReloadTimeMultiplier = newOneHandedReloadTimeMultiplier; }

		/// Gets the reload angle this HDFirearm will use when support is available.
		/// @return The reload angle this HDFirearm will use when support is available, in radians.
		float GetReloadAngle() const { return m_ReloadAngle; }

		/// Sets the reload angle this HDFirearm should use when support is available.
		/// @param newReloadAngle The new reload angle this HDFirearm should use when support is available.
		void SetReloadAngle(float newReloadAngle) { m_ReloadAngle = newReloadAngle; }

		/// Gets the reload angle this HDFirearm will use when support is not available.
		/// @return The reload angle this HDFirearm will use when support is not available, in radians.
		float GetOneHandedReloadAngle() const { return m_OneHandedReloadAngle; }

		/// Sets the reload angle this HDFirearm should use when support is not available.
		/// @param newOneHandedReloadAngle The new reload angle this HDFirearm should use when support is not available.
		void SetOneHandedReloadAngle(float newOneHandedReloadAngle) { m_OneHandedReloadAngle = newOneHandedReloadAngle; }

		/// Gets the reload angle this HDFirearm is currently using, based on whether or not support is available.
		/// @return The current reload angle of this HDFirearm, in radians.
		float GetCurrentReloadAngle() const { return m_SupportAvailable ? m_ReloadAngle : m_OneHandedReloadAngle; }

		/// Gets the range of normal shaking of entire weapon.
		/// @return A float with the range in degrees.
		float GetShakeRange() const { return m_ShakeRange; };

		/// Sets the range of normal shaking of entire weapon.
		/// @param range A float with the range in degrees.
		void SetShakeRange(float range) { m_ShakeRange = range; };

		/// Gets the range of shaking of entire weapon during sharp aiming.
		/// @return A float with the range in degrees.
		float GetSharpShakeRange() const { return m_SharpShakeRange; };

		/// Sets the range of shaking of entire weapon during sharp aiming.
		/// @param range A float with the range in degrees.
		void SetSharpShakeRange(float range) { m_SharpShakeRange = range; };

		/// Gets the factor for how much more weapon shakes if it isn't supported
		/// by a second hand.
		/// @return A float with the factor.
		float GetNoSupportFactor() const { return m_NoSupportFactor; };

		/// Sets the factor for how much more weapon shakes if it isn't supported
		/// by a second hand.
		/// @param factor A float with the factor.
		void SetNoSupportFactor(float factor) { m_NoSupportFactor = factor; };

		/// Gets the range of spread angle of fired particles, in one direction.
		/// @return A float with the range in degrees.
		float GetParticleSpreadRange() const { return m_ParticleSpreadRange; };

		/// Sets the range of spread angle of fired particles, in one direction.
		/// @param range A float with the range in degrees.
		void SetParticleSpreadRange(float range) { m_ParticleSpreadRange = range; };

		/// Gets the random velocity variation scalar at which this HDFirearm's shell is to be ejected.
		/// @return A float with the scalar value.
		float GetShellVelVariation() const { return m_ShellVelVariation; }

		/// Sets the random velocity variation scalar at which this HDFirearm's shell is to be ejected.
		/// @param newValue The new velocity variation scalar.
		void SetShellVelVariation(float newVariation) { m_ShellVelVariation = newVariation; }

		/// Sets the stiffness scalar of the joint of this HDFirearm. Unlike Attachable::SetJointStiffness, there are no limitations on this value.
		/// 1.0 means impulse forces on this attachable will be transferred to the parent with 100% strength, 0 means they will not transfer at all, negative values will apply negative force, which may behave oddly.
		/// @param jointStiffness A float describing the normalized stiffness scalar of this Attachable's joint.
		void SetJointStiffness(float jointStiffness) override { m_JointStiffness = jointStiffness; }

		/// Gets the velocity the AI use when aiming this weapon.
		/// @return A float with the velocity in m/s.
		float GetAIFireVel();

		/// Gets the bullet life time the AI use when aiming this weapon.
		/// @return A float with the life time in ms.
		unsigned long GetAIBulletLifeTime();

		/// Gets the bullet acceleration scalar the AI use when aiming this weapon.
		/// @return A float with the scalar.
		float GetBulletAccScalar();

		/// Gets the blast radius the AI use when aiming this weapon.
		/// @return A float with the blast radius in pixels.
		float GetAIBlastRadius() const;

		/// Gets how much material the projectiles from this weapon can destory.
		/// @return A float with the material strength.
		float GetAIPenetration() const;

		/// Estimates how close the projectiles from two weapons will land.
		/// @param pWeapon A HDFirearm pointer to compare with.
		/// @return A float with the distance in pixels.
		float CompareTrajectories(HDFirearm* pWeapon);

		/// Gets the absolute position of the magazine or other equivalent point of
		/// this.
		/// @return A vector describing the absolute world coordinates for the magazine
		/// attachment point of this
		Vector GetMagazinePos() const override;

		/// Gets the absolute position of the muzzle or other equivalent point of
		/// this.
		/// @return A vector describing the absolute world coordinates for the muzzle point
		/// of this
		Vector GetMuzzlePos() const override;

		/// Gets the unrotated relative offset from the position to the muzzle or
		/// other equivalent point of this.
		/// @return A unrotated vector describing the relative for the muzzle point of
		/// this from this' position.
		Vector GetMuzzleOffset() const override { return m_MuzzleOff; }

		/// Sets the unrotated relative offset from the position to the muzzle or
		/// other equivalent point of this.
		/// @param newOffset New offset value.
		void SetMuzzleOffset(Vector newOffset) override { m_MuzzleOff = newOffset; }

		/// Gets the absolute position of the Shell ejection point.
		/// @return A vector describing the absolute world coordinates for the Shell
		/// ejection point of this
		Vector GetEjectionPos() const;

		/// Gets the unrotated relative offset from the position to the Shell ejection point.
		/// @return A unrotated vector describing the relative for the Shell ejection point of
		/// this from this' position.
		Vector GetEjectionOffset() const { return m_EjectOff; }

		/// Sets the unrotated relative offset from the position to the Shell ejection point.
		/// @param newOffset New offset value.
		void SetEjectionOffset(Vector newOffset) { m_EjectOff = newOffset; }

		/// Gets this HDFirearm's pre fire sound. Ownership is NOT transferred!
		/// @return The SoundContainer for this HDFirearm's pre fire sound.
		SoundContainer* GetPreFireSound() const { return m_PreFireSound; }

		/// Sets this HDFirearm's pre fire sound. Ownership IS transferred!
		/// @param newSound The new SoundContainer for this HDFirearm's pre fire sound.
		void SetPreFireSound(SoundContainer* newSound) { m_PreFireSound = newSound; }

		/// Gets this HDFirearm's fire sound. Ownership is NOT transferred!
		/// @return The SoundContainer for this HDFirearm's fire sound.
		SoundContainer* GetFireSound() const { return m_FireSound; }

		/// Sets this HDFirearm's fire sound. Ownership IS transferred!
		/// @param newSound The new SoundContainer for this HDFirearm's fire sound.
		void SetFireSound(SoundContainer* newSound) { m_FireSound = newSound; }

		/// Gets this HDFirearm's fire echo sound. Ownership is NOT transferred!
		/// @return The SoundContainer for this HDFirearm's fire echo sound.
		SoundContainer* GetFireEchoSound() const { return m_FireEchoSound; }

		/// Sets this HDFirearm's fire echo sound. Ownership IS transferred!
		/// @param newSound The new SoundContainer for this HDFirearm's fire echo sound.
		void SetFireEchoSound(SoundContainer* newSound) { m_FireEchoSound = newSound; }

		/// Gets this HDFirearm's active sound. Ownership is NOT transferred!
		/// @return The SoundContainer for this HDFirearm's active sound.
		SoundContainer* GetActiveSound() const { return m_ActiveSound; }

		/// Sets this HDFirearm's active sound. Ownership IS transferred!
		/// @param newSound The new SoundContainer for this HDFirearm's active sound.
		void SetActiveSound(SoundContainer* newSound) { m_ActiveSound = newSound; }

		/// Gets this HDFirearm's deactivation sound. Ownership is NOT transferred!
		/// @return The SoundContainer for this HDFirearm's deactivation sound.
		SoundContainer* GetDeactivationSound() const { return m_DeactivationSound; }

		/// Sets this HDFirearm's deactivation sound. Ownership IS transferred!
		/// @param newSound The new SoundContainer for this HDFirearm's deactivation sound.
		void SetDeactivationSound(SoundContainer* newSound) { m_DeactivationSound = newSound; }

		/// Gets this HDFirearm's empty sound. Ownership is NOT transferred!
		/// @return The SoundContainer for this HDFirearm's empty sound.
		SoundContainer* GetEmptySound() const { return m_EmptySound; }

		/// Sets this HDFirearm's empty sound. Ownership IS transferred!
		/// @param newSound The new SoundContainer for this HDFirearm's empty sound.
		void SetEmptySound(SoundContainer* newSound) { m_EmptySound = newSound; }

		/// Gets this HDFirearm's reload start sound. Ownership is NOT transferred!
		/// @return The SoundContainer for this HDFirearm's reload start sound.
		SoundContainer* GetReloadStartSound() const { return m_ReloadStartSound; }

		/// Sets this HDFirearm's reload start sound. Ownership IS transferred!
		/// @param newSound The new SoundContainer for this HDFirearm's reload start sound.
		void SetReloadStartSound(SoundContainer* newSound) { m_ReloadStartSound = newSound; }

		/// Gets this HDFirearm's reload end sound. Ownership is NOT transferred!
		/// @return The SoundContainer for this HDFirearm's reload end sound.
		SoundContainer* GetReloadEndSound() const { return m_ReloadEndSound; }

		/// Sets this HDFirearm's reload end sound. Ownership IS transferred!
		/// @param newSound The new SoundContainer for this HDFirearm's reload end sound.
		void SetReloadEndSound(SoundContainer* newSound) { m_ReloadEndSound = newSound; }

		/// Resest all the timers used by this. Can be emitters, etc. This is to
		/// prevent backed up emissions to come out all at once while this has been
		/// held dormant in an inventory.
		void ResetAllTimers() override {
			HeldDevice::ResetAllTimers();
			m_LastFireTmr.Reset();
			m_ReloadTmr.Reset();
		}

		/// Gets this HDFirearm's reload progress as a scalar from 0 to 1.
		/// @return The reload progress as a scalar from 0 to 1.
		float GetReloadProgress() const { return IsReloading() && m_BaseReloadTime > 0 ? static_cast<float>(m_ReloadTmr.SimTimeLimitProgress()) : 1.0F; }

		/// Does the calculations necessary to detect whether this HDFirearm is at rest or not. IsAtRest() retrieves the answer.
		void RestDetection() override;

		/// Activates one of this HDFirearm's features. Analogous to 'pulling
		/// the trigger'.
		void Activate() override;

		/// Deactivates one of this HDFirearm's features. Analogous to 'releasing
		/// the trigger'.
		void Deactivate() override;

		/// Aborts playing of active sound no matter what. Used to silence spinning
		/// weapons when weapons swapped
		void StopActivationSound();

		/// Throws out the currently used Magazine, if any, and puts in a new one
		/// after the reload delay is up.
		void Reload() override;

		/// Tells whether the device is curtrently being reloaded.
		/// @return Whetehr being reloaded.
		bool IsReloading() const override { return m_Reloading; }

		/// Tells whether the device just finished reloading this frame.
		/// @return Whether just done reloading this frame.
		bool DoneReloading() const override { return m_DoneReloading; }

		/// Tells whether the device is curtrently in need of being reloaded.
		/// @return Whetehr in need of reloading (ie not full).
		bool NeedsReloading() const override;

		/// Tells whether the device is curtrently full and reloading won't have
		/// any effect.
		/// @return Whetehr magazine is full or not.
		bool IsFull() const override;

		bool IsEmpty() const override;

		/// Tells whether the device is fully automatic or not.
		/// @return Whether the player can hold down fire and this will fire repeatedly.
		bool IsFullAuto() const { return m_FullAuto; }

		/// Gets whether this HDFirearm is set to be reloadable or not.
		/// @return Whether this HDFirearm is reloadable.
		bool IsReloadable() const { return m_Reloadable; }

		/// Sets whether this HDFirearm is reloadable or not and halts the reloading process.
		/// @param isReloadable Whether this HDFirearm is reloadable.
		void SetReloadable(bool isReloadable) {
			m_Reloadable = isReloadable;
			m_Reloading = m_Reloading && m_Reloadable;
		}

		/// Sets whether the device is fully automatic or not.
		/// @param newValue New value.
		void SetFullAuto(bool newValue) { m_FullAuto = newValue; }

		/// Updates this MovableObject. Supposed to be done every frame.
		void Update() override;

		/// Draws this HDFirearm's current graphical representation to a
		/// BITMAP of choice.
		/// @param pTargetBitmap A pointer to a BITMAP to draw on.
		/// @param targetPos The absolute position of the target bitmap's upper left corner in the Scene. (default: Vector())
		/// @param mode In which mode to draw in. See the DrawMode enumeration for the modes. (default: g_DrawColor)
		/// @param onlyPhysical Whether to not draw any extra 'ghost' items of this MovableObject, (default: false)
		/// indicator arrows or hovering HUD text and so on.
		void Draw(BITMAP* pTargetBitmap, const Vector& targetPos = Vector(), DrawMode mode = g_DrawColor, bool onlyPhysical = false) const override;

		/// Draws an aiming aid in front of this HeldDevice.
		/// @param pTargetBitmap A pointer to a BITMAP to draw on.
		/// @param targetPos The absolute position of the target bitmap's upper left corner in the Scene. (default: Vector())
		/// @param whichScreen Which player's screen this is being drawn to. May affect what HUD elements (default: 0)
		/// get drawn etc.
		void DrawHUD(BITMAP* pTargetBitmap, const Vector& targetPos = Vector(), int whichScreen = 0, bool playerControlled = false) override;

		/// Estimates what material strength one round in the magazine can destroy.
		/// @return The maximum material strength the regular or the tracer round can destroy.
		float EstimateDigStrength() const;

		/// Whether at least one round has already been	fired during the current activation.
		/// @return Returns true if at least one round has already been fired during the current activation.
		bool FiredOnce() const { return m_FiredOnce; }

		/// Whether at least one round has already been fired during the current frame.
		/// @return Returns true at least one round has already been fired during the current frame.
		bool FiredFrame() const { return m_FireFrame; }

		/// Gets whether this HDFirearm is ready to be fired.
		/// @return Whether this HDFirearm is ready to pop another Round.
		bool CanFire() const { return m_LastFireTmr.IsPastSimMS(GetMSPerRound()); }

		/// Gets whether this HDFirearm is halfway to be fired. Used for evenly spacing out dual-wielded fire.
		/// @return Whether this HDFirearm is halfway to pop another Round.
		bool HalfwayToNextRound() const { return m_LastFireTmr.IsPastSimMS(GetMSPerRound() / 2.0); }

		/// How many rounds were fired during this frame.
		/// @return Returns the number of rounds fired during this frame.
		int RoundsFired() const { return m_RoundsFired; }

		/// If true then the m_Frame property is not changed bye the Update function
		/// @return Whether this HDFirearm is animated manually.
		bool IsAnimatedManually() const { return m_IsAnimatedManually; }

		/// Sets Whether this HDFirearm is animated manually.
		/// @param newValue Manual animation flag value.
		void SetAnimatedManually(bool newValue) { m_IsAnimatedManually = newValue; }

		/// Protected member variable and method declarations
	protected:
		/// Sets this Attachable's parent MOSRotating, and also sets its Team based on its parent and, if the Attachable is set to collide, adds/removes Atoms to its new/old parent.
		/// Additionally, sets this HDFirearm as not firing or reloading, and resets its reload timer.
		/// @param newParent A pointer to the MOSRotating to set as the new parent. Ownership is NOT transferred!
		void SetParent(MOSRotating* newParent) override {
			HeldDevice::SetParent(newParent);
			Deactivate();
			m_Reloading = false;
			m_ReloadTmr.Reset();
		}

		// Member variables.
		static Entity::ClassInfo m_sClass;

		// The mag reference which all current mags are generated from.
		// NOT owned by this, owned by PresetMan
		const Magazine* m_pMagazineReference;
		// Magazine MovableObject. Owned
		Magazine* m_pMagazine;
		// Muzzle Flash Attachable. Owned
		Attachable* m_pFlash;

		SoundContainer* m_PreFireSound; //!< The sound this HDFirearm should play before it starts firing. Distinct from activation sound in that it will play exactly once per trigger pull and not pitch up.
		// The audio of this FireArm being fired.
		SoundContainer* m_FireSound;
		SoundContainer* m_FireEchoSound; //!< The audio that is played as the echo for the gun. Each shot will restart this sound, so it doesn't ever overlap.
		// The audio that is played immediately upon activation, but perhaps before actual first firing, if there's a pre-delay
		SoundContainer* m_ActiveSound;
		// The audio that is played immediately upon cease of activation
		SoundContainer* m_DeactivationSound;
		// The audio of this FireArm being fired empty.
		SoundContainer* m_EmptySound;
		// The audio of this FireArm being reloaded.
		SoundContainer* m_ReloadStartSound;
		SoundContainer* m_ReloadEndSound;
		// The offset of how long before the reload finishes the sound plays
		float m_ReloadEndOffset;
		// Whether or not the end-of-relaod sound has already been played or not.
		bool m_HasPlayedEndReloadSound;

		// Rate of fire, in rounds per min.
		// If 0, firearm is semi-automatic (ie only one discharge per activation).
		int m_RateOfFire;
		// Delay between activation and full round output is achieved, in ms
		int m_ActivationDelay;
		// Delay between release of activation and another can be started, in ms
		int m_DeactivationDelay;
		// Reloading or not
		bool m_Reloading;
		// Just done reloading this frame
		bool m_DoneReloading;
		// Base reload time in millisecs.
		int m_BaseReloadTime;
		// Whether this HDFirearm is full or semi-auto.
		bool m_FullAuto;
		// Whether particles fired from this HDFirearm will ignore hits with itself,
		// and the root parent of this HDFirearm, regardless if they are set to hit MOs.
		bool m_FireIgnoresThis;
		bool m_Reloadable; //!< Whether this HDFirearm is reloadable by normal means.
		float m_OneHandedReloadTimeMultiplier; //!< The multiplier for how long this weapon takes to reload when being used one-handed. Only relevant for one-handed weapons.
		bool m_DualReloadable; //!< Whether or not this weapon can be dual-reloaded, i.e. both guns can reload at once instead of having to wait til the other dual-wielded gun isn't being reloaded. Only relevant for one-handed weapons.
		float m_ReloadAngle; //!< The angle offset for the default reload animation, in radians.
		float m_OneHandedReloadAngle; //!< The angle offset for one-handed reload animation, in radians.

		// Timer for timing how long ago the last round was fired.
		Timer m_LastFireTmr;
		// Timer for timing reload times.
		Timer m_ReloadTmr;

		// The point from where the projectiles appear.
		Vector m_MuzzleOff;
		// The point from where the discharged shells appear.
		Vector m_EjectOff;
		// Offset to magazine.
		Vector m_MagOff;
		// Range of normal shaking of entire weapon.
		float m_ShakeRange;
		// Range of shaking of entire weapon during sharp aiming.
		float m_SharpShakeRange;
		// Factor for how much more weapon shakes if it isn't supported by a second hand.
		float m_NoSupportFactor;
		// Range of spread angle of fired particles, in one direction
		float m_ParticleSpreadRange;
		// Angle in which shells are ejected relative to this weapon
		float m_ShellEjectAngle;
		// Range of spread angle of ejected shells, in one direction
		float m_ShellSpreadRange;
		// Range of spread in ang vel of ejected shells, in one direction
		float m_ShellAngVelRange;
		float m_ShellVelVariation; //!< The velocity variation scalar of ejected shells.
		// The amount of screenshake that recoil causes
		float m_RecoilScreenShakeAmount;
		// The muzzle velocity the AI use when aiming this weapon
		float m_AIFireVel;
		// The bullet life time the AI use when aiming this weapon
		unsigned long m_AIBulletLifeTime;
		// The bullet acc scalar the AI use when aiming this weapon
		float m_AIBulletAccScalar;

		// Whether at least one round has already been
		// fired during the current activation.
		bool m_FiredOnce;
		// Whether at least one round has already been
		// fired during the current frame.
		bool m_FireFrame;
		// Whether at least one round was fired during the last frame
		bool m_FiredLastFrame;
		// Whether, if this HDFireArm is empty, pin has already clicked once during
		// current acticvation.
		bool m_AlreadyClicked;
		// How many rounds were fired during this frame
		int m_RoundsFired;
		// If true m_Frame is not changed during an update hence the animation
		// is done by external Lua code
		bool m_IsAnimatedManually;

		bool m_LegacyCompatibilityRoundsAlwaysFireUnflipped; //<! Legacy compatibility flag to make it so rounds don't flip with the gun. Useful for old mods with things like missiles that accounted for the old code that didn't flip them properly.

		/* TODO
		    // Path the the script file that contains the ballistic solution function of this
		    std::string m_BallisticScriptPath;
		    // The name of the function that
		    std::string m_BallisticScriptFunction;
		*/

		/// Private member variable and method declarations
	private:
		/// Ensures the reload Timer's time limit is set accordingly, based on whether the HDFirearm has support available.
		void CorrectReloadTimerForSupportAvailable() { m_ReloadTmr.SetSimTimeLimitMS(static_cast<double>(static_cast<float>(m_BaseReloadTime) * (m_SupportAvailable ? 1.0F : m_OneHandedReloadTimeMultiplier))); }

		/// Clears all the member variables of this HDFirearm, effectively
		/// resetting the members of this abstraction level only.
		void Clear();

		// Disallow the use of some implicit methods.
		HDFirearm(const HDFirearm& reference) = delete;
		HDFirearm& operator=(const HDFirearm& rhs) = delete;
	};

} // namespace RTE
