#pragma once

/// Header file for the ACRocket class.
/// @author Daniel Tabar
/// data@datarealms.com
/// http://www.datarealms.com
/// Inclusions of header files
#include "ACraft.h"

namespace RTE {

	class Attachable;
	class Arm;
	class Leg;
	// class LimbPath;

	/// A rocket craft, with main engine and rotation of the whole body as a
	/// means of steering.
	/// 12/13/2006 ARocket changed names to ACRocket, parent changed to ACraft
	class ACRocket : public ACraft {
		friend struct EntityLuaBindings;

		enum LandingGearState {
			RAISED = 0,
			LOWERED,
			LOWERING,
			RAISING,
			GearStateCount
		};

		/// Public member variable, method and friend function declarations
	public:
		// Concrete allocation and cloning definitions
		EntityAllocation(ACRocket);
		SerializableOverrideMethods;
		ClassInfoGetters;

		/// Constructor method used to instantiate a ACRocket object in system
		/// memory. Create() should be called before using the object.
		ACRocket();

		/// Destructor method used to clean up a ACRocket object before deletion
		/// from system memory.
		~ACRocket() override;

		/// Makes the ACRocket object ready for use.
		/// @return An error return value signaling sucess or any particular failure.
		/// Anything below 0 is an error signal.
		int Create() override;

		/// Creates a ACRocket to be identical to another, by deep copy.
		/// @param reference A reference to the ACRocket to deep copy.
		/// @return An error return value signaling sucess or any particular failure.
		/// Anything below 0 is an error signal.
		int Create(const ACRocket& reference);

		/// Resets the entire ACRocket, including its inherited members, to their
		/// default settings or values.
		void Reset() override {
			Clear();
			ACraft::Reset();
		}

		/// Destroys and resets (through Clear()) the SceneLayer object.
		/// @param notInherited Whether to only destroy the members defined in this derived class, or (default: false)
		/// to destroy all inherited members also.
		void Destroy(bool notInherited = false) override;

		/// Gets the altitide of this' pos (or appropriate low point) over the
		/// terrain, in pixels.
		/// @param max The max altitude you care to check for. 0 Means check the whole scene's height. (default: 0)
		/// @param accuracy The accuracy within which measurement is acceptable. Higher number (default: 0)
		/// here means less calculation.
		/// @return The rough altitude over the terrain, in pixels.
		float GetAltitude(int max = 0, int accuracy = 0) override;

		/// Update called prior to controller update. Ugly hack. Supposed to be done every frame.
		void PreControllerUpdate() override;

		/// Updates this MovableObject. Supposed to be done every frame.
		/// @param Nosssssssne.
		void Update() override;

		/// Draws this ACRocket's current graphical representation to a
		/// BITMAP of choice.
		/// @param pTargetBitmap A pointer to a BITMAP to draw on.
		/// @param targetPos The absolute position of the target bitmap's upper left corner in the Scene. (default: Vector())
		/// @param mode In which mode to draw in. See the DrawMode enumeration for the modes. (default: g_DrawColor)
		/// @param onlyPhysical Whether to not draw any extra 'ghost' items of this MovableObject, (default: false)
		/// indicator arrows or hovering HUD text and so on.
		void Draw(BITMAP* pTargetBitmap, const Vector& targetPos = Vector(), DrawMode mode = g_DrawColor, bool onlyPhysical = false) const override;

		/// The recomended, not absolute, maximum number of actors that fit in the
		/// invetory. Used by the activity AI.
		/// @return An integer with the recomended number of actors that fit in the craft.
		/// Default is two.
		int GetMaxPassengers() const override { return m_MaxPassengers > -1 ? m_MaxPassengers : 2; }

		/// Gets the right leg of this ACRocket.
		/// @return A pointer to the right Leg of this ACRocket. Ownership is NOT transferred.
		Leg* GetRightLeg() const { return m_pRLeg; }

		/// Sets the right Leg for this ACRocket.
		/// @param newLeg The new Leg to use.
		void SetRightLeg(Leg* newLeg);

		/// Gets the left Leg of this ACRocket.
		/// @return A pointer to the left Leg of this ACRocket. Ownership is NOT transferred.
		Leg* GetLeftLeg() const { return m_pLLeg; }

		/// Sets the left Leg for this ACRocket.
		/// @param newLeg The new Leg to use.
		void SetLeftLeg(Leg* newLeg);

		/// Gets the main thruster of this ACRocket.
		/// @return A pointer to the main thruster of this ACRocket. Ownership is NOT transferred.
		AEmitter* GetMainThruster() const { return m_pMThruster; }

		/// Sets the main thruster for this ACRocket.
		/// @param newThruster The new thruster to use.
		void SetMainThruster(AEmitter* newThruster);

		/// Gets the right side thruster of this ACRocket.
		/// @return A pointer to the right side thruster of this ACRocket. Ownership is NOT transferred.
		AEmitter* GetRightThruster() const { return m_pRThruster; }

		/// Sets the right side thruster for this ACRocket.
		/// @param newThruster The new thruster to use.
		void SetRightThruster(AEmitter* newThruster);

		/// Gets the left side thruster of this ACRocket.
		/// @return A pointer to the left side thruster of this ACRocket. Ownership is NOT transferred.
		AEmitter* GetLeftThruster() const { return m_pLThruster; }

		/// Sets the left side thruster for this ACRocket.
		/// @param newThruster The new thruster to use.
		void SetLeftThruster(AEmitter* newThruster);

		/// Gets the right side secondary thruster of this ACRocket.
		/// @return A pointer to the right side secondary thruster of this ACRocket. Ownership is NOT transferred.
		AEmitter* GetURightThruster() const { return m_pURThruster; }

		/// Sets the right side secondary thruster for this ACRocket.
		/// @param newThruster The new thruster to use.
		void SetURightThruster(AEmitter* newThruster);

		/// Gets the left side secondary thruster of this ACRocket.
		/// @return A pointer to the left side secondary thruster of this ACRocket. Ownership is NOT transferred.
		AEmitter* GetULeftThruster() const { return m_pULThruster; }

		/// Sets the left side secondary thruster for this ACRocket.
		/// @param newThruster The new thruster to use.
		void SetULeftThruster(AEmitter* newThruster);

		/// Gets the the landing gear state
		/// @return Current landing gear state.
		unsigned int GetGearState() const { return m_GearState; }

		/// Protected member variable and method declarations
	protected:
		// Member variables
		static Entity::ClassInfo m_sClass;
		// Attached nosecone/capsule.
		//    Attachable *m_pCapsule;
		// Right landing gear.
		Leg* m_pRLeg;
		// Left landing gear.
		Leg* m_pLLeg;
		// Body AtomGroups.
		AtomGroup* m_pBodyAG;
		// Limb AtomGroups.
		AtomGroup* m_pRFootGroup;
		AtomGroup* m_pLFootGroup;
		// Thruster emitters.
		AEmitter* m_pMThruster;
		AEmitter* m_pRThruster;
		AEmitter* m_pLThruster;
		AEmitter* m_pURThruster;
		AEmitter* m_pULThruster;
		// Current landing gear action state.
		unsigned int m_GearState;
		// Limb paths for different movement states.
		// [0] is for the right limbs, and [1] is for left.
		LimbPath m_Paths[2][GearStateCount];
		float m_MaxGimbalAngle; //!< How much the main engine is able to tilt in order to stabilize the rocket.

		/// Private member variable and method declarations
	private:
		/// Clears all the member variables of this ACRocket, effectively
		/// resetting the members of this abstraction level only.
		void Clear();

		// Disallow the use of some implicit methods.
		ACRocket(const ACRocket& reference) = delete;
		ACRocket& operator=(const ACRocket& rhs) = delete;
	};

} // namespace RTE
