#pragma once

/// Header file for the MOSRotating class.
/// @author Daniel Tabar
/// data@datarealms.com
/// http://www.datarealms.com
/// Inclusions of header files
#include "MOSprite.h"
#include "Gib.h"

#include <unordered_set>

namespace RTE {

	class AtomGroup;
	struct HitData;
	class AEmitter;
	class Attachable;
	class SoundContainer;

	/// A sprite movable object that can rotate.
	class MOSRotating : public MOSprite {

		/// Public member variable, method and friend function declarations
	public:
		friend class AtomGroup;
		friend class SLTerrain;
		friend struct EntityLuaBindings;

		// Concrete allocation and cloning definitions
		EntityAllocation(MOSRotating);
		SerializableOverrideMethods;
		ClassInfoGetters;

		/// Constructor method used to instantiate a MOSRotating object in system
		/// memory. Create() should be called before using the object.
		MOSRotating();

		/// Destructor method used to clean up a MOSRotating object before deletion
		/// from system memory.
		~MOSRotating() override;

		/// Makes the MOSRotating object ready for use.
		/// @return An error return value signaling sucess or any particular failure.
		/// Anything below 0 is an error signal.
		int Create() override;

		/// Makes the MOSRotating object ready for use.
		/// @param spriteFile A pointer to ContentFile that represents the bitmap file that will be
		/// used to create the Sprite.
		/// @param frameCount The number of frames in the Sprite's animation. (default: 1)
		/// @param mass A float specifying the object's mass in Kilograms (kg). (default: 1)
		/// @param position A Vector specifying the initial position. (default: Vector(0)
		/// @param 0) A Vector specifying the initial velocity.
		/// @param velocity The amount of time in ms this MovableObject will exist. 0 means unlim. (default: Vector(0)
		/// @return An error return value signaling sucess or any particular failure.
		/// Anything below 0 is an error signal.
		int Create(ContentFile spriteFile, const int frameCount = 1, const float mass = 1, const Vector& position = Vector(0, 0), const Vector& velocity = Vector(0, 0), const unsigned long lifetime = 0);

		/// Creates a MOSRotating to be identical to another, by deep copy.
		/// @param reference A reference to the MOSRotating to deep copy.
		/// @return An error return value signaling sucess or any particular failure.
		/// Anything below 0 is an error signal.
		int Create(const MOSRotating& reference);

		/// Resets the entire MOSRotating, including its inherited members, to their
		/// default settings or values.
		void Reset() override {
			Clear();
			MOSprite::Reset();
		}

		/// Destroys and resets (through Clear()) the SceneLayer object.
		/// @param notInherited Whether to only destroy the members defined in this derived class, or (default: false)
		/// to destroy all inherited members also.
		void Destroy(bool notInherited = false) override;

		/// Gets the radius of this MOSRotating, not including any Attachables.
		/// @return
		float GetIndividualRadius() const { return m_SpriteRadius; }

		/// Gets the radius of this MOSRotating, including any Attachables.
		/// @return The radius of this MOSRotating, including any Attachables.
		float GetRadius() const override { return std::max(m_SpriteRadius, m_FarthestAttachableDistanceAndRadius); }

		/// Gets the diameter of this MOSRotating, not including any Attachables.
		/// @return
		float GetIndividualDiameter() const { return m_SpriteDiameter; }

		/// Gets the diameter of this MOSRotating, including any Attachables.
		/// @return The diameter of this MOSRotating, including any Attachables.
		float GetDiameter() const override { return GetRadius() * 2.0F; }

		/// Checks if the given Attachable should affect radius, and handles it if it should.
		/// @param attachable The Attachable to check.
		/// @return Whether the radius affecting Attachable changed as a result of this call.
		virtual bool HandlePotentialRadiusAffectingAttachable(const Attachable* attachable);

		/// Gets the mass value of this MOSRotating, not including any Attachables or wounds.
		/// @return The mass of this MOSRotating.
		float GetIndividualMass() const { return MovableObject::GetMass(); }

		/// Gets the mass value of this MOSRotating, including the mass of all its Attachables and wounds, and their Attachables and so on.
		/// @return The mass of this MOSRotating and all of its Attachables and wounds in Kilograms (kg).
		float GetMass() const override { return MovableObject::GetMass() + m_AttachableAndWoundMass; }

		/// Updates the total mass of Attachables and wounds for this MOSRotating, intended to be used when Attachables' masses get modified. Simply subtracts the old mass and adds the new one.
		/// @param oldAttachableOrWoundMass The mass the Attachable or wound had before its mass was modified.
		/// @param newAttachableOrWoundMass The up-to-date mass of the Attachable or wound after its mass was modified.
		virtual void UpdateAttachableAndWoundMass(float oldAttachableOrWoundMass, float newAttachableOrWoundMass) { m_AttachableAndWoundMass += newAttachableOrWoundMass - oldAttachableOrWoundMass; }

		/// Gets the MOIDs of this MOSRotating and all its Attachables and Wounds, putting them into the MOIDs vector.
		/// @param MOIDs The vector that will store all the MOIDs of this MOSRotating.
		void GetMOIDs(std::vector<MOID>& MOIDs) const override;

		/// Sets the MOID of this MOSRotating and any Attachables on it to be g_NoMOID (255) for this frame.
		void SetAsNoID() override;

		/// Sets this MOSRotating to not hit a specific other MO and all its children even though MO hitting is enabled on this MOSRotating.
		/// @param moToNotHit A pointer to the MO to not be hitting. Null pointer means don't ignore anything. Ownership is NOT transferred!
		/// @param forHowLong How long, in seconds, to ignore the specified MO. A negative number means forever.
		void SetWhichMOToNotHit(MovableObject* moToNotHit = nullptr, float forHowLong = -1) override;

		/// Gets the current AtomGroup of this MOSRotating.
		/// @return A const reference to the current AtomGroup.
		AtomGroup* GetAtomGroup() { return m_pAtomGroup; }

		/// Gets the main Material  of this MOSRotating.
		/// @return The the Material of this MOSRotating.
		Material const* GetMaterial() const override;

		/// Gets the drawing priority of this MovableObject, if two things were
		/// overlap when copying to the terrain, the higher priority MO would
		/// end up getting drawn.
		/// @return The the priority  of this MovableObject. Higher number, the higher
		/// priority.
		int GetDrawPriority() const override;

		/// Gets the current recoil impulse force Vector of this MOSprite.
		/// @return A const reference to the current recoil impulse force in kg * m/s.
		const Vector& GetRecoilForce() const { return m_RecoilForce; }

		/// Gets the current recoil offset Vector of this MOSprite.
		/// @return A const reference to the current recoil offset.
		const Vector& GetRecoilOffset() const { return m_RecoilOffset; }

		/// Gets direct access to the list of object this is to generate upon gibbing.
		/// @return A pointer to the list of gibs. Ownership is NOT transferred!
		std::list<Gib>* GetGibList() { return &m_Gibs; }

		/// Adds graphical recoil offset to this MOSprite according to its angle.
		void AddRecoil();

		/// Adds recoil offset to this MOSprite.
		/// @param force A vector with the recoil impulse force in kg * m/s.
		/// @param offset A vector with the recoil offset in pixels.
		/// @param recoil Whether recoil should be activated or not for the next Draw(). (default: true)
		void SetRecoil(const Vector& force, const Vector& offset, bool recoil = true) {
			m_RecoilForce = force;
			m_RecoilOffset = offset;
			m_Recoiled = recoil;
		}

		/// Returns whether this MOSprite is currently under the effects of
		/// recoil.
		bool IsRecoiled() { return m_Recoiled; }

		/// Sets whether or not this MOSRotating should check for deep penetrations
		/// the terrain or not.
		/// @param enable Whether to enable deep penetration checking or not. (default: true)
		void EnableDeepCheck(const bool enable = true) { m_DeepCheck = enable; }

		/// Sets to force a deep checking of this' silhouette against the terrain
		/// and create an outline hole in the terrain, generating particles of the
		/// intersecting pixels in the terrain.
		/// @param enable Whether to force a deep penetration check for this sim frame or not.. (default: true)
		void ForceDeepCheck(const bool enable = true) { m_ForceDeepCheck = enable; }

		/// Calculates the collision response when another MO's Atom collides with
		/// this MO's physical representation. The effects will be applied
		/// directly to this MO, and also represented in the passed in HitData.
		/// @param hitData Reference to the HitData struct which describes the collision. This
		/// will be modified to represent the results of the collision.
		/// @return Whether the collision has been deemed valid. If false, then disregard
		/// any impulses in the Hitdata.
		bool CollideAtPoint(HitData& hitData) override;

		/// Defines what should happen when this MovableObject hits and then bounces off of something.
		/// This is called by the owned Atom/AtomGroup of this MovableObject during travel.
		/// @param hd The HitData describing the collision in detail.
		/// @return Whether the MovableObject should immediately halt any travel going on after this bounce.
		bool OnBounce(HitData& hd) override;

		/// Defines what should happen when this MovableObject hits and then
		/// sink into something. This is called by the owned Atom/AtomGroup
		/// of this MovableObject during travel.
		/// @param hd The HitData describing the collision in detail.
		/// @return Wheter the MovableObject should immediately halt any travel going on
		/// after this sinkage.
		bool OnSink(HitData& hd) override;

		/// Determines whether a particle which has hit this MO will penetrate,
		/// and if so, whether it gets lodged or exits on the other side of this
		/// MO. Appropriate effects will be determined and applied ONLY IF there
		/// was penetration! If not, nothing will be affected.
		/// @param hd The HitData describing the collision in detail, the impulses have to
		/// have been filled out!
		/// @return Whether the particle managed to penetrate into this MO or not. If
		/// somehting but a MOPixel or MOSParticle is being passed in as hitor,
		/// false will trivially be returned here.
		virtual bool ParticlePenetration(HitData& hd);

		/// Destroys this MOSRotating and creates its specified Gibs in its place with appropriate velocities. Any Attachables are removed and also given appropriate velocities.
		/// @param impactImpulse The impulse (kg * m/s) of the impact causing the gibbing to happen.
		/// @param movableObjectToIgnore A pointer to an MO which the Gibs and Attachables should not be colliding with.
		virtual void GibThis(const Vector& impactImpulse = Vector(), MovableObject* movableObjectToIgnore = nullptr);

		/// Checks whether any of the Atom:s in this MovableObject are on top of
		/// terrain pixels, and if so, attempt to move this out so none of this'
		/// Atoms are on top of the terrain any more.
		/// @param strongerThan Only consider materials stronger than this in the terrain for (default: g_MaterialAir)
		/// intersections.
		/// @return Whether any intersection was successfully resolved. Will return true
		/// even if there wasn't any intersections to begin with.
		bool MoveOutOfTerrain(unsigned char strongerThan = g_MaterialAir) override;

		/// Gathers, clears and applies this MOSRotating's accumulated forces.
		void ApplyForces() override;

		/// Gathers, clears and applies this MOSRotating's accumulated impulse forces, gibbing if appropriate.
		void ApplyImpulses() override;

		/// Gets the list of Attachables on this MOSRotating.
		/// @return The list of Attachables on this MOSRotating.
		const std::list<Attachable*>& GetAttachables() const { return m_Attachables; }

		/// Gets whether or not the given Attachable is a hardcoded Attachable (e.g. an Arm, Leg, Turret, etc.)
		/// @param attachableToCheck The Attachable to check.
		/// @return Whether or not the Attachable is hardcoded.
		bool AttachableIsHardcoded(const Attachable* attachableToCheck) const;

		/// Adds the passed in Attachable the list of Attachables and sets its parent to this MOSRotating.
		/// @param attachable The Attachable to add.
		virtual void AddAttachable(Attachable* attachable);

		/// Adds the passed in Attachable the list of Attachables, changes its parent offset to the passed in Vector, and sets its parent to this MOSRotating.
		/// @param attachable The Attachable to add.
		/// @param parentOffsetToSet The Vector to set as the Attachable's parent offset.
		virtual void AddAttachable(Attachable* attachable, const Vector& parentOffsetToSet);

		/// Removes the Attachable corresponding to the passed in UniqueID and sets its parent to nullptr. Does not add it to MovableMan or add break wounds.
		/// @param attachableUniqueID The UniqueID of the Attachable to remove.
		/// @return A pointer to the removed Attachable. Ownership IS transferred!
		virtual Attachable* RemoveAttachable(long attachableUniqueID) { return RemoveAttachable(attachableUniqueID, false, false); }

		/// Removes the Attachable corresponding to the passed in UniqueID and sets its parent to nullptr. Optionally adds it to MovableMan and/or adds break wounds.
		/// If the Attachable is not set to delete or delete when removed from its parent, and addToMovableMan is false, the caller must hang onto a pointer to the Attachable ahead of time to avoid memory leaks.
		/// @param attachableUniqueID The UniqueID of the Attachable to remove.
		/// @param addToMovableMan Whether or not to add the Attachable to MovableMan once it has been removed.
		/// @param addBreakWounds Whether or not to add break wounds to the removed Attachable and this MOSRotating.
		/// @return A pointer to the removed Attachable, if it wasn't added to MovableMan or nullptr if it was. Ownership IS transferred!
		virtual Attachable* RemoveAttachable(long attachableUniqueID, bool addToMovableMan, bool addBreakWounds);

		/// Removes the passed in Attachable and sets its parent to nullptr. Does not add it to MovableMan or add break wounds.
		/// @param attachable The Attachable to remove.
		/// @return A pointer to the removed Attachable. Ownership IS transferred!
		virtual Attachable* RemoveAttachable(Attachable* attachable) { return RemoveAttachable(attachable, false, false); }

		/// Removes the passed in Attachable and sets its parent to nullptr. Optionally adds it to MovableMan and/or adds break wounds.
		/// If the Attachable is not set to delete or delete when removed from its parent, and addToMovableMan is false, the caller must hang onto a pointer to the Attachable ahead of time to avoid memory leaks.
		/// @param attachable The Attachable to remove.
		/// @param addToMovableMan Whether or not to add the Attachable to MovableMan once it has been removed.
		/// @param addBreakWounds Whether or not to add break wounds to the removed Attachable and this MOSRotating.
		/// @return A pointer to the removed Attachable, if it wasn't added to MovableMan or nullptr if it was. Ownership IS transferred!
		virtual Attachable* RemoveAttachable(Attachable* attachable, bool addToMovableMan, bool addBreakWounds);

		/// Removes the passed in Attachable, and sets it to delete so it will go straight to MovableMan and be handled there.
		/// @param attachable The Attacahble to remove and delete.
		void RemoveAndDeleteAttachable(Attachable* attachable);

		/// Either removes or deletes all of this MOSRotating's Attachables.
		/// @param destroy Whether to remove or delete the Attachables. Setting this to true deletes them, setting it to false removes them.
		void RemoveOrDestroyAllAttachables(bool destroy);

		/// Gets a damage-transferring, impulse-vulnerable Attachable nearest to the passed in offset.
		/// @param offset The offset that will be compared to each Attachable's ParentOffset.
		/// @return The nearest detachable Attachable, or nullptr if none was found.
		Attachable* GetNearestDetachableAttachableToOffset(const Vector& offset) const;

		/// Gibs or detaches any Attachables that would normally gib or detach from the passed in impulses.
		/// @param impulseVector The impulse vector which determines the Attachables to gib or detach. Will be filled out with the remainder of impulses.
		void DetachAttachablesFromImpulse(Vector& impulseVector);

		/// Resest all the timers used by this. Can be emitters, etc. This is to
		/// prevent backed up emissions to come out all at once while this has been
		/// held dormant in an inventory.
		void ResetAllTimers() override;

		/// Does the calculations necessary to detect whether this MOSRotating is at rest or not. IsAtRest() retrieves the answer.
		void RestDetection() override;

		/// Indicates whether this MOSRotating has been at rest with no movement for longer than its RestThreshold.
		bool IsAtRest() override;

		/// Indicates whether this MOSRotating's current graphical representation, including its Attachables, overlaps a point in absolute scene coordinates.
		/// @param scenePoint The point in absolute scene coordinates to check for overlap with.
		/// @return Whether or not this MOSRotating's graphical representation overlaps the given scene point.
		bool IsOnScenePoint(Vector& scenePoint) const override;

		/// Cuts this' silhouette out from the terrain's material and color layers.
		void EraseFromTerrain();

		/// Checks if any of this' deep group atmos are on top of the terrain, and
		/// if so, erases this' silhouette from the terrain.
		/// @param makeMOPs Whether to make any MOPixels from erased terrain pixels at all. (default: true)
		/// @param skipMOP The size of the gaps between MOPixels knocked loose by the terrain erasure. (default: 2)
		/// @param maxMOP The max number of MOPixel:s to generate as dislodged particles from the (default: 100)
		/// erased terrain.
		/// @return Whether deep penetration was detected and erasure was done.
		bool DeepCheck(bool makeMOPs = true, int skipMOP = 2, int maxMOP = 100);

		/// Travels this MOSRotating, using its physical representation.
		void Travel() override;

		/// Does stuff that needs to be done after Update(). Always call after calling Update.
		void PostTravel() override;

		/// Updates this MovableObject. Supposed to be done every frame.
		void Update() override;

		// Stuff that needs to be updated post-Update.
		void PostUpdate() override;

		/// Draws this MOSRotating's current graphical representation to a
		/// BITMAP of choice.
		/// @param pTargetBitmap A pointer to a BITMAP to draw on.
		/// @param targetPos The absolute position of the target bitmap's upper left corner in the Scene. (default: Vector())
		/// @param mode In which mode to draw in. See the DrawMode enumeration for the modes. (default: g_DrawColor)
		/// @param onlyPhysical Whether to not draw any extra 'ghost' items of this MovableObject, (default: false)
		/// indicator arrows or hovering HUD text and so on.
		void Draw(BITMAP* pTargetBitmap, const Vector& targetPos = Vector(), DrawMode mode = g_DrawColor, bool onlyPhysical = false) const override;

		/// Gets the gib impulse limit for this MOSRotating, i.e. the amount of impulse force required in a frame to gib this MOSRotating.
		/// @return The gib impulse limit of this MOSRotating.
		float GetGibImpulseLimit() const { return m_GibImpulseLimit; }

		/// Sets the gib impulse limit for this MOSRotating, i.e. the amount of impulse force required in a frame to gib this MOSRotating.
		/// @param newGibImpulseLimit The new gib impulse limit to use.
		void SetGibImpulseLimit(float newGibImpulseLimit) { m_GibImpulseLimit = newGibImpulseLimit; }

		/// Gets the gib wound limit for this MOSRotating, i.e. the total number of wounds required to gib this MOSRotating. Does not include any Attachables.
		/// @return
		int GetGibWoundLimit() const { return GetGibWoundLimit(false, false, false); }

		/// Gets the gib wound limit for this MOSRotating, i.e. the total number of wounds required to gib this MOSRotating.
		/// Optionally adds the gib wound limits of Attachables (and their Attachables, etc.) that match the conditions set by the provided parameters.
		/// @param includePositiveDamageAttachables Whether to count wounds from Attachables that have a positive damage multiplier, i.e. those that damage their parent (this MOSRotating) when wounded.
		/// @param includeNegativeDamageAttachables Whether to count wounds from Attachables that have a negative damage multiplier, i.e. those that heal their parent (this MOSRotating) when wounded.
		/// @param includeNoDamageAttachables Whether to count wounds from Attachables that a zero damage multiplier, i.e. those that do not affect their parent (this MOSRotating) when wounded.
		/// @return The wound limit of this MOSRotating and, optionally, its Attachables.
		int GetGibWoundLimit(bool includePositiveDamageAttachables, bool includeNegativeDamageAttachables, bool includeNoDamageAttachables) const;

		/// Sets the gib wound limit for this MOSRotating, i.e. the total number of wounds required to gib this MOSRotating.
		/// This will not directly trigger gibbing, even if the limit is lower than the current number of wounds.
		/// @param newLimit The new gib wound limit to use.
		void SetGibWoundLimit(int newGibWoundLimit) { m_GibWoundLimit = newGibWoundLimit; }

		/// Gets the rate at which wound count of this MOSRotating will diminish the impulse limit.
		/// @return The rate at which wound count affects the impulse limit.
		float GetWoundCountAffectsImpulseLimitRatio() const { return m_WoundCountAffectsImpulseLimitRatio; }

		/// Gets whether this MOSRotating should gib at the end of its lifetime instead of just being deleted.
		/// @return Whether this MOSRotating should gib at the end of its lifetime instead of just being deleted.
		bool GetGibAtEndOfLifetime() const { return m_GibAtEndOfLifetime; }

		/// Sets whether this MOSRotating should gib at the end of its lifetime instead of just being deleted.
		/// @param shouldGibAtEndOfLifetime Whether or not this MOSRotating should gib at the end of its lifetime instead of just being deleted.
		void SetGibAtEndOfLifetime(bool shouldGibAtEndOfLifetime) { m_GibAtEndOfLifetime = shouldGibAtEndOfLifetime; }

		/// Gets the gib blast strength this MOSRotating, i.e. the strength with which Gibs and Attachables will be launched when this MOSRotating is gibbed.
		/// @return The gib blast strength of this MOSRotating.
		float GetGibBlastStrength() const { return m_GibBlastStrength; }

		/// Sets the gib blast strength this MOSRotating, i.e. the strength with which Gibs and Attachables will be launched when this MOSRotating is gibbed.
		/// @param newGibBlastStrength The new gib blast strength to use.
		void SetGibBlastStrength(float newGibBlastStrength) { m_GibBlastStrength = newGibBlastStrength; }

		/// Gets the amount of screenshake this will cause upon gibbing.
		/// @return The amount of screenshake this will cause when gibbing. If -1, this is calculated automatically.
		float GetGibScreenShakeAmount() const { return m_GibScreenShakeAmount; }

		/// Gets a const reference to the list of Attachables on this MOSRotating.
		/// @return A const reference to the list of Attachables on this MOSRotating.
		const std::list<Attachable*>& GetAttachableList() const { return m_Attachables; }

		/// Gets a const reference to the list of wounds on this MOSRotating.
		/// @return A const reference to the list of wounds on this MOSRotating.
		const std::vector<AEmitter*>& GetWoundList() const { return m_Wounds; }

		/// Gets the number of wounds attached to this MOSRotating.
		/// Includes any Attachables (and their Attachables, etc.) that have a positive damage multiplier.
		/// @return The number of wounds on this MOSRotating.
		int GetWoundCount() const { return GetWoundCount(true, false, false); }

		/// Gets the number of wounds attached to this MOSRotating.
		/// Optionally adds the wound counts of Attachables (and their Attachables, etc.) that match the conditions set by the provided parameters.
		/// @param includePositiveDamageAttachables Whether to count wounds from Attachables that have a positive damage multiplier, i.e. those that damage their parent (this MOSRotating) when wounded.
		/// @param includeNegativeDamageAttachables Whether to count wounds from Attachables that have a negative damage multiplier, i.e. those that heal their parent (this MOSRotating) when wounded.
		/// @param includeNoDamageAttachables Whether to count wounds from Attachables that a zero damage multiplier, i.e. those that do not affect their parent (this MOSRotating) when wounded.
		/// @return The number of wounds on this MOSRotating and, optionally, its Attachables.
		int GetWoundCount(bool includePositiveDamageAttachables, bool includeNegativeDamageAttachables, bool includeNoDamageAttachables) const;

		/// Adds the passed in wound AEmitter to the list of wounds and changes its parent offset to the passed in Vector.
		/// @param woundToAdd The wound AEmitter to add.
		/// @param parentOffsetToSet The vector to set as the wound AEmitter's parent offset.
		/// @param checkGibWoundLimit Whether to gib this MOSRotating if adding this wound raises its wound count past its gib wound limit. Defaults to true.
		virtual void AddWound(AEmitter* woundToAdd, const Vector& parentOffsetToSet, bool checkGibWoundLimit = true);

		/// Removes the specified number of wounds from this MOSRotating, and returns damage caused by these removed wounds.
		/// Includes any Attachables (and their Attachables, etc.) that have a positive damage multiplier.
		/// @param numberOfWoundsToRemove The number of wounds that should be removed.
		/// @return The amount of damage caused by these wounds, taking damage multipliers into account.
		virtual float RemoveWounds(int numberOfWoundsToRemove) { return RemoveWounds(numberOfWoundsToRemove, true, false, false); }

		/// Removes the specified number of wounds from this MOSRotating, and returns damage caused by these removed wounds.
		/// Optionally removes wounds from Attachables (and their Attachables, etc.) that match the conditions set by the provided inclusion parameters.
		/// @param numberOfWoundsToRemove The number of wounds that should be removed.
		/// @param includePositiveDamageAttachables Whether to count wounds from Attachables that have a positive damage multiplier, i.e. those that damage their parent (this MOSRotating) when wounded.
		/// @param includeNegativeDamageAttachables Whether to count wounds from Attachables that have a negative damage multiplier, i.e. those that heal their parent (this MOSRotating) when wounded.
		/// @param includeNoDamageAttachables Whether to count wounds from Attachables that a zero damage multiplier, i.e. those that do not affect their parent (this MOSRotating) when wounded.
		/// @return The amount of damage caused by these wounds, taking damage multipliers into account.
		virtual float RemoveWounds(int numberOfWoundsToRemove, bool includePositiveDamageAttachables, bool includeNegativeDamageAttachables, bool includeNoDamageAttachables);

		/// Cleans up and destroys the script state of this object, calling the Destroy callback in lua
		void DestroyScriptState();

		/// Sets damage multiplier of this attachable.
		/// @param newValue New multiplier value.
		void SetDamageMultiplier(float newValue) {
			m_DamageMultiplier = newValue;
			m_NoSetDamageMultiplier = false;
		}

		/// Returns damage multiplier of this attachable.
		/// @return Current multiplier value.
		float GetDamageMultiplier() const { return m_DamageMultiplier; }

		/// Gets whether the damage multiplier for this MOSRotating has been directly set, or is at its default value.
		/// @return Whether the damage multiplier for this MOSRotating has been set.
		bool HasNoSetDamageMultiplier() const { return m_NoSetDamageMultiplier; }

		/// Gets the velocity orientation scalar of this MOSRotating.
		/// @return New scalar value.
		float GetOrientToVel() const { return m_OrientToVel; }

		/// Sets the velocity orientation scalar of this MOSRotating.
		/// @param newValue New scalar value.
		void SetOrientToVel(float newValue) { m_OrientToVel = newValue; }

		/// Sets this MOSRotating and all its children to drawn white for a specified amount of time.
		/// @param durationMS Duration of flash in real time MS.
		void FlashWhite(int durationMS = 32) {
			m_FlashWhiteTimer.SetRealTimeLimitMS(durationMS);
			m_FlashWhiteTimer.Reset();
		}

		/// Retrurns the amount of impulse force exerted on this during the last frame.
		/// @return The amount of impulse force exerted on this during the last frame.
		Vector GetTravelImpulse() const { return m_TravelImpulse; }

		/// Sets the amount of impulse force exerted on this during the last frame.
		/// @param impulse New impulse value
		void SetTravelImpulse(Vector impulse) { m_TravelImpulse = impulse; }

		/// Gets this MOSRotating's gib sound. Ownership is NOT transferred!
		/// @return The SoundContainer for this MOSRotating's gib sound.
		SoundContainer* GetGibSound() const { return m_GibSound; }

		/// Sets this MOSRotating's gib sound. Ownership IS transferred!
		/// @param newSound The new SoundContainer for this MOSRotating's gib sound.
		void SetGibSound(SoundContainer* newSound) { m_GibSound = newSound; }

		/// Ensures all attachables and wounds are positioned and rotated correctly. Must be run when this MOSRotating is added to MovableMan to avoid issues with Attachables spawning in at (0, 0).
		virtual void CorrectAttachableAndWoundPositionsAndRotations() const;

		/// Method to be run when the game is saved via ActivityMan::SaveCurrentGame. Not currently used in metagame or editor saving.
		void OnSave() override;

		/// Protected member variable and method declarations
	protected:
		/// Transfers forces and impulse forces from the given Attachable to this MOSRotating, gibbing and/or removing the Attachable if needed.
		/// @param attachable A pointer to the Attachable to apply forces from. Ownership is NOT transferred!
		/// @return Whether or not the Attachable has been removed, in which case it'll usually be passed to MovableMan.
		bool TransferForcesFromAttachable(Attachable* attachable);

		/// Makes this MO register itself and all its attached children in the
		/// MOID register and get ID:s for itself and its children for this frame.
		/// @param MOIDIndex The MOID index to register itself and its children in.
		/// @param rootMOID The MOID of the root MO of this MO, ie the highest parent of this MO. (default: g_NoMOID)
		/// 0 means that this MO is the root, ie it is owned by MovableMan.
		/// @param makeNewMOID Whether this MO should make a new MOID to use for itself, or to use (default: true)
		/// the same as the last one in the index (presumably its parent),
		void UpdateChildMOIDs(std::vector<MovableObject*>& MOIDIndex, MOID rootMOID = g_NoMOID, bool makeNewMOID = true) override;

		/// Creates the particles specified by this MOSRotating's list of Gibs and adds them to MovableMan with appropriately randomized velocities, based on this MOSRotating's gib blast strength.
		/// @param impactImpulse The impulse (kg * m/s) of the impact that caused the gibbing to happen.
		/// @param movableObjectToIgnore A pointer to an MO which the Attachables should not be colliding with.
		void CreateGibsWhenGibbing(const Vector& impactImpulse, MovableObject* movableObjectToIgnore);

		/// Removes all Attachables from this MOSR, deleting them or adding them to MovableMan as appropriate, and giving them randomized velocities based on their properties and this MOSRotating's gib blast strength.
		/// @param impactImpulse The impulse (kg * m/s) of the impact that caused the gibbing to happen.
		/// @param movableObjectToIgnore A pointer to an MO which the Attachables should not be colliding with.
		void RemoveAttachablesWhenGibbing(const Vector& impactImpulse, MovableObject* movableObjectToIgnore);

		// Member variables
		static Entity::ClassInfo m_sClass;
		//    float m_Torque; // In kg * r/s^2 (Newtons).
		//    float m_ImpulseTorque; // In kg * r/s.
		// The group of Atom:s that will be the physical reperesentation of this MOSRotating.
		AtomGroup* m_pAtomGroup;
		// The group of Atom:s that will serve as a means to detect deep terrain penetration.
		AtomGroup* m_pDeepGroup;
		// Whether or not to check for deep penetrations.
		bool m_DeepCheck;
		// A trigger for forcing a deep check to happen
		bool m_ForceDeepCheck;
		// Whether deep penetration happaned in the last frame or not, and how hard it was.
		float m_DeepHardness;
		// The amount of impulse force exerted on this during the last frame.
		Vector m_TravelImpulse;
		// The precomupted center location of the sprite relative to the MovableObject::m_Pos.
		Vector m_SpriteCenter;
		// How much to orient the rotation of this to match the velocity vector each frame 0 = none, 1.0 = immediately align with vel vector
		float m_OrientToVel;
		// Whether the SpriteMO is currently pushed back by recoil or not.
		bool m_Recoiled;
		// The impulse force in kg * m/s that represents the recoil.
		Vector m_RecoilForce;
		// The vector that the recoil offsets the sprite when m_Recoiled is true.
		Vector m_RecoilOffset;
		// The list of wound AEmitters currently attached to this MOSRotating, and owned here as well.
		std::vector<AEmitter*> m_Wounds;
		// The list of Attachables currently attached and Owned by this.
		std::list<Attachable*> m_Attachables;
		std::unordered_set<unsigned long> m_ReferenceHardcodedAttachableUniqueIDs; //!< An unordered set is filled with the Unique IDs of all of the reference object's hardcoded Attachables when using the copy Create.
		std::unordered_map<unsigned long, std::function<void(MOSRotating*, Attachable*)>> m_HardcodedAttachableUniqueIDsAndSetters; //!< An unordered map of Unique IDs to setter lambda functions, used to call the appropriate hardcoded Attachable setter when a hardcoded Attachable is removed.
		std::unordered_map<unsigned long, std::function<void(MOSRotating*, Attachable*)>> m_HardcodedAttachableUniqueIDsAndRemovers; //!< An unordered map of Unique IDs to remove lambda functions, used to call the appropriate hardcoded Attachable remover when a hardcoded Attachable is removed and calling the setter with nullptr won't work.
		const Attachable* m_RadiusAffectingAttachable; //!< A pointer to the Attachable that is currently affecting the radius. Used for some efficiency benefits.
		float m_FarthestAttachableDistanceAndRadius; //!< The distance + radius of the radius affecting Attachable.
		float m_AttachableAndWoundMass; //!< The mass of all Attachables and wounds on this MOSRotating. Used in combination with its actual mass and any other affecting factors to get its total mass.
		// The list of Gib:s this will create when gibbed
		std::list<Gib> m_Gibs;
		// The amount of impulse force required to gib this, in kg * (m/s). 0 means no limit
		float m_GibImpulseLimit;
		int m_GibWoundLimit; //!< The number of wounds that will gib this MOSRotating. 0 means that it can't be gibbed via wounds.
		float m_GibBlastStrength; //!< The strength with which Gibs and Attachables will get launched when this MOSRotating is gibbed.
		float m_GibScreenShakeAmount; //!< Determines how much screenshake this causes upon gibbing.
		float m_WoundCountAffectsImpulseLimitRatio; //!< The rate at which this MOSRotating's wound count will diminish the impulse limit.
		bool m_DetachAttachablesBeforeGibbingFromWounds; //!< Whether to detach any Attachables of this MOSRotating when it should gib from hitting its wound limit, instead of gibbing the MOSRotating itself.
		bool m_GibAtEndOfLifetime; //!< Whether or not this MOSRotating should gib when it reaches the end of its lifetime, instead of just deleting.
		// Gib sound effect
		SoundContainer* m_GibSound;
		// Whether to flash effect on gib
		bool m_EffectOnGib;
		// How far this is audiable (in screens) when gibbing
		float m_LoudnessOnGib;

		float m_DamageMultiplier; //!< Damage multiplier for this MOSRotating.
		bool m_NoSetDamageMultiplier; //!< Whether or not the damage multiplier for this MOSRotating was set.

		Timer m_FlashWhiteTimer; //!< The timer for timing white draw mode duration.

		// Intermediary drawing bitmap used to flip rotating bitmaps. Owned!
		BITMAP* m_pFlipBitmap;
		BITMAP* m_pFlipBitmapS;
		// Intermediary drawing bitmap used to draw sihouettes and other effects. Not owned; points to the shared static bitmaps
		BITMAP* m_pTempBitmap;
		// Temp drawing bitmaps shared between all MOSRotatings
		static BITMAP* m_spTempBitmap16;
		static BITMAP* m_spTempBitmap32;
		static BITMAP* m_spTempBitmap64;
		static BITMAP* m_spTempBitmap128;
		static BITMAP* m_spTempBitmap256;
		static BITMAP* m_spTempBitmap512;

		// Intermediary drawing bitmap used to draw MO silhouettes. Not owned; points to the shared static bitmaps
		BITMAP* m_pTempBitmapS;
		// Temp drawing bitmaps shared between all MOSRotatings
		static BITMAP* m_spTempBitmapS16;
		static BITMAP* m_spTempBitmapS32;
		static BITMAP* m_spTempBitmapS64;
		static BITMAP* m_spTempBitmapS128;
		static BITMAP* m_spTempBitmapS256;
		static BITMAP* m_spTempBitmapS512;

		/// Private member variable and method declarations
	private:
		/// Clears all the member variables of this MOSRotating, effectively
		/// resetting the members of this abstraction level only.
		void Clear();

		// Disallow the use of some implicit methods.
		MOSRotating(const MOSRotating& reference) = delete;
		MOSRotating& operator=(const MOSRotating& rhs) = delete;
	};

} // namespace RTE
