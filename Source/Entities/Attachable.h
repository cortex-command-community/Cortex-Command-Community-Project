#pragma once

#include "MOSRotating.h"
#include "PieSlice.h"

namespace RTE {

	class AEmitter;

	/// An articulated, detachable part of an Actor's body.
	class Attachable : public MOSRotating {
		friend class MOSRotating;

	public:
		EntityAllocation(Attachable);
		AddScriptFunctionNames(MOSRotating, "OnAttach", "OnDetach");
		SerializableOverrideMethods;
		ClassInfoGetters;

#pragma region Creation
		/// Constructor method used to instantiate a Attachable object in system memory. Create() should be called before using the object.
		Attachable();

		/// Makes the Attachable object ready for use.
		/// @return An error return value signaling success or any particular failure. Anything below 0 is an error signal.
		int Create() override;

		/// Creates an Attachable to be identical to another, by deep copy.
		/// @param reference A reference to the Attachable to deep copy.
		/// @return An error return value signaling success or any particular failure. Anything below 0 is an error signal.
		int Create(const Attachable& reference);
#pragma endregion

#pragma region Destruction
		/// Destructor method used to clean up an Attachable object before deletion from system memory.
		~Attachable() override;

		/// Destroys and resets (through Clear()) the Attachable object.
		/// @param notInherited Whether to only destroy the members defined in this derived class, or to destroy all inherited members also.
		void Destroy(bool notInherited = false) override {
			if (!notInherited) {
				MOSRotating::Destroy();
			}
			Clear();
		}

		/// Resets the entire Attachable, including its inherited members, to their default settings or values.
		void Reset() override {
			Clear();
			MOSRotating::Reset();
		}
#pragma endregion

#pragma region Parent Getters and Setters
		/// Gets the MOSRotating which is the parent of this Attachable.
		/// @return A pointer to the parent of this Attachable.
		MOSRotating* GetParent() override { return m_Parent; }

		/// Gets the MOSRotating which is the parent of this Attachable.
		/// @return A pointer to the parent of this Attachable.
		const MOSRotating* GetParent() const override { return m_Parent; }

		/// Indicates whether this Attachable is attached to an MOSRotating parent or not.
		/// @return Whether it's attached or not.
		bool IsAttached() const { return m_Parent != nullptr; }

		/// Indicates whether this Attachable is attached to the specified MOSRotating or not.
		/// @param parentToCheck A pointer to which MOSRotating you want to check for.
		/// @return Whether it's attached or not.
		bool IsAttachedTo(const MOSRotating* parentToCheck) const { return m_Parent == parentToCheck; }

		/// Gets the MO which is the ultimate root parent of this Attachable and its parent.
		/// @return A pointer to the highest root parent of this Attachable.
		MovableObject* GetRootParent() override { return m_Parent ? m_Parent->GetRootParent() : this; }

		/// Gets the MO which is the ultimate root parent of this Attachable and its parent.
		/// @return A pointer to the highest root parent of this Attachable.
		const MovableObject* GetRootParent() const override { return m_Parent ? m_Parent->GetRootParent() : this; }

		/// Gets the stored offset between this Attachable's parent's position and the joint position. This should be maintained by the parent.
		/// @return A const reference Vector describing the offset from the parent's position to the joint position.
		const Vector& GetParentOffset() const { return m_ParentOffset; }

		/// Sets the stored offset between this Attachable's parent's Pos and the joint position. This should be maintained by the parent.
		/// @param newParentOffset A const reference to the new parent offset.
		void SetParentOffset(const Vector& newParentOffset) { m_ParentOffset = newParentOffset; }

		/// Gets whether this Attachable is to be drawn after (in front of) or before (behind) its parent.
		/// @return Whether this Attachable is to be drawn after its parent or not.
		bool IsDrawnAfterParent() const override { return m_DrawAfterParent; }

		/// Sets whether this Attachable is to be drawn after (in front of) or before (behind) its parent.
		/// @param drawAfterParent Whether this Attachable is to be drawn after its parent.
		void SetDrawnAfterParent(bool drawAfterParent) { m_DrawAfterParent = drawAfterParent; }

		/// Gets whether this Attachable should be drawn normally by its parent.
		/// Some attachables (e.g. AEmitter flashes) require custom handling for when they should or shouldn't draw, to be done by the specific parent class.
		/// @return Whether this Attachable should be drawn normally by its parent.
		bool IsDrawnNormallyByParent() const { return m_DrawnNormallyByParent; }

		/// Sets whether this Attachable should be drawn normally by its parent.
		/// Some attachables (e.g. AEmitter flashes) require custom handling for when they should or shouldn't draw, to be done by the specific parent class.
		/// @param drawnNormallyByParent Whether this Attachable should be drawn normally by its parent.
		void SetDrawnNormallyByParent(bool drawnNormallyByParent) { m_DrawnNormallyByParent = drawnNormallyByParent; }

		/// Gets whether this Attachable will be deleted when removed from its parent. Has no effect until the Attachable has been added to a parent.
		/// @return Whether this Attachable is marked to be deleted when removed from its parent or not.
		bool GetDeleteWhenRemovedFromParent() const { return m_DeleteWhenRemovedFromParent; }

		/// Sets whether this Attachable will be deleted when removed from its parent.
		/// @param deleteWhenRemovedFromParent Whether this Attachable should be deleted when removed from its parent.
		void SetDeleteWhenRemovedFromParent(bool deleteWhenRemovedFromParent) { m_DeleteWhenRemovedFromParent = deleteWhenRemovedFromParent; }

		/// Gets whether this Attachable will gib when removed from its parent. Has no effect until the Attachable has been added to a parent.
		/// @return Whether this Attachable is marked to gib when removed from its parent or not.
		bool GetGibWhenRemovedFromParent() const { return m_GibWhenRemovedFromParent; }

		/// Sets whether this Attachable will gib when removed from its parent.
		/// @param gibWhenRemovedFromParent Whether this Attachable should gib when removed from its parent.
		void SetGibWhenRemovedFromParent(bool gibWhenRemovedFromParent) { m_GibWhenRemovedFromParent = gibWhenRemovedFromParent; }

		/// Gets whether forces transferred from this Attachable should be applied at its parent's offset (rotated to match the parent) where they will produce torque, or directly at its parent's position.
		/// @return Whether forces transferred from this Attachable should be applied at an offset.
		bool GetApplyTransferredForcesAtOffset() const { return m_ApplyTransferredForcesAtOffset; }

		/// Sets whether forces transferred from this Attachable should be applied at its parent's offset (rotated to match the parent) where they will produce torque, or directly at its parent's position.
		/// @param appliesTransferredForcesAtOffset Whether forces transferred from this Attachable should be applied at an offset.
		void SetApplyTransferredForcesAtOffset(bool appliesTransferredForcesAtOffset) { m_ApplyTransferredForcesAtOffset = appliesTransferredForcesAtOffset; }
#pragma endregion

#pragma region Parent Gib Handling Getters and Setters
		/// Gets the percentage chance that this Attachable will gib when its parent does. 0 means never, 1 means always.
		/// @return A float with the percentage chance this Attachable will gib when its parent gibs.
		float GetGibWithParentChance() const { return m_GibWithParentChance; }

		/// Sets the percentage chance that this Attachable will gib when its parent does. 0 means never, 1 means always.
		/// @param gibWithParentChance A float describing the percentage chance this Attachable will gib when its parent gibs.
		void SetGibWithParentChance(float gibWithParentChance) { m_GibWithParentChance = gibWithParentChance; }

		/// Gets the multiplier for how strongly this Attachable's parent's gib blast strength will be applied to it when its parent's gibs
		/// @return A float with the parent gib blast strength multiplier of this Attachable.
		float GetParentGibBlastStrengthMultiplier() const { return m_ParentGibBlastStrengthMultiplier; }

		/// Sets the multiplier for how strongly this Attachable's parent's gib blast strength will be applied to it when its parent's gibs
		/// @param parentGibBlastStrengthMultiplier A float describing the parent gib blast strength multiplier of this Attachable.
		void SetParentGibBlastStrengthMultiplier(float parentGibBlastStrengthMultiplier) { m_ParentGibBlastStrengthMultiplier = parentGibBlastStrengthMultiplier; }
#pragma endregion

#pragma region Temporary Handling for Wounds, to be Replaced by a Wound Object in Future
		/// Gets whether or not this Attachable is a wound, as determined by its parent MOSR.
		/// @return Whether or not this Attachable is a wound.
		bool IsWound() const { return m_IsWound; }

		/// Sets whether or not this Attachable is a wound, to be done by its parent MOSR.
		/// @param isWound Whether or not this Attachable should be a wound.
		void SetIsWound(bool isWound) { m_IsWound = isWound; }
#pragma endregion

#pragma region Joint Getters and Setters
		/// Gets the amount of impulse force the joint of this Attachable can handle before breaking.
		/// @return A float with the max tolerated impulse force in kg * m/s.
		float GetJointStrength() const { return m_JointStrength; }

		/// Sets the amount of impulse force the joint of this Attachable can handle before breaking.
		/// @param jointStrength A float describing the max tolerated impulse force in Newtons (kg * m/s).
		void SetJointStrength(float jointStrength) { m_JointStrength = jointStrength; }

		/// Gets the stiffness scalar of the joint of this Attachable, normalized between 0 and 1.0.
		/// 1.0 means impulse forces on this attachable will be transferred to the parent with 100% strength, 0 means they will not transfer at all.
		/// @return The normalized stiffness scalar of this Attachable's joint.
		float GetJointStiffness() const { return m_JointStiffness; }

		/// Sets the stiffness scalar of the joint of this Attachable, limited between 0 and 1.0.
		/// 1.0 means impulse forces on this attachable will be transferred to the parent with 100% strength, 0 means they will not transfer at all.
		/// @param jointStiffness A float describing the normalized stiffness scalar of this Attachable's joint. It will automatically be limited between 0 and 1.0.
		virtual void SetJointStiffness(float jointStiffness) { m_JointStiffness = std::clamp(jointStiffness, 0.0F, 1.0F); }

		/// Gets the offset of the joint (the point around which this Attachable and its parent hinge) from this Attachable's center of mass/origin.
		/// @return A const reference Vector describing the offset of the joint relative to this Attachable's origin/center of mass position.
		const Vector& GetJointOffset() const { return m_JointOffset; }

		/// Sets the offset of the joint (the point around which this Attachable and its parent hinge) from this Attachable's center of mass/origin.
		/// @param newJointOffset A Vector describing the offset of the joint relative to the this Attachable's origin/center of mass position.
		void SetJointOffset(const Vector& newJointOffset) { m_JointOffset = newJointOffset; }

		/// Gets the absolute position of the joint that the parent of this Attachable sets upon Update().
		/// @return A Vector describing the current absolute position of the joint.
		const Vector& GetJointPos() const { return m_JointPos; }
#pragma endregion

#pragma region Force Transferral
		/// Bundles up all the accumulated forces of this Attachable and calculates how they transfer to the joint, and therefore to the parent.
		/// If the accumulated forces exceed the joint strength of this Attachable, the jointForces Vector will be filled to the limit and false will be returned.
		/// Additionally, in this case, the Attachable will remove itself from its parent.
		/// @param jointForces A vector that will have the forces affecting the joint ADDED to it.
		/// @return False if the Attachable has no parent or its accumulated forces are greater than its joint strength, otherwise true.
		bool TransferJointForces(Vector& jointForces);

		/// Bundles up all the accumulated impulse forces of this Attachable and calculates how they transfer to the joint, and therefore to the parent.
		/// If the accumulated impulse forces exceed the joint strength or gib impulse limit of this Attachable, the jointImpulses Vector will be filled up to that limit and false will be returned.
		/// Additionally, in this case, the Attachable will remove itself from its parent and gib itself if appropriate.
		/// @param jointImpulses A vector that will have the impulse forces affecting the joint ADDED to it.
		/// @param jointStiffnessValueToUse An optional override for the Attachable's joint stiffness for this function call. Primarily used to allow subclasses to perform special behavior.
		/// @param jointStrengthValueToUse An optional override for the Attachable's joint strength for this function call. Primarily used to allow subclasses to perform special behavior.
		/// @param gibImpulseLimitValueToUse An optional override for the Attachable's gib impulse limit for this function call. Primarily used to allow subclasses to perform special behavior.
		/// @return False if the Attachable has no parent or its accumulated forces are greater than its joint strength or gib impulse limit, otherwise true.
		virtual bool TransferJointImpulses(Vector& jointImpulses, float jointStiffnessValueToUse = -1, float jointStrengthValueToUse = -1, float gibImpulseLimitValueToUse = -1);
#pragma endregion

#pragma region Damage and Wound Management
		/// Adds the specified number of damage points to this attachable.
		/// @param damageAmount The amount of damage to add.
		void AddDamage(float damageAmount) { m_DamageCount += damageAmount; }

		/// Calculates the amount of damage this Attachable has sustained since the last time this method was called and returns it, modified by the Attachable's damage multiplier.
		/// This should normally be called AFTER updating this Attachable in order to get the correct damage for a given frame.
		/// @return A float with the damage accumulated, multiplied by the Attachable's damage multiplier, since the last time this method was called.
		float CollectDamage();

		/// Gets the AEmitter that represents the wound added to this Attachable when it gets detached from its parent. OWNERSHIP IS NOT TRANSFERRED!
		/// @return A const pointer to the break wound AEmitter.
		const AEmitter* GetBreakWound() const { return m_BreakWound; }

		/// Sets the AEmitter that represents the wound added to this Attachable when it gets detached from its parent. OWNERSHIP IS NOT TRANSFERRED!
		/// @param breakWound The AEmitter to use for this Attachable's breakwound.
		void SetBreakWound(AEmitter* breakWound) { m_BreakWound = breakWound; }

		/// Gets the AEmitter that represents the wound added to this Attachable's parent when this Attachable gets detached from its parent. OWNERSHIP IS NOT TRANSFERRED!
		/// @return A const pointer to the parent break wound AEmitter.
		const AEmitter* GetParentBreakWound() const { return m_ParentBreakWound; }

		/// Sets the AEmitter that represents the wound added to this Attachable's parent when this Attachable gets detached from its parent. OWNERSHIP IS NOT TRANSFERRED!
		/// @param breakWound The AEmitter to use for the parent's breakwound.
		void SetParentBreakWound(AEmitter* breakWound) { m_ParentBreakWound = breakWound; }
#pragma endregion

#pragma region Inherited Value Getters and Setters
		/// Gets whether or not this Attachable inherits its parent's HFlipped value, i.e. whether it has its HFlipped value reset to match/reverse its parent's every frame, if attached.
		/// 0 means no inheritance, 1 means normal inheritance, anything else means reversed inheritance (i.e. if the parent's HFlipped value is true, this Attachable's HFlipped value will be false).
		/// @return Whether or not this Attachable inherits its parent's HFlipped value.
		int InheritsHFlipped() const { return m_InheritsHFlipped; }

		/// Sets whether or not this Attachable inherits its parent's HFlipped value, i.e. whether it has its HFlipped value reset to match/reverse its parent's every frame, if attached.
		/// 0 means no inheritance, 1 means normal inheritance, anything else means reversed inheritance (i.e. if the parent's HFlipped value is true, this Attachable's HFlipped value will be false).
		/// @param inheritsRotAngle Whether or not to inherit its parent's HFlipped value.
		void SetInheritsHFlipped(int inheritsHFlipped) { m_InheritsHFlipped = inheritsHFlipped; }

		/// Gets whether or not this Attachable inherits its RotAngle from its parent, i.e. whether it has its RotAngle reset to match its parent every frame, if attached.
		/// @return Whether or not this Attachable inherits its parent's RotAngle.
		bool InheritsRotAngle() const { return m_InheritsRotAngle; }

		/// Sets whether or not this Attachable inherits its RotAngle from its parent, i.e. whether it has its RotAngle reset to match its parent every frame, if attached.
		/// @param inheritsRotAngle Whether or not to inherit its parent's RotAngle.
		void SetInheritsRotAngle(bool inheritsRotAngle) { m_InheritsRotAngle = inheritsRotAngle; }

		/// Gets the offset of this Attachable's rotation angle from its parent. Only actually applied if the Attachable is set to inherit its parent's rotation angle.
		/// @return This Attachable's inherited rotation angle offset.
		float GetInheritedRotAngleOffset() const { return m_InheritedRotAngleOffset; }

		/// Sets the offset of this Attachable's rotation angle from its parent. Only actually applied if the Attachable is set to inherit its parent's rotation angle.
		/// @param inheritedRotAngleOffset Thee new rotation angle offset for this Attachable.
		void SetInheritedRotAngleOffset(float inheritedRotAngleOffset) { m_InheritedRotAngleOffset = inheritedRotAngleOffset; }

		/// Gets whether or not this Attachable inherits its Frame from its parent, if attached.
		/// @return Whether or not this Attachable inherits its parent's Frame.
		bool InheritsFrame() const { return m_InheritsFrame; }

		/// Sets whether or not this Attachable inherits its Frame from its parent, if attached.
		/// @param inheritsFrame Whether or not to inherit its parent's Frame.
		void SetInheritsFrame(bool inheritsFrame) { m_InheritsFrame = inheritsFrame; }
#pragma endregion

#pragma region Collision Management
		/// Gets the subgroup ID of this' Atoms.
		/// @return The subgroup ID of this' Atoms.
		long GetAtomSubgroupID() const { return m_AtomSubgroupID; }

		/// Sets the subgroup ID of this' Atoms
		/// @param newID A long describing the new subgroup id of this' Atoms.
		void SetAtomSubgroupID(long subgroupID = 0) { m_AtomSubgroupID = subgroupID; }

		/// Gets whether this Attachable currently has terrain collisions enabled and it's atoms are present in the parent AtomGroup.
		/// Attachables with Attachable parents that don't collide with terrain will not collide with terrain. This chains up to the root parent.
		/// @return If true, terrain collisions while attached are enabled and atoms are present in parent AtomGroup.
		bool GetCollidesWithTerrainWhileAttached() const { return m_CollidesWithTerrainWhileAttached; }

		/// Sets whether this Attachable currently has terrain collisions enabled and it's atoms are present in the parent AtomGroup.
		/// @param collidesWithTerrainWhileAttached Whether this attachable currently has terrain collisions enabled and it's atoms are present in the parent AtomGroup.
		void SetCollidesWithTerrainWhileAttached(bool collidesWithTerrainWhileAttached);

		/// Gets whether this Attachable is currently able to collide with terrain, taking into account its terrain collision settings and those of its parent and so on.
		/// @return Whether this Attachable is currently able to collide with terrain, taking into account its terrain collision settings and those of its parent and so on.
		bool CanCollideWithTerrain() const;

		/// Gets whether this Attachable currently ignores collisions with single-atom particles.
		/// @return >Whether this attachable ignores collisions with single-atom particles.
		bool GetIgnoresParticlesWhileAttached() const { return m_IgnoresParticlesWhileAttached; }

		/// Sets whether this Attachable currently ignores collisions with single-atom particles.
		/// @param collidesWithTerrainWhileAttached Whether this attachable ignores collisions with single-atom particles.
		void SetIgnoresParticlesWhileAttached(bool ignoresParticlesWhileAttached) { m_IgnoresParticlesWhileAttached = ignoresParticlesWhileAttached; }
#pragma endregion

#pragma region Override Methods
		/// Calculates the collision response when another MO's Atom collides with this MO's physical representation.
		/// The effects will be applied directly to this MO, and also represented in the passed in HitData.
		/// @param hitData Reference to the HitData struct which describes the collision. This will be modified to represent the results of the collision.
		/// @return Whether the collision has been deemed valid. If false, then disregard any impulses in the HitData.
		bool CollideAtPoint(HitData& hitData) override;

		/// Determines whether a particle which has hit this MO will penetrate, and if so, whether it gets lodged or exits on the other side of this MO.
		/// Appropriate effects will be determined and applied ONLY IF there was penetration! If not, nothing will be affected.
		/// @param hitData The HitData describing the collision in detail, the impulses have to have been filled out!
		/// @return
		/// Whether the particle managed to penetrate into this MO or not.
		/// If something other than an MOPixel or MOSParticle is being passed in as the hitor, false will trivially be returned here.
		bool ParticlePenetration(HitData& hitData) override;

		/// Destroys this Attachable and creates its specified Gibs in its place with appropriate velocities.
		/// Any Attachables are removed and also given appropriate velocities.
		/// @param impactImpulse The impulse (kg * m/s) of the impact causing the gibbing to happen.
		/// @param movableObjectToIgnore A pointer to an MO which the Gibs and Attachables should not be colliding with.
		void GibThis(const Vector& impactImpulse = Vector(), MovableObject* movableObjectToIgnore = nullptr) override;

		/// Checks if the given Attachable should affect radius, and handles it if it should.
		/// @param attachable The Attachable to check.
		/// @return Whether the radius affecting Attachable changed as a result of this call.
		bool HandlePotentialRadiusAffectingAttachable(const Attachable* attachable) override;

		/// Updates this Attachable's Lua scripts.
		/// @return An error return value signaling success or any particular failure. Anything below 0 is an error signal.
		int UpdateScripts() override;

		/// Updates this Attachable. Supposed to be done every frame.
		///	NOTE - Attachable subclasses that do things before calling Attachable::Update should make sure to call Attachable::PreUpdate.
		void Update() override;
#pragma endregion

		/// Pre-update method that should be run by all Attachable sub-classes that do things before calling Attachable::Update.
		void PreUpdate();

#pragma region Override Methods for Handling Mass
		/// Sets the mass of this Attachable.
		/// @param newMass A float specifying the new mass value in Kilograms (kg).
		void SetMass(const float newMass) final;

		/// Updates the total mass of Attachables and wounds for this Attachable, intended to be used when Attachables' masses get modified. Simply subtracts the old mass and adds the new one.
		/// @param oldAttachableOrWoundMass The mass the Attachable or wound had before its mass was modified.
		/// @param newAttachableOrWoundMass The up-to-date mass of the Attachable or wound after its mass was modified.
		void UpdateAttachableAndWoundMass(float oldAttachableOrWoundMass, float newAttachableOrWoundMass) final;

		/// Adds the passed in Attachable the list of Attachables and sets its parent to this Attachable.
		/// @param attachable The Attachable to add.
		void AddAttachable(Attachable* attachable) final { MOSRotating::AddAttachable(attachable); }

		/// Adds the passed in Attachable the list of Attachables, changes its parent offset to the passed in Vector, and sets its parent to this Attachable.
		/// @param attachable The Attachable to add.
		/// @param parentOffsetToSet The Vector to set as the Attachable's parent offset.
		void AddAttachable(Attachable* attachable, const Vector& parentOffsetToSet) final;

		/// Removes the Attachable corresponding to the passed in UniqueID and sets its parent to nullptr. Does not add it to MovableMan or add break wounds.
		/// @param attachableUniqueID The UniqueID of the Attachable to remove.
		/// @return A pointer to the removed Attachable. Ownership IS transferred!
		Attachable* RemoveAttachable(long attachableUniqueID) final { return MOSRotating::RemoveAttachable(attachableUniqueID); }

		/// Removes the Attachable corresponding to the passed in UniqueID and sets its parent to nullptr. Optionally adds it to MovableMan and/or adds break wounds.
		/// If the Attachable is not set to delete or delete when removed from its parent, and addToMovableMan is false, the caller must hang onto a pointer to the Attachable ahead of time to avoid memory leaks.
		/// @param attachableUniqueID The UniqueID of the Attachable to remove.
		/// @param addToMovableMan Whether or not to add the Attachable to MovableMan once it has been removed.
		/// @param addBreakWounds Whether or not to add break wounds to the removed Attachable and this Attachable.
		/// @return A pointer to the removed Attachable, if it wasn't added to MovableMan or nullptr if it was. Ownership IS transferred!
		Attachable* RemoveAttachable(long attachableUniqueID, bool addToMovableMan, bool addBreakWounds) final { return MOSRotating::RemoveAttachable(attachableUniqueID, addToMovableMan, addBreakWounds); }

		/// Removes the passed in Attachable and sets its parent to nullptr. Does not add it to MovableMan or add break wounds.
		/// @param attachable The Attachable to remove.
		/// @return A pointer to the removed Attachable. Ownership IS transferred!
		Attachable* RemoveAttachable(Attachable* attachable) final { return MOSRotating::RemoveAttachable(attachable); }

		/// Removes the passed in Attachable and sets its parent to nullptr. Optionally adds it to MovableMan and/or adds break wounds.
		/// If the Attachable is not set to delete or delete when removed from its parent, and addToMovableMan is false, the caller must hang onto a pointer to the Attachable ahead of time to avoid memory leaks.
		/// @param attachable The Attachable to remove.
		/// @param addToMovableMan Whether or not to add the Attachable to MovableMan once it has been removed.
		/// @param addBreakWounds Whether or not to add break wounds to the removed Attachable and this Attachable.
		/// @return A pointer to the removed Attachable, if it wasn't added to MovableMan or nullptr if it was. Ownership IS transferred!
		Attachable* RemoveAttachable(Attachable* attachable, bool addToMovableMan, bool addBreakWounds) final;

		/// Adds the passed in wound AEmitter to the list of wounds and changes its parent offset to the passed in Vector.
		/// @param woundToAdd The wound AEmitter to add.
		/// @param parentOffsetToSet The vector to set as the wound AEmitter's parent offset.
		/// @param checkGibWoundLimit Whether to gib this Attachable if adding this wound raises its wound count past its gib wound limit. Defaults to true.
		void AddWound(AEmitter* woundToAdd, const Vector& parentOffsetToSet, bool checkGibWoundLimit = true) final;

		/// Removes the specified number of wounds from this Attachable, and returns damage caused by these removed wounds.
		/// Includes any Attachables (and their Attachables, etc.) that have a positive damage multiplier.
		/// @param numberOfWoundsToRemove The number of wounds that should be removed.
		/// @return The amount of damage caused by these wounds, taking damage multipliers into account.
		float RemoveWounds(int numberOfWoundsToRemove) final { return MOSRotating::RemoveWounds(numberOfWoundsToRemove); }

		/// Removes the specified number of wounds from this Attachable, and returns damage caused by these removed wounds.
		/// Optionally removes wounds from Attachables (and their Attachables, etc.) that match the conditions set by the provided inclusion parameters.
		/// @param numberOfWoundsToRemove The number of wounds that should be removed.
		/// @param includePositiveDamageAttachables Whether to count wounds from Attachables that have a positive damage multiplier, i.e. those that damage their parent (this Attachable) when wounded.
		/// @param includeNegativeDamageAttachables Whether to count wounds from Attachables that have a negative damage multiplier, i.e. those that heal their parent (this Attachable) when wounded.
		/// @param includeNoDamageAttachables Whether to count wounds from Attachables that a zero damage multiplier, i.e. those that do not affect their parent (this Attachable) when wounded.
		/// @return The amount of damage caused by these wounds, taking damage multipliers into account.
		float RemoveWounds(int numberOfWoundsToRemove, bool includeAttachablesWithAPositiveDamageMultiplier, bool includeAttachablesWithANegativeDamageMultiplier, bool includeAttachablesWithNoDamageMultiplier) override;
#pragma endregion

	protected:
		static Entity::ClassInfo m_sClass; //!< ClassInfo for this class.

		MOSRotating* m_Parent; //!< Pointer to the MOSRotating this attachable is attached to.
		Vector m_ParentOffset; //!< The offset from the parent's Pos to the joint point this Attachable is attached with.
		bool m_DrawAfterParent; //!< Whether to draw this Attachable after (in front of) or before (behind) the parent.
		bool m_DrawnNormallyByParent; //!< Whether this Attachable will be drawn normally when attached, or will require special handling by some non-MOSR parent type.
		bool m_DeleteWhenRemovedFromParent; //!< Whether this Attachable should be deleted when removed from its parent.
		bool m_GibWhenRemovedFromParent; //!< Whether this Attachable should gib when removed from its parent.
		bool m_ApplyTransferredForcesAtOffset; //!< Whether forces transferred from this Attachable should be applied at the rotated parent offset (which will produce torque), or directly at the parent's position. Mostly useful to make jetpacks and similar emitters viable.

		float m_GibWithParentChance; //!< The percentage chance that this Attachable will gib when its parent does. 0 means never, 1 means always.
		float m_ParentGibBlastStrengthMultiplier; //!< The multiplier for how strongly this Attachable's parent's gib blast strength will be applied to it when its parent's gibs.

		// TODO This is a stopgap for a dedicated Wound class, that would be helpful to simplify things like this and default damage multiplier handling.
		bool m_IsWound; //!< Whether or not this Attachable has been added as a wound. Only set and applied for Attachables with parents.

		float m_JointStrength; //!< The amount of impulse force needed on this to detach it from the host Actor, in kg * m/s. A value of 0 means the join is infinitely strong and will never break naturally.
		float m_JointStiffness; //!< The normalized joint stiffness scalar. 1.0 means impulse forces on this attachable will be transferred to the parent with 100% strength, 0 means they will not transfer at all.
		Vector m_JointOffset; //!< The offset to the joint (the point around which this Attachable and its parent hinge) from its center of mass/origin.
		Vector m_JointPos; //!< The absolute position of the joint that the parent sets upon Update() if this Attachable is attached to it.

		float m_DamageCount; //!< The number of damage points that this Attachable has accumulated since the last time CollectDamage() was called.
		const AEmitter* m_BreakWound; //!< The wound this Attachable will receive when it breaks from its parent.
		const AEmitter* m_ParentBreakWound; //!< The wound this Attachable's parent will receive when the Attachable breaks from its parent.

		int m_InheritsHFlipped; //!< Whether this Attachable should inherit its parent's HFlipped. Defaults to 1 (normal inheritance).
		bool m_InheritsRotAngle; //!< Whether this Attachable should inherit its parent's RotAngle. Defaults to true.
		float m_InheritedRotAngleOffset; //!< The offset by which this Attachable should be rotated when it's set to inherit its parent's rotation angle. Defaults to 0.
		bool m_InheritsFrame; //!< Whether this Attachable should inherit its parent's Frame. Defaults to false.

		long m_AtomSubgroupID; //!< The Atom IDs this' atoms will have when attached and added to a parent's AtomGroup.
		bool m_CollidesWithTerrainWhileAttached; //!< Whether this attachable currently has terrain collisions enabled while it's attached to a parent.
		bool m_IgnoresParticlesWhileAttached; //!< Whether this Attachable should ignore collisions with single-atom MOs while attached.

		std::vector<std::unique_ptr<PieSlice>> m_PieSlices; //!< The vector of PieSlices belonging to this Attachable. Added to and removed from the RootParent as appropriate, when a parent is set.

		Vector m_PrevParentOffset; //!< The previous frame's parent offset.
		Vector m_PrevJointOffset; //!< The previous frame's joint offset.
		float m_PrevRotAngleOffset; //!< The previous frame's difference between this Attachable's RotAngle and it's root parent's RotAngle.
		bool m_PreUpdateHasRunThisFrame; //!< Whether or not PreUpdate has run this frame. PreUpdate, like Update, should only run once per frame.

		/// Sets this Attachable's parent MOSRotating, and also sets its Team based on its parent and, if the Attachable is set to collide, adds/removes Atoms to its new/old parent.
		/// @param newParent A pointer to the MOSRotating to set as the new parent. Ownership is NOT transferred!
		virtual void SetParent(MOSRotating* newParent);

	private:
		/// Updates the position of this Attachable based on its parent offset and joint offset. Used during update and when something sets these offsets through setters.
		void UpdatePositionAndJointPositionBasedOnOffsets();

		/// Turns on/off this Attachable's terrain collisions while it is attached by adding/removing its Atoms to/from its root parent's AtomGroup.
		/// @param addAtoms Whether to add this Attachable's Atoms to the root parent's AtomGroup or remove them.
		/// @param propagateToChildAttachables Whether this Atom addition or removal should be propagated to any child Attachables (as appropriate).
		void AddOrRemoveAtomsFromRootParentAtomGroup(bool addAtoms, bool propagateToChildAttachables);

		/// Add or removes this Attachable's PieSlices and PieMenu listeners, as well as those of any of its child Attachables, from the given PieMenu (should be the root parent's PieMenu).
		/// Note that listeners are only added for Attachables that have at least one script file with the appropriate Lua function.
		/// @param pieMenuToModify The PieMenu to modify, passed in to keep the recursion simple and clean.
		/// @param addToPieMenu Whether to add this Attachable's PieSlices and listeners to, or remove them from, the root parent's PieMenu.
		void AddOrRemovePieSlicesAndListenersFromPieMenu(PieMenu* pieMenuToModify, bool addToPieMenu);

		/// Clears all the member variables of this Attachable, effectively resetting the members of this abstraction level only.
		void Clear();

		// Disallow the use of some implicit methods.
		Attachable(const Attachable& reference) = delete;
		Attachable& operator=(const Attachable& rhs) = delete;
	};
} // namespace RTE
