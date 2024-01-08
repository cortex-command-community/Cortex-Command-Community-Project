#ifndef _RTEACRAB_
#define _RTEACRAB_

//////////////////////////////////////////////////////////////////////////////////////////
// File:            ACrab.h
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Header file for the ACrab class.
// Project:         Retro Terrain Engine
// Author(s):       Daniel Tabar
//                  data@datarealms.com
//                  http://www.datarealms.com


//////////////////////////////////////////////////////////////////////////////////////////
// Inclusions of header files

#include "Actor.h"
#include "LimbPath.h"
#include "Leg.h"

struct BITMAP;

namespace RTE
{

class Turret;
class AEJetpack;


//////////////////////////////////////////////////////////////////////////////////////////
// Class:           ACrab
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     A crab-like actor with four legs.
// Parent(s):       Actor.
// Class history:   10/24/2007 ACrab created.

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


	//////////////////////////////////////////////////////////////////////////////////////////
	// Public member variable, method and friend function declarations

public:


// Concrete allocation and cloning definitions
	EntityAllocation(ACrab);
	AddScriptFunctionNames(Actor, "OnStride");
	SerializableOverrideMethods;
	ClassInfoGetters;
	DefaultPieMenuNameGetter(HasObjectInGroup("Turrets") ? "Default Turret Pie Menu" : "Default Crab Pie Menu");

	//////////////////////////////////////////////////////////////////////////////////////////
	// Constructor:     ACrab
	//////////////////////////////////////////////////////////////////////////////////////////
	// Description:     Constructor method used to instantiate a ACrab object in system
	//                  memory. Create() should be called before using the object.
	// Arguments:       None.

	ACrab() { Clear(); }


	//////////////////////////////////////////////////////////////////////////////////////////
	// Destructor:      ~ACrab
	//////////////////////////////////////////////////////////////////////////////////////////
	// Description:     Destructor method used to clean up a ACrab object before deletion
	//                  from system memory.
	// Arguments:       None.

	~ACrab() override { Destroy(true); }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the ACrab object ready for use.
// Arguments:       None.
// Return value:    An error return value signaling sucess or any particular failure.
//                  Anything below 0 is an error signal.

	int Create() override;


 //////////////////////////////////////////////////////////////////////////////////////////
 // Virtual method:  Create
 //////////////////////////////////////////////////////////////////////////////////////////
 // Description:     Creates a ACrab to be identical to another, by deep copy.
 // Arguments:       A reference to the ACrab to deep copy.
 // Return value:    An error return value signaling sucess or any particular failure.
 //                  Anything below 0 is an error signal.

	int Create(const ACrab &reference);


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Reset
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Resets the entire ACrab, including its inherited members, to their
//                  default settings or values.
// Arguments:       None.
// Return value:    None.

	void Reset() override { Clear(); Actor::Reset(); }


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Destroy
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destroys and resets (through Clear()) the SceneLayer object.
// Arguments:       Whether to only destroy the members defined in this derived class, or
//                  to destroy all inherited members also.
// Return value:    None.

	void Destroy(bool notInherited = false) override;


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  GetEyePos
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the absoltue position of this' eye, or equivalent, where look
//                  vector starts from.
// Arguments:       None.
// Return value:    A Vector with the absolute position of this' eye or view point.

	Vector GetEyePos() const override;


	/// <summary>
	/// Gets the Turret of this ACrab.
	/// </summary>
	/// <returns>A pointer to Turret of this ACrab. Ownership is NOT transferred!</returns>
	Turret * GetTurret() const { return m_pTurret; }

	/// <summary>
	/// Sets the Turret for this ACrab. Ownership IS transferred!
	/// </summary>
	/// <param name="newTurret">The new Turret to use.</param>
	void SetTurret(Turret *newTurret);

	/// <summary>
	/// Gets the jetpack of this ACrab.
	/// </summary>
	/// <returns>A pointer to the jetpack of this ACrab. Ownership is NOT transferred!</returns>
	AEJetpack * GetJetpack() const { return m_pJetpack; }

	/// <summary>
	/// Sets the jetpack for this ACrab. Ownership IS Transferred!
	/// </summary>
	/// <param name="newJetpack">The new jetpack to use.</param>
	void SetJetpack(AEJetpack *newJetpack);

    /// <summary>
    /// Gets the left foreground Leg of this ACrab.
    /// </summary>
    /// <returns>A pointer to the left foreground Leg of this ACrab. Ownership is NOT transferred!</returns>
    Leg * GetLeftFGLeg() const { return m_pLFGLeg; }

    /// <summary>
    /// Sets the left foreground Leg for this ACrab. Ownership IS transferred!
    /// </summary>
    /// <param name="newLeg">The new Leg to use.</param>
    void SetLeftFGLeg(Leg *newLeg);

    /// <summary>
    /// Gets the left background Leg of this ACrab.
    /// </summary>
    /// <returns>A pointer to the left background Leg of this ACrab. Ownership is NOT transferred!</returns>
    Leg * GetLeftBGLeg() const { return m_pLBGLeg; }

    /// <summary>
    /// Sets the left background Leg for this ACrab. Ownership IS transferred!
    /// </summary>
    /// <param name="newLeg">The new Leg to use.</param>
    void SetLeftBGLeg(Leg *newLeg);

    /// <summary>
    /// Gets the right foreground Leg of this ACrab.
    /// </summary>
    /// <returns>A pointer to the right foreground Leg of this ACrab. Ownership is NOT transferred!</returns>
    Leg * GetRightFGLeg() const { return m_pRFGLeg; }

    /// <summary>
    /// Sets the right foreground Leg for this ACrab. Ownership IS transferred!
    /// </summary>
    /// <param name="newLeg">The new Leg to use.</param>
    void SetRightFGLeg(Leg *newLeg);

    /// <summary>
    /// Gets the right BG Leg of this ACrab.
    /// </summary>
    /// <returns>A pointer to the right background Leg of this ACrab. Ownership is NOT transferred!</returns>
    Leg * GetRightBGLeg() const { return m_pRBGLeg; }

    /// <summary>
    /// Sets the right background Leg for this ACrab. Ownership IS transferred!
    /// </summary>
    /// <param name="newLeg">The new Leg to use.</param>
    void SetRightBGLeg(Leg *newLeg);


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  CollideAtPoint
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Calculates the collision response when another MO's Atom collides with
//                  this MO's physical representation. The effects will be applied
//                  directly to this MO, and also represented in the passed in HitData.
// Arguments:       Reference to the HitData struct which describes the collision. This
//                  will be modified to represent the results of the collision.
// Return value:    Whether the collision has been deemed valid. If false, then disregard
//                  any impulses in the Hitdata.

	bool CollideAtPoint(HitData &hitData) override;

	/// <summary>
	/// Tries to handle the activated PieSlice in this object's PieMenu, if there is one, based on its SliceType.
	/// </summary>
	/// <param name="pieSliceType">The SliceType of the PieSlice being handled.</param>
	/// <returns>Whether or not the activated PieSlice SliceType was able to be handled.</returns>
	bool HandlePieCommand(PieSlice::SliceType pieSliceType) override;


//////////////////////////////////////////////////////////////////////////////////////////
// Method:  GetEquippedItem
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns whatever is equipped in the turret, if anything. OWNERSHIP IS NOT TRANSFERRED!
// Arguments:       None.
// Return value:    The currently equipped item, if any.

	MovableObject * GetEquippedItem() const;


//////////////////////////////////////////////////////////////////////////////////////////
// Method:  FirearmIsReady
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Indicates whether the currently held HDFirearm's is ready for use, and has
//                  ammo etc.
// Arguments:       None.
// Return value:    Whether a currently HDFirearm (if any) is ready for use.

	bool FirearmIsReady() const;


//////////////////////////////////////////////////////////////////////////////////////////
// Method:  FirearmIsEmpty
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Indicates whether the currently held HDFirearm's is out of ammo.
// Arguments:       None.
// Return value:    Whether a currently HDFirearm (if any) is out of ammo.

	bool FirearmIsEmpty() const;


//////////////////////////////////////////////////////////////////////////////////////////
// Method:  FirearmNeedsReload
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Indicates whether the currently held HDFirearm's is almost out of ammo.
// Arguments:       None.
// Return value:    Whether a currently HDFirearm (if any) has less than half of ammo left.

	bool FirearmNeedsReload() const;

	/// <summary>
	/// Gets whether or not all of this ACrab's Turret's HDFirearms are full.
	/// </summary>
	/// <returns>Whether or not all of this ACrab's Turret's HDFirearms are full. Will return true if there is no Turret or no HDFirearms.</returns>
	bool FirearmsAreFull() const;


//////////////////////////////////////////////////////////////////////////////////////////
// Method:  FirearmIsSemiAuto
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Indicates whether the currently held HDFirearm's is semi or full auto.
// Arguments:       None.
// Return value:    Whether a currently HDFirearm (if any) is a semi auto device.

	bool FirearmIsSemiAuto() const;


//////////////////////////////////////////////////////////////////////////////////////////
// Method:  FirearmActivationDelay
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Returns the currently held device's delay between pulling the trigger
//                  and activating.
// Arguments:       None.
// Return value:    Delay in ms or zero if not a HDFirearm.

int FirearmActivationDelay() const;


//////////////////////////////////////////////////////////////////////////////////////////
// Method:  ReloadFirearms
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Reloads the currently held firearms, if any.
// Arguments:       None.
// Return value:    None.

	void ReloadFirearms();


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  IsWithinRange
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Tells whether a point on the scene is within close range of the currently
//                  used device and aiming status, if applicable.
// Arguments:       A Vector witht he aboslute coordinates of a point to check.
// Return value:    Whether the point is within close range of this.

	bool IsWithinRange(Vector &point) const override;


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Look
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Casts an unseen-revealing ray in the direction of where this is facing.
// Arguments:       The degree angle to deviate from the current view point in the ray
//                  casting. A random ray will be chosen out of this +-range.
//                  The range, in pixels, beyond the actors sharp aim that the ray will have.
// Return value:    Whether any unseen pixels were revealed by this look.

	bool Look(float FOVSpread, float range) override;


//////////////////////////////////////////////////////////////////////////////////////////
// Method:  LookForMOs
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Casts an MO detecting ray in the direction of where the head is looking
//                  at the time. Factors including head rotation, sharp aim mode, and
//                  other variables determine how this ray is cast.
// Arguments:       The degree angle to deviate from the current view point in the ray
//                  casting. A random ray will be chosen out of this +-range.
//                  A specific material ID to ignore (see through)
//                  Whether to ignore all terrain or not (true means 'x-ray vision').
// Return value:    A pointer to the MO seen while looking.

	MovableObject * LookForMOs(float FOVSpread = 45, unsigned char ignoreMaterial = 0, bool ignoreAllTerrain = false);


	/// <summary>
	/// Gets the GUI representation of this ACrab, only defaulting to its Turret or body if no GraphicalIcon has been defined.
	/// </summary>
	/// <returns>The graphical representation of this ACrab as a BITMAP.</returns>
	BITMAP * GetGraphicalIcon() const override;


	/// <summary>
	/// Gets whether this ACrab has just taken a stride this frame.
	/// </summary>
	/// <returns>Whether this ACrab has taken a stride this frame or not.</returns>
	bool StrideFrame() const { return m_StrideFrame; }

//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  PreControllerUpdate
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Update called prior to controller update. Ugly hack. Supposed to be done every frame.
// Arguments:       None.
// Return value:    None.

	void PreControllerUpdate() override;

//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Update
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates this MovableObject. Supposed to be done every frame.
// Arguments:       None.
// Return value:    None.

	void Update() override;


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  Draw
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Draws this ACrab's current graphical representation to a
//                  BITMAP of choice.
// Arguments:       A pointer to a BITMAP to draw on.
//                  The absolute position of the target bitmap's upper left corner in the Scene.
//                  In which mode to draw in. See the DrawMode enumeration for the modes.
//                  Whether to not draw any extra 'ghost' items of this MovableObject,
//                  indicator arrows or hovering HUD text and so on.
// Return value:    None.

	void Draw(BITMAP *pTargetBitmap, const Vector &targetPos = Vector(), DrawMode mode = g_DrawColor, bool onlyPhysical = false) const override;


//////////////////////////////////////////////////////////////////////////////////////////
// Virtual method:  DrawHUD
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Draws this Actor's current graphical HUD overlay representation to a
//                  BITMAP of choice.
// Arguments:       A pointer to a BITMAP to draw on.
//                  The absolute position of the target bitmap's upper left corner in the Scene.
//                  Which player's screen this is being drawn to. May affect what HUD elements
//                  get drawn etc.
// Return value:    None.

	void DrawHUD(BITMAP *pTargetBitmap, const Vector &targetPos = Vector(), int whichScreen = 0, bool playerControlled = false) override;


	/// <summary>
	/// Gets the LimbPath corresponding to the passed in Side, Layer and MovementState values.
	/// </summary>
	/// <param name="side">Whether to get the left or right side.</param>
	/// <param name="layer">Whether to get foreground or background LimbPath.</param>
	/// <param name="movementState">Which movement state to get the LimbPath for.</param>
	/// <returns>The LimbPath corresponding to the passed in Layer and MovementState values.</returns>
	LimbPath *GetLimbPath(Side side, Layer layer, MovementState movementState) { return &m_Paths[side][layer][movementState]; }


//////////////////////////////////////////////////////////////////////////////////////////
// Method:  GetLimbPathSpeed
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Get walking limb path speed for the specified preset.
// Arguments:       Speed preset to set 0 = LimbPath::SLOW, 1 = Limbpath::NORMAL, 2 = LimbPath::FAST
// Return value:    Limb path speed for the specified preset in m/s.

	float GetLimbPathSpeed(int speedPreset) const;


//////////////////////////////////////////////////////////////////////////////////////////
// Method:  SetLimbPathSpeed
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Set walking limb path speed for the specified preset.
// Arguments:       Speed preset to set 0 = LimbPath::SLOW, 1 = Limbpath::NORMAL, 2 = LimbPath::FAST. New speed value in m/s.
// Return value:    None.

	void SetLimbPathSpeed(int speedPreset, float speed);


//////////////////////////////////////////////////////////////////////////////////////////
// Method:  GetLimbPathPushForce
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets the default force that a limb traveling walking LimbPath can push against
//                  stuff in the scene with.
// Arguments:       None.
// Return value:    The default set force maximum, in kg * m/s^2.

	float GetLimbPathPushForce() const;


//////////////////////////////////////////////////////////////////////////////////////////
// Method:  SetLimbPathPushForce
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the default force that a limb traveling walking LimbPath can push against
//                  stuff in the scene with.
// Arguments:       The default set force maximum, in kg * m/s^2.
// Return value:    None

	void SetLimbPathPushForce(float force);


	/// <summary>
	/// Gets this ACrab's stride sound. Ownership is NOT transferred!
	/// </summary>
	/// <returns>The SoundContainer for this ACrab's stride sound.</returns>
	SoundContainer * GetStrideSound() const { return m_StrideSound; }

	/// <summary>
	/// Sets this ACrab's stride sound. Ownership IS transferred!
	/// </summary>
	/// <param name="newSound">The new SoundContainer for this ACrab's stride sound.</param>
	void SetStrideSound(SoundContainer *newSound) { m_StrideSound = newSound; }

	/// <summary>
	/// Gets the upper limit of this ACrab's aim range.
	/// </summary>
	/// <returns>The upper limit of this ACrab's aim range.</returns>
	float GetAimRangeUpperLimit() const { return m_AimRangeUpperLimit; }

	/// <summary>
	/// Sets the upper limit of this ACrab's aim range.
	/// </summary>
	/// <param name="aimRangeUpperLimit">The new upper limit of this ACrab's aim range.</param>
	void SetAimRangeUpperLimit(float aimRangeUpperLimit) { m_AimRangeUpperLimit = aimRangeUpperLimit; }

	/// <summary>
	/// Gets the lower limit of this ACrab's aim range.
	/// </summary>
	/// <returns>The lower limit of this ACrab's aim range.</returns>
	float GetAimRangeLowerLimit() const { return m_AimRangeLowerLimit; }

	/// <summary>
	/// Sets the lower limit of this ACrab's aim range.
	/// </summary>
	/// <param name="aimRangeLowerLimit">The new lower limit of this ACrab's aim range.</param>
	void SetAimRangeLowerLimit(float aimRangeLowerLimit) { m_AimRangeLowerLimit = aimRangeLowerLimit; }

//////////////////////////////////////////////////////////////////////////////////////////
// Protected member variable and method declarations

protected:

    /// <summary>
    /// Function that is called when we get a new movepath.
	/// This processes and cleans up the movepath.
    /// </summary>
    void OnNewMovePath() override;

	// Member variables
	static Entity::ClassInfo m_sClass;

	// Turret which can be mounted with a weapon
	Turret *m_pTurret;
	//TODO when this class is cleaned up these legs and footgroups should probably be renamed. L and R should be expanded to Left and Right. I think FG and BG can stay as is cause they're everywhere.
	// Left Foreground leg.
	Leg *m_pLFGLeg;
	// Left Background leg.
	Leg *m_pLBGLeg;
	// Right Foreground leg.
	Leg *m_pRFGLeg;
	// Right Background leg.
	Leg *m_pRBGLeg;
	// Limb AtomGroups.
	AtomGroup *m_pLFGFootGroup;
	AtomGroup *m_BackupLFGFootGroup;
	AtomGroup *m_pLBGFootGroup;
	AtomGroup *m_BackupLBGFootGroup;
	AtomGroup *m_pRFGFootGroup;
	AtomGroup *m_BackupRFGFootGroup;
	AtomGroup *m_pRBGFootGroup;
	AtomGroup *m_BackupRBGFootGroup;
	// The sound of the actor taking a step (think robot servo)
	SoundContainer *m_StrideSound;
	// Jetpack booster.
	AEJetpack *m_pJetpack;
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
	/// <summary>
	/// Event listener to be run while this ACrab's PieMenu is opened.
	/// </summary>
	/// <param name="pieMenu">The PieMenu this event listener needs to listen to. This will always be this' m_PieMenu and only exists for std::bind.</param>
	/// <returns>An error return value signaling success or any particular failure. Anything below 0 is an error signal.</returns>
	int WhilePieMenuOpenListener(const PieMenu *pieMenu) override;
#pragma endregion


//////////////////////////////////////////////////////////////////////////////////////////
// Private member variable and method declarations

private:

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Clear
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears all the member variables of this ACrab, effectively
//                  resetting the members of this abstraction level only.
// Arguments:       None.
// Return value:    None.

	void Clear();

	// Disallow the use of some implicit methods.
	ACrab(const ACrab &reference) = delete;
	ACrab & operator=(const ACrab &rhs) = delete;

};

} // namespace RTE

#endif // File
