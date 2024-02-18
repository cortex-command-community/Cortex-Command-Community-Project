#pragma once

/// Header file for the ACrab class.
/// @author Daniel Tabar
/// data@datarealms.com
/// http://www.datarealms.com
/// Inclusions of header files
#include "Actor.h"
#include "LimbPath.h"
#include "Leg.h"

struct BITMAP;

namespace RTE {

	class Turret;
	class AEJetpack;

	/// A crab-like actor with four legs.
	class ACrab : public Actor {
		friend struct EntityLuaBindings;

		enum Side {
			LEFTSIDE = 0,
			RIGHTSIDE,
			SIDECOUNT
		};

		enum Layer {
			FGROUND = 0,
			BGROUND,
			LAYERCOUNT
		};

		/// Public member variable, method and friend function declarations
	public:
		// Concrete allocation and cloning definitions
		EntityAllocation(ACrab);
		AddScriptFunctionNames(Actor, "OnStride");
		SerializableOverrideMethods;
		ClassInfoGetters;
		DefaultPieMenuNameGetter(HasObjectInGroup("Turrets") ? "Default Turret Pie Menu" : "Default Crab Pie Menu");

		/// Constructor method used to instantiate a ACrab object in system
		/// memory. Create() should be called before using the object.
		ACrab();

		/// Destructor method used to clean up a ACrab object before deletion
		/// from system memory.
		~ACrab() override;

		/// Makes the ACrab object ready for use.
		/// @return An error return value signaling sucess or any particular failure.
		/// Anything below 0 is an error signal.
		int Create() override;

		/// Creates a ACrab to be identical to another, by deep copy.
		/// @param reference A reference to the ACrab to deep copy.
		/// @return An error return value signaling sucess or any particular failure.
		/// Anything below 0 is an error signal.
		int Create(const ACrab& reference);

		/// Resets the entire ACrab, including its inherited members, to their
		/// default settings or values.
		void Reset() override {
			Clear();
			Actor::Reset();
		}

		/// Destroys and resets (through Clear()) the SceneLayer object.
		/// @param notInherited Whether to only destroy the members defined in this derived class, or (default: false)
		/// to destroy all inherited members also.
		void Destroy(bool notInherited = false) override;

		/// Gets the absoltue position of this' eye, or equivalent, where look
		/// vector starts from.
		/// @return A Vector with the absolute position of this' eye or view point.
		Vector GetEyePos() const override;

		/// Gets the Turret of this ACrab.
		/// @return A pointer to Turret of this ACrab. Ownership is NOT transferred!
		Turret* GetTurret() const { return m_pTurret; }

		/// Sets the Turret for this ACrab. Ownership IS transferred!
		/// @param newTurret The new Turret to use.
		void SetTurret(Turret* newTurret);

		/// Gets the jetpack of this ACrab.
		/// @return A pointer to the jetpack of this ACrab. Ownership is NOT transferred!
		AEJetpack* GetJetpack() const { return m_pJetpack; }

		/// Sets the jetpack for this ACrab. Ownership IS Transferred!
		/// @param newJetpack The new jetpack to use.
		void SetJetpack(AEJetpack* newJetpack);

		/// Gets the left foreground Leg of this ACrab.
		/// @return A pointer to the left foreground Leg of this ACrab. Ownership is NOT transferred!
		Leg* GetLeftFGLeg() const { return m_pLFGLeg; }

		/// Sets the left foreground Leg for this ACrab. Ownership IS transferred!
		/// @param newLeg The new Leg to use.
		void SetLeftFGLeg(Leg* newLeg);

		/// Gets the left background Leg of this ACrab.
		/// @return A pointer to the left background Leg of this ACrab. Ownership is NOT transferred!
		Leg* GetLeftBGLeg() const { return m_pLBGLeg; }

		/// Sets the left background Leg for this ACrab. Ownership IS transferred!
		/// @param newLeg The new Leg to use.
		void SetLeftBGLeg(Leg* newLeg);

		/// Gets the right foreground Leg of this ACrab.
		/// @return A pointer to the right foreground Leg of this ACrab. Ownership is NOT transferred!
		Leg* GetRightFGLeg() const { return m_pRFGLeg; }

		/// Sets the right foreground Leg for this ACrab. Ownership IS transferred!
		/// @param newLeg The new Leg to use.
		void SetRightFGLeg(Leg* newLeg);

		/// Gets the right BG Leg of this ACrab.
		/// @return A pointer to the right background Leg of this ACrab. Ownership is NOT transferred!
		Leg* GetRightBGLeg() const { return m_pRBGLeg; }

		/// Sets the right background Leg for this ACrab. Ownership IS transferred!
		/// @param newLeg The new Leg to use.
		void SetRightBGLeg(Leg* newLeg);

		/// Tries to handle the activated PieSlice in this object's PieMenu, if there is one, based on its SliceType.
		/// @param pieSliceType The SliceType of the PieSlice being handled.
		/// @return Whether or not the activated PieSlice SliceType was able to be handled.
		bool HandlePieCommand(PieSliceType pieSliceType) override;

		/// Returns whatever is equipped in the turret, if anything. OWNERSHIP IS NOT TRANSFERRED!
		/// @return The currently equipped item, if any.
		MovableObject* GetEquippedItem() const;

		/// Indicates whether the currently held HDFirearm's is ready for use, and has
		/// ammo etc.
		/// @return Whether a currently HDFirearm (if any) is ready for use.
		bool FirearmIsReady() const;

		/// Indicates whether the currently held HDFirearm's is out of ammo.
		/// @return Whether a currently HDFirearm (if any) is out of ammo.
		bool FirearmIsEmpty() const;

		/// Indicates whether the currently held HDFirearm's is almost out of ammo.
		/// @return Whether a currently HDFirearm (if any) has less than half of ammo left.
		bool FirearmNeedsReload() const;

		/// Gets whether or not all of this ACrab's Turret's HDFirearms are full.
		/// @return Whether or not all of this ACrab's Turret's HDFirearms are full. Will return true if there is no Turret or no HDFirearms.
		bool FirearmsAreFull() const;

		/// Indicates whether the currently held HDFirearm's is semi or full auto.
		/// @return Whether a currently HDFirearm (if any) is a semi auto device.
		bool FirearmIsSemiAuto() const;

		/// Returns the currently held device's delay between pulling the trigger
		/// and activating.
		/// @return Delay in ms or zero if not a HDFirearm.
		int FirearmActivationDelay() const;

		/// Reloads the currently held firearms, if any.
		void ReloadFirearms();

		/// Tells whether a point on the scene is within close range of the currently
		/// used device and aiming status, if applicable.
		/// @param point A Vector witht he aboslute coordinates of a point to check.
		/// @return Whether the point is within close range of this.
		bool IsWithinRange(Vector& point) const override;

		/// Casts an unseen-revealing ray in the direction of where this is facing.
		/// @param FOVSpread The degree angle to deviate from the current view point in the ray
		/// casting. A random ray will be chosen out of this +-range.
		/// @param range The range, in pixels, beyond the actors sharp aim that the ray will have.
		/// @return Whether any unseen pixels were revealed by this look.
		bool Look(float FOVSpread, float range) override;

		/// Casts an MO detecting ray in the direction of where the head is looking
		/// at the time. Factors including head rotation, sharp aim mode, and
		/// other variables determine how this ray is cast.
		/// @param FOVSpread The degree angle to deviate from the current view point in the ray (default: 45)
		/// casting. A random ray will be chosen out of this +-range.
		/// @param ignoreMaterial A specific material ID to ignore (see through) (default: 0)
		/// @param ignoreAllTerrain Whether to ignore all terrain or not (true means 'x-ray vision'). (default: false)
		/// @return A pointer to the MO seen while looking.
		MovableObject* LookForMOs(float FOVSpread = 45, unsigned char ignoreMaterial = 0, bool ignoreAllTerrain = false);

		/// Gets the GUI representation of this ACrab, only defaulting to its Turret or body if no GraphicalIcon has been defined.
		/// @return The graphical representation of this ACrab as a BITMAP.
		BITMAP* GetGraphicalIcon() const override;

		/// Gets whether this ACrab has just taken a stride this frame.
		/// @return Whether this ACrab has taken a stride this frame or not.
		bool StrideFrame() const { return m_StrideFrame; }

		/// Update called prior to controller update. Ugly hack. Supposed to be done every frame.
		void PreControllerUpdate() override;

		/// Updates this MovableObject. Supposed to be done every frame.
		void Update() override;

		/// Draws this ACrab's current graphical representation to a
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

		/// Gets the LimbPath corresponding to the passed in Side, Layer and MovementState values.
		/// @param side Whether to get the left or right side.
		/// @param layer Whether to get foreground or background LimbPath.
		/// @param movementState Which movement state to get the LimbPath for.
		/// @return The LimbPath corresponding to the passed in Layer and MovementState values.
		LimbPath* GetLimbPath(Side side, Layer layer, MovementState movementState) { return &m_Paths[side][layer][movementState]; }

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

		/// Gets this ACrab's stride sound. Ownership is NOT transferred!
		/// @return The SoundContainer for this ACrab's stride sound.
		SoundContainer* GetStrideSound() const { return m_StrideSound; }

		/// Sets this ACrab's stride sound. Ownership IS transferred!
		/// @param newSound The new SoundContainer for this ACrab's stride sound.
		void SetStrideSound(SoundContainer* newSound) { m_StrideSound = newSound; }

		/// Gets the upper limit of this ACrab's aim range.
		/// @return The upper limit of this ACrab's aim range.
		float GetAimRangeUpperLimit() const { return m_AimRangeUpperLimit; }

		/// Sets the upper limit of this ACrab's aim range.
		/// @param aimRangeUpperLimit The new upper limit of this ACrab's aim range.
		void SetAimRangeUpperLimit(float aimRangeUpperLimit) { m_AimRangeUpperLimit = aimRangeUpperLimit; }

		/// Gets the lower limit of this ACrab's aim range.
		/// @return The lower limit of this ACrab's aim range.
		float GetAimRangeLowerLimit() const { return m_AimRangeLowerLimit; }

		/// Sets the lower limit of this ACrab's aim range.
		/// @param aimRangeLowerLimit The new lower limit of this ACrab's aim range.
		void SetAimRangeLowerLimit(float aimRangeLowerLimit) { m_AimRangeLowerLimit = aimRangeLowerLimit; }

		/// Protected member variable and method declarations
	protected:
		/// Function that is called when we get a new movepath.
		/// This processes and cleans up the movepath.
		void OnNewMovePath() override;

		// Member variables
		static Entity::ClassInfo m_sClass;

		// Turret which can be mounted with a weapon
		Turret* m_pTurret;
		// TODO when this class is cleaned up these legs and footgroups should probably be renamed. L and R should be expanded to Left and Right. I think FG and BG can stay as is cause they're everywhere.
		//  Left Foreground leg.
		Leg* m_pLFGLeg;
		// Left Background leg.
		Leg* m_pLBGLeg;
		// Right Foreground leg.
		Leg* m_pRFGLeg;
		// Right Background leg.
		Leg* m_pRBGLeg;
		// Limb AtomGroups.
		AtomGroup* m_pLFGFootGroup;
		AtomGroup* m_BackupLFGFootGroup;
		AtomGroup* m_pLBGFootGroup;
		AtomGroup* m_BackupLBGFootGroup;
		AtomGroup* m_pRFGFootGroup;
		AtomGroup* m_BackupRFGFootGroup;
		AtomGroup* m_pRBGFootGroup;
		AtomGroup* m_BackupRBGFootGroup;
		// The sound of the actor taking a step (think robot servo)
		SoundContainer* m_StrideSound;
		// Jetpack booster.
		AEJetpack* m_pJetpack;
		// Blink timer
		Timer m_IconBlinkTimer;
		// Whether a stride was taken this frame or not.
		bool m_StrideFrame = false;
		// Limb paths for different movement states.
		// First which side, then which background/foreground, then the movement state
		LimbPath m_Paths[SIDECOUNT][LAYERCOUNT][MOVEMENTSTATECOUNT];
		// Whether was aiming during the last frame too.
		bool m_Aiming;
		// Controls the start of leg synch.
		bool m_StrideStart[SIDECOUNT];
		// Times the strides to make sure they get restarted if they end up too long
		Timer m_StrideTimer[SIDECOUNT];
		// The maximum angle MountedMO can be aimed up, positive values only, in radians
		float m_AimRangeUpperLimit;
		// The maximum angle MountedMO can be aimed down, positive values only, in radians
		float m_AimRangeLowerLimit;

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
		// Whether mouse input should be locked to the aim range, or whether we can "wander" out. For static emplacements like turrets, this is good, but for moving things it's a bit sticky.
		bool m_LockMouseAimInput;

#pragma region Event Handling
		/// Event listener to be run while this ACrab's PieMenu is opened.
		/// @param pieMenu The PieMenu this event listener needs to listen to. This will always be this' m_PieMenu and only exists for std::bind.
		/// @return An error return value signaling success or any particular failure. Anything below 0 is an error signal.
		int WhilePieMenuOpenListener(const PieMenu* pieMenu) override;
#pragma endregion

		/// Private member variable and method declarations
	private:
		/// Clears all the member variables of this ACrab, effectively
		/// resetting the members of this abstraction level only.
		void Clear();

		// Disallow the use of some implicit methods.
		ACrab(const ACrab& reference) = delete;
		ACrab& operator=(const ACrab& rhs) = delete;
	};

} // namespace RTE
