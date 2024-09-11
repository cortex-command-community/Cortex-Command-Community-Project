#pragma once

#include "Atom.h"

namespace RTE {

	class MOSRotating;
	class LimbPath;

	/// A group of Atoms that move and interact with the terrain in unison.
	class AtomGroup : public Entity {

	public:
		EntityAllocation(AtomGroup);
		SerializableOverrideMethods;
		ClassInfoGetters;

#pragma region Creation
		/// Constructor method used to instantiate an AtomGroup object in system memory. Create() should be called before using the object.
		AtomGroup();

		/// Copy constructor method used to instantiate an AtomGroup object identical to an already existing one.
		/// @param reference An AtomGroup object which is passed in by reference.
		AtomGroup(const AtomGroup& reference);

		/// Makes the AtomGroup object ready for use.
		/// @return An error return value signaling success or any particular failure. Anything below 0 is an error signal.
		int Create() override;

		/// Creates an AtomGroup to be identical to another, by deep copy.
		/// @param reference A reference to the AtomGroup to deep copy.
		/// @return An error return value signaling success or any particular failure. Anything below 0 is an error signal.
		int Create(const AtomGroup& reference) { return Create(reference, false); }

		/// Creates an AtomGroup to be identical to another, by deep copy, with the option to only copy Atoms that belong to the reference AtomGroup's owner thereby excluding any Atom subgroups.
		/// @param reference A reference to the AtomGroup to deep copy.
		/// @param onlyCopyOwnerAtoms Whether or not to only copy Atoms that belong to the reference AtomGroup's owner directly.
		/// @return An error return value signaling success or any particular failure. Anything below 0 is an error signal.
		int Create(const AtomGroup& reference, bool onlyCopyOwnerAtoms);

		/// Creates an AtomGroup after the silhouette shape of a passed in MOSRotating by dotting the outline of the sprite with Atoms.
		/// The passed in MOSRotating will also be made the owner of this AtomGroup! Ownership of the MOSRotating is NOT transferred!
		/// @param ownerMOSRotating A pointer to a MOSRotating whose outline will be approximated by Atoms of this AtomGroup, and that will be set as the owner of this AtomGroup.
		/// @return An error return value signaling success or any particular failure. Anything below 0 is an error signal.
		int Create(MOSRotating* ownerMOSRotating) { return Create(ownerMOSRotating, m_Material, m_Resolution, m_Depth); }

		/// Creates an AtomGroup after the silhouette shape of a passed in MOSRotating by dotting the outline of the sprite with Atoms.
		/// The passed in MOSRotating will also be made the owner of this AtomGroup! Ownership of the MOSRotating is NOT transferred!
		/// @param ownerMOSRotating A pointer to an MOSRotating whose outline will be approximated by Atoms of this AtomGroup, and that will be set as the owner of this AtomGroup.
		/// @param material The Material that the Atoms of this AtomGroup should be.
		/// @param resolution Resolution, or density of the Atoms in representing the MOSRotating's outline. Lower value equals higher density.
		/// @param depth The depth into the sprite that the Atoms should be placed.
		/// @return An error return value signaling success or any particular failure. Anything below 0 is an error signal.
		int Create(MOSRotating* ownerMOSRotating, Material const* material, int resolution = 1, int depth = 0);
#pragma endregion

#pragma region Destruction
		/// Destructor method used to clean up an AtomGroup object before deletion from system memory.
		~AtomGroup() override;

		/// Destroys and resets (through Clear()) the AtomGroup object.
		/// @param notInherited Whether to only destroy the members defined in this derived class, or to destroy all inherited members also.
		void Destroy(bool notInherited = false) override;

		/// Resets the entire AtomGroup, including its inherited members, to their default settings or values.
		void Reset() override {
			Clear();
			Entity::Reset();
		}
#pragma endregion

#pragma region Getters and Setters
		/// Gets the current list of Atoms that make up the group.
		/// @return A const reference to the Atom list.
		const std::vector<Atom*>& GetAtomList() const { return m_Atoms; }

		/// Sets the a new list of Atoms that make up the group.
		/// @param newAtoms List of Atoms that make up the group.
		void SetAtomList(const std::vector<Atom*>& newAtoms);

		/// Gets the current number of Atoms that make up the group.
		/// @return The number of Atoms that make up the group.
		int GetAtomCount() const { return m_Atoms.size(); }

		/// Gets max radius of the AtomGroup through the longest magnitude of all the Atom's offsets.
		/// @return The largest magnitude of Atom's offsets, in pixels.
		float CalculateMaxRadius() const;

		/// Gets the current owner MOSRotating of this AtomGroup.
		/// @return A pointer to the owner.
		MOSRotating* GetOwner() const { return m_OwnerMOSR; }

		/// Sets the current owner MOSRotating of this AtomGroup.
		/// @param newOwner A pointer to the new owner. Ownership is NOT transferred!
		void SetOwner(MOSRotating* newOwner);

		/// Gets the Material of this AtomGroup.
		/// @return A const pointer to the Material.
		const Material* GetMaterial() const { return (m_Material) ? m_Material : g_SceneMan.GetMaterialFromID(g_MaterialAir); }

		/// Gets whether this AtomGroup's Atoms are to be automatically generated based on a bitmap, or manually specified.
		/// @return Whether this AtomGroup is auto generated from a bitmap or not.
		bool AutoGenerate() const { return m_AutoGenerate; }

		/// Gets the resolution (density of Atoms) of this AtomGroup. Higher values mean a less dense and less accurate physical representation of the owner MOSR's graphical representation.
		/// @return The resolution value of this AtomGroup. 0 means the Atoms in this AtomGroup were defined manually.
		int GetResolution() const { return m_Resolution; }

		/// Gets the depth Atoms in this AtomGroup are placed off the edge of the owning MOSR's graphical representation outline towards it's center.
		/// @return The depth, in pixels. If 0, Atoms are placed right on the edge of the MOSR outline.
		int GetDepth() const { return m_Depth; }

		/// Gets the offset of an Atom in this AtomGroup adjusted to the Owner MOSRotating horizontal flip and rotation.
		/// @param atom The individual Atom to get the offset for.
		/// @return The offset of an Atom in this AtomGroup adjusted to the Owner MOSRotating horizontal flip and rotation.
		Vector GetAdjustedAtomOffset(const Atom* atom) const;

		/// Gets the current position of this AtomGroup as a limb.
		/// @param hFlipped Whether to adjust the position for horizontal flip or not.
		/// @return The absolute limb position in the world.
		Vector GetLimbPos(bool hFlipped = false) const { return m_LimbPos.GetFloored() + m_JointOffset.GetXFlipped(hFlipped); }

		/// Sets the current position of this AtomGroup as a limb.
		/// @param newPos The Vector with the new absolute position.
		/// @param hFlipped Whether to adjust the new position for horizontal flip or not.
		void SetLimbPos(const Vector& newPos, bool hFlipped = false) { m_LimbPos = newPos - m_JointOffset.GetXFlipped(hFlipped); }

		/// Gets the current mass moment of inertia of this AtomGroup.
		/// @return A float with the moment of inertia, in Kg * meter^2.
		float GetMomentOfInertia();

		/// Sets the offset of the joint relative to this AtomGroup's origin when used as a limb.
		/// @param newOffset The new joint offset.
		void SetJointOffset(const Vector& newOffset) { m_JointOffset = newOffset; }
#pragma endregion

#pragma region Atom Management
		/// Adds a new Atom into the internal list that makes up this AtomGroup. Ownership of the Atom IS transferred!
		/// Note, this resets the moment of inertia, which then has to be recalculated.
		/// @param newAtom A pointer to an Atom that will pushed onto the end of the list. Ownership IS transferred!
		/// @param subgroupID The subgroup ID that the new Atom will have within the group.
		void AddAtom(Atom* newAtom, long subgroupID = 0) {
			newAtom->SetSubID(subgroupID);
			m_Atoms.push_back(newAtom);
			m_MomentOfInertia = 0.0F;
		}

		/// Adds a list of new Atoms to the internal list that makes up this AtomGroup. Ownership of all Atoms in the list IS NOT transferred!
		/// @param atomList A list of pointers to Atoms whose copies will be pushed onto the end of this AtomGroup's list. Ownership IS NOT transferred!
		/// @param subgroupID The desired subgroup ID for the Atoms being added.
		/// @param offset An offset that should be applied to all added Atoms.
		/// @param offsetRotation The rotation of the placed Atoms around the specified offset.
		void AddAtoms(const std::vector<Atom*>& atomList, long subgroupID = 0, const Vector& offset = Vector(), const Matrix& offsetRotation = Matrix());

		/// Removes all Atoms of a specific subgroup ID from this AtomGroup.
		/// @param removeID The ID of the subgroup of Atoms to remove.
		/// @return Whether any Atoms of that subgroup ID were found and removed.
		bool RemoveAtoms(long removeID);

		/// Removes all atoms in this AtomGroup, leaving it empty of Atoms.
		void RemoveAllAtoms() {
			m_Atoms.clear();
			m_SubGroups.clear();
			m_MomentOfInertia = 0.0F;
			m_StoredOwnerMass = 0.0F;
		}

		/// Gets whether the AtomGroup contains a subgroup with the given subgroupID.
		/// @param subgroupID The subgroupID to check for.
		/// @return Whether this AtomGroup contains a subgroup with the given subgroupID.
		bool ContainsSubGroup(long subgroupID) const { return m_SubGroups.count(subgroupID) != 0; }

		/// Updates the offsets of a subgroup of Atoms in this AtomGroup. This allows repositioning a subgroup to match the position and rotation of the graphical representation of it's owner MOSR.
		/// @param subgroupID The desired subgroup ID of the Atoms to update offsets for.
		/// @param newOffset The change in offset for the Atoms of the specified subgroup.
		/// @param newOffsetRotation The rotation of the updated Atoms around the specified offset.
		/// @return Whether any Atoms were found and updated for the specified subgroup.
		bool UpdateSubAtoms(long subgroupID = 0, const Vector& newOffset = Vector(), const Matrix& newOffsetRotation = Matrix());
#pragma endregion

#pragma region Travel
		/// Makes this AtomGroup travel and react when terrain is hit. Effects are directly applied to the owning MOSRotating.
		/// @param travelTime The amount of time in seconds that this AtomGroup is supposed to travel.
		/// @param callOnBounce Whether to call the parent MOSR's OnBounce function upon bouncing against anything or not.
		/// @param callOnSink Whether to call the parent MOSR's OnSink function upon sinking into anything or not.
		/// @return The amount of time remaining of the travelTime passed in, in seconds. This may only possibly be a non-zero if callOnBounce or callOnSink are true.
		float Travel(float travelTime, bool callOnBounce = false, bool callOnSink = false);

		/// Makes this AtomGroup travel and react when terrain is hit. Effects are applied to the passed in variables.
		/// @param position A reference to a Vector with the starting position of the AtomGroup origin. Will be altered according to where the AtomGroup ends up after it's done traveling.
		/// @param velocity A Vector with the total desired velocity of the AtomGroup. Will also be altered according to any collision response.
		/// @param rotation The current rotation Matrix of the AtomGroup. Will be altered accordingly as travel happens.
		/// @param angularVel The current desired angular velocity of the owner MOSR, in rad/sec. Will be altered.
		/// @param didWrap A bool that will be set to whether the position change involved a wrapping of the scene or not.
		/// @param totalImpulse A float to be filled out with the total magnitudes of all the forces exerted on this through collisions during this frame.
		/// @param mass The designated mass of the AtomGroup at this time.
		/// @param travelTime The amount of time in seconds that this AtomGroup is supposed to travel.
		/// @param callOnBounce Whether to call the parent MOSR's OnBounce function upon bouncing against anything or not.
		/// @param callOnSink Whether to call the parent MOSR's OnSink function upon sinking into anything or not.
		/// @return The amount of time remaining of the travelTime passed in, in seconds. This may only possibly be a non-zero if callOnBounce or callOnSink are true.
		/// @remark
		/// Pseudocode explaining how this works can be found at: https://github.com/cortex-command-community/Cortex-Command-Community-Project-Source/wiki/Notes-on-AtomGroup::Travel.
		float Travel(Vector& position, Vector& velocity, Matrix& rotation, float& angularVel, bool& didWrap, Vector& totalImpulse, float mass, float travelTime, bool callOnBounce = false, bool callOnSink = false);

		/// Makes this AtomGroup travel without rotation and react with the scene by pushing against it.
		/// @param position A reference to a Vector with the starting position of the AtomGroup origin. Will be altered according to where the AtomGroup ends up after done traveling.
		/// @param velocity A Vector with the total desired velocity of the AtomGroup.
		/// @param pushForce The maximum force that the push against other stuff in the scene can have, in Newtons (N).
		/// @param didWrap A bool that will be set to whether the position change involved a wrapping of the scene or not.
		/// @param travelTime The amount of time in seconds that this AtomGroup is supposed to travel.
		/// @param callOnBounce Whether to call the parent MOSR's OnBounce function upon bouncing against anything or not.
		/// @param callOnSink Whether to call the parent MOSR's OnSink function upon sinking into anything or not.
		/// @return A Vector with the resulting push impulse force, in Newton-second (Ns).
		Vector PushTravel(Vector& position, const Vector& velocity, float pushForce, bool& didWrap, float travelTime, bool callOnBounce = false, bool callOnSink = false);

		/// Makes this AtomGroup travel as a pushing entity relative to the position of the owning MOSRotating.
		/// If stuff in the scene is hit, resulting forces are be added to the owning MOSRotating's impulse forces.
		/// @param jointPos A reference to a Vector with the world position of the limb joint which this AtomGroup is being pushed along.
		/// @param velocity A Vector with the velocity of the owning MOSRotating.
		/// @param rotation A Matrix with the rotation of the owning MOSRotating.
		/// @param limbPath A LimbPath which this AtomGroup should travel along.
		/// @param travelTime The amount of time in seconds that this AtomGroup is supposed to travel.
		/// @param restarted Pointer to a bool which gets set to true if the LimbPath got restarted during this push. It does NOT get initialized to false!
		/// @param affectRotation Whether the forces created by this should have rotational leverage on the owner or only have translational effect.
		/// @param rotationOffset The position, relative to the owning actor's position, that we should rotate around.
		/// @param rotationOffset The positional offset to apply to our limb path.
		/// @return Whether the LimbPath passed in could start free of terrain or not.
		bool PushAsLimb(const Vector& jointPos, const Vector& velocity, const Matrix& rotation, LimbPath& limbPath, const float travelTime, bool* restarted = nullptr, bool affectRotation = true, Vector rotationOffset = Vector(), Vector positionOffset = Vector());

		/// Makes this AtomGroup travel as a lifeless limb, constrained to a radius around the joint pin in the center.
		/// @param ownerPos A Vector with the world position of the owner MOSRotating.
		/// @param jointOffset A Vector with the rotated offset of the joint that this should flail around in a radius.
		/// @param limbRadius The radius/range of the limb this is to simulate.
		/// @param velocity A Vector with the velocity of the owning MOSRotating.
		/// @param angularVel A float with the angular velocity in rad/sec of the owning MOSRotating.
		/// @param mass The mass of this dead weight limb.
		/// @param travelTime The amount of time in seconds that this AtomGroup is supposed to travel.
		void FlailAsLimb(const Vector& ownerPos, const Vector& jointOffset, const float limbRadius, const Vector& velocity, const float angularVel, const float mass, const float travelTime);
#pragma endregion

#pragma region Collision
		/// Adds a MOID that this AtomGroup should ignore collisions with during its next Travel sequence.
		/// @param moidToIgnore The MOID to add to the ignore list.
		void AddMOIDToIgnore(MOID moidToIgnore) { m_IgnoreMOIDs.push_back(moidToIgnore); }

		/// Checks whether this AtomGroup is set to ignore collisions with a MOSR of a specific MOID.
		/// @param whichMOID The MOID to check if it is ignored.
		/// @return Whether or not this MOID is being ignored.
		bool IsIgnoringMOID(MOID whichMOID) { return (*(m_Atoms.begin()))->IsIgnoringMOID(whichMOID); }

		/// Clears the list of MOIDs that this AtomGroup is set to ignore collisions with during its next Travel sequence.
		/// This should be done each frame so that fresh MOIDs can be re-added. (MOIDs are only valid during a frame).
		void ClearMOIDIgnoreList() { m_IgnoreMOIDs.clear(); }

		/// Gets whether any of the Atoms in this AtomGroup are on top of terrain pixels.
		/// @return Whether any Atom of this AtomGroup is on top of a terrain pixel.
		bool InTerrain() const;

		/// Gets the ratio of how many Atoms of this AtomGroup are on top of intact terrain pixels.
		/// @return The ratio of Atoms on top of terrain pixels, from 0 to 1.0.
		float RatioInTerrain() const;

		/// Checks whether any of the Atoms in this AtomGroup are on top of terrain pixels, and if so, attempt to move the OwnerMO out so none of the Atoms are inside any terrain pixels anymore.
		/// @param position Current position of the owner MOSR.
		/// @param strongerThan Only attempt to move out of materials stronger than this specific ID.
		/// @return Whether any intersection was successfully resolved. Will return true even if there wasn't any intersections to begin with.
		bool ResolveTerrainIntersection(Vector& position, unsigned char strongerThan = 0) const;

		/// Checks whether any of the Atoms in this AtomGroup are on top of MOSprites, and if so, attempt to move the OwnerMO out so none of the Atoms are inside the other MOSprite's silhouette anymore.
		/// @param position Current position of the owner MOSR.
		/// @return Whether all intersections were successfully resolved.
		bool ResolveMOSIntersection(Vector& position);

		/// Returns the surface area for a given pixel width.
		/// @return Our surface area.
		float GetSurfaceArea(int pixelWidth) const;
#pragma endregion

#pragma region Debug
		/// Draws this AtomGroup's current graphical debug representation to a BITMAP of choice.
		/// @param targetBitmap A pointer to a BITMAP to draw on.
		/// @param targetPos The absolute position of the target bitmap's upper left corner in the Scene.
		/// @param useLimbPos Whether to use the limb position of this AtomGroup, or the owner's position.
		/// @param color The color to draw the Atoms' pixels as.
		void Draw(BITMAP* targetBitmap, const Vector& targetPos, bool useLimbPos = false, unsigned char color = 34) const;
#pragma endregion

	protected:
		/// Enumeration for how the AtomGroup's area is distributed. Linear means it acts a 2D line whereas Circle/Square acts as a pseudo-3d circle/square.
		enum class AreaDistributionType {
			Linear,
			Circle,
			Square
		};

		static const std::unordered_map<std::string, AreaDistributionType> c_AreaDistributionTypeMap; //!< A map of strings to AreaDistributionTypes to support string parsing for the AreaDistributionType enum.

		static Entity::ClassInfo m_sClass; //!< ClassInfo for this class.

		// TODO: It's probably worth trying out changing this from a list to a vector. m_Atoms is iterated over often and we could probably get some big gainz by doing this swap.
		// The downside is anytime attachables with atoms get added we may have the cost of resizing the vector but that's an uncommon use case while iterating over atoms happens multiple times per frame.
		std::vector<Atom*> m_Atoms; //!< List of Atoms that constitute the group. Owned by this.
		std::unordered_map<long, std::vector<Atom*>> m_SubGroups; //!< Sub groupings of Atoms. Points to Atoms owned in m_Atoms. Not owned.

		MOSRotating* m_OwnerMOSR; //!< The owner of this AtomGroup. The owner is obviously not owned by this AtomGroup.
		float m_StoredOwnerMass; //!< The stored mass for the owner MOSR. Used to figure out when the moment of inertia needs to be recalculated due to significant mass changes.
		const Material* m_Material; //!< Material of this AtomGroup.

		bool m_AutoGenerate; //!< Whether the Atoms in this AtomGroup were automatically generated based on a sprite, or manually defined.

		/// The density of Atoms in this AtomGroup along the outline of the owning MOSR's graphical representation. Higher values mean more pixels are skipped along the outline when placing Atoms.
		/// For example: a box that is 20x20px will have an outline of 80px, with a resolution value of 10 an Atom will be placed every 10 pixels on this outline, resulting in an AtomGroup that
		/// consists of 8 Atoms total with 2 Atoms on each plane. Note that the outline isn't actually "unwrapped" and higher values may result in slightly less accurate Atom placement on complex sprites.
		/// 0 means the Atoms in this AtomGroup were defined manually. 1 means the whole outline will be populated with Atoms, resulting in the most accurate physical representation.
		int m_Resolution;

		int m_Depth; //!< The depth Atoms in this AtomGroup are placed off the edge of the owning MOSR's graphical representation outline towards it's center, in pixels.

		Vector m_JointOffset; //!< The offset of the wrist/ankle relative to this AtomGroup's origin when used as a limb.
		Vector m_LimbPos; //!< The last position of this AtomGroup when used as a limb.

		float m_MomentOfInertia; //!< Moment of Inertia for this AtomGroup.

		std::vector<MOID> m_IgnoreMOIDs; //!< List of MOIDs this AtomGroup will ignore collisions with.

		AreaDistributionType m_AreaDistributionType; //!< How this AtomGroup will distribute energy when it collides with something.

		float m_AreaDistributionSurfaceAreaMultiplier; //!< A multiplier for the AtomGroup's surface area, which affects how much it digs into terrain. 0.5 would halve the surface area so it would dig into terrain twice as much, 2.0 would make it dig into terrain half as much.

	private:
#pragma region Create Breakdown
		/// Generates an AtomGroup using the owner MOSRotating's sprite outline.
		/// @param ownerMOSRotating MOSRotating whose outline will be approximated by Atoms of this AtomGroup.
		void GenerateAtomGroup(MOSRotating* ownerMOSRotating);

		/// Create and add an Atom to this AtomGroup's list of Atoms. This is called during GenerateAtomGroup().
		/// @param ownerMOSRotating MOSRotating whose outline will be approximated by Atoms of this AtomGroup.
		/// @param spriteOffset Sprite offset relative to the owner MOSRotating.
		/// @param x X coordinate in the sprite frame.
		/// @param y Y coordinate in the sprite frame.
		/// @param calcNormal Whether to set a normal for the Atom. Should be true for surface Atoms.
		void AddAtomToGroup(MOSRotating* ownerMOSRotating, const Vector& spriteOffset, int x, int y, bool calcNormal);
#pragma endregion

		/// Clears all the member variables of this AtomGroup, effectively resetting the members of this abstraction level only.
		void Clear();
	};
} // namespace RTE
