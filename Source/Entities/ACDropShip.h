#pragma once

/// Header file for the ACDropShip class.
/// @author Daniel Tabar
/// data@datarealms.com
/// http://www.datarealms.com
/// Inclusions of header files
#include "ACraft.h"

namespace RTE {

	class Attachable;

	/// A hovering craft, with two engines on each attached on each end which
	/// tilt independently of the body to achieve steering.
	class ACDropShip : public ACraft {

		/// Public member variable, method and friend function declarations
	public:
		// Concrete allocation and cloning definitions
		EntityAllocation(ACDropShip);
		SerializableOverrideMethods;
		ClassInfoGetters;

		/// Constructor method used to instantiate a ACDropShip object in system
		/// memory. Create() should be called before using the object.
		ACDropShip();

		/// Destructor method used to clean up a ACDropShip object before deletion
		/// from system memory.
		~ACDropShip() override;

		/// Makes the ACDropShip object ready for use.
		/// @return An error return value signaling sucess or any particular failure.
		/// Anything below 0 is an error signal.
		int Create() override;

		/// Creates a ACDropShip to be identical to another, by deep copy.
		/// @param reference A reference to the ACDropShip to deep copy.
		/// @return An error return value signaling sucess or any particular failure.
		/// Anything below 0 is an error signal.
		int Create(const ACDropShip& reference);

		/// Resets the entire ACDropShip, including its inherited members, to their
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

		/// Checks for obstacles in the travel direction.
		/// @param distance How far ahead of travel direction to check for obstacles.
		/// @return Which MOID was detected as obstacle. g_NoMOID means nothing was detected.
		MOID DetectObstacle(float distance);

		/// Tells whether this has the means and will try to right itself, or if
		/// that's up to the Controller to do.
		/// @return Wheter this will try to auto stabilize.
		bool AutoStabilizing() override { return true; }

		/// Update called prior to controller update. Ugly hack. Supposed to be done every frame.
		void PreControllerUpdate() override;

		/// The recomended, not absolute, maximum number of actors that fit in the
		/// invetory. Used by the activity AI.
		/// @return An integer with the recomended number of actors that fit in the craft.
		/// Default is four.
		int GetMaxPassengers() const override { return m_MaxPassengers > -1 ? m_MaxPassengers : 4; }

		/// Gets the right side thruster of this ACDropship.
		/// @return A pointer to the right side thruster of this ACDropship. Ownership is NOT transferred.
		AEmitter* GetRightThruster() const { return m_pRThruster; }

		/// Sets the right side thruster for this ACDropship.
		/// @param newThruster The new thruster to use.
		void SetRightThruster(AEmitter* newThruster);

		/// Gets the left side thruster of this ACDropship.
		/// @return A pointer to the left side thruster of this ACDropship. Ownership is NOT transferred.
		AEmitter* GetLeftThruster() const { return m_pLThruster; }

		/// Sets the left side thruster for this ACDropship.
		/// @param newThruster The new thruster to use.
		void SetLeftThruster(AEmitter* newThruster);

		/// Gets the right side secondary thruster of this ACDropship.
		/// @return A pointer to the right side secondary thruster of this ACDropship. Ownership is NOT transferred.
		AEmitter* GetURightThruster() const { return m_pURThruster; }

		/// Sets the right side secondary thruster for this ACDropship.
		/// @param newThruster The new thruster to use.
		void SetURightThruster(AEmitter* newThruster);

		/// Gets the left side secondary thruster of this ACDropship.
		/// @return A pointer to the left side secondary thruster of this ACDropship. Ownership is NOT transferred.
		AEmitter* GetULeftThruster() const { return m_pULThruster; }

		/// Sets the left side secondary thruster for this ACDropship.
		/// @param newThruster The new thruster to use.
		void SetULeftThruster(AEmitter* newThruster);

		/// Gets the left side hatch of this ACDropship.
		/// @return A pointer to the left side hatch of this ACDropship. Ownership is NOT transferred.
		Attachable* GetLeftHatch() const { return m_pLHatch; }

		/// Sets the left side hatch for this ACDropship.
		/// @param newHatch The new hatch to use.
		void SetLeftHatch(Attachable* newHatch);

		/// Gets the right side hatch of this ACDropship.
		/// @return A pointer to the right side hatch of this ACDropship. Ownership is NOT transferred.
		Attachable* GetRightHatch() const { return m_pRHatch; }

		/// Sets the right side hatch for this ACDropship.
		/// @param newHatch The new hatch to use.
		void SetRightHatch(Attachable* newHatch);

		/// Get max engine rotation angle in degrees.
		/// @return Max engine angle in degrees.
		float GetMaxEngineAngle() const { return m_MaxEngineAngle; }

		/// Sets max engine rotation angle in degrees.
		/// @param newAngle Max engine angle in degrees.
		void SetMaxEngineAngle(float newAngle) { m_MaxEngineAngle = newAngle; }

		/// Gets the abstract rate of LateralControl change. Default is 6
		/// @return Current lateral control speed value.
		float GetLateralControlSpeed() const { return m_LateralControlSpeed; }

		/// Sets the abstract rate of LateralControl change. Default is 6
		/// @param newSpeed New lateral control speed value.
		void SetLateralControlSpeed(float newSpeed) { m_LateralControl = newSpeed; }

		/// Sets lateral control value -1.0 to 1.0 control of sideways movement. 0 means try to stand still in X.
		/// @return Current lateral control value.
		float GetLateralControl() const { return m_LateralControl; }

		/// Gets the modifier for height at which this ACDropship should hover above terrain.
		/// @return The modifier for height at which this ACDropship should hover above terrain.
		float GetHoverHeightModifier() const { return m_HoverHeightModifier; }

		/// Sets the modifier for height at which this ACDropship should hover above terrain.
		/// @param newHoverHeightModifier The new modifier for height at which this ACDropship should hover above terrain.
		void SetHoverHeightModifier(float newHoverHeightModifier) { m_HoverHeightModifier = newHoverHeightModifier; }

		/// Protected member variable and method declarations
	protected:
		// Member variables
		static Entity::ClassInfo m_sClass;
		// Body AtomGroups.
		AtomGroup* m_pBodyAG;
		// Thruster emitters.
		// TODO when this class is cleaned up, these and their getters and setters should probably be renamed (I'd argue the lua bindings should be broken to match but that's debatable). L and R should be Left and Right and they should probably be Primary and Secondary.
		AEmitter* m_pRThruster;
		AEmitter* m_pLThruster;
		AEmitter* m_pURThruster;
		AEmitter* m_pULThruster;

		// Hatch doors
		Attachable* m_pRHatch;
		Attachable* m_pLHatch;
		// How much the hatch doors rotate to open
		Matrix m_HatchSwingRange;
		// From 0 to 1.0, the state of hatch door openness
		float m_HatchOpeness;

		// -1.0 to 1.0 control of sideways movement. 0 means try to stand still in X.
		float m_LateralControl;
		// Abstract speed at which Lateralcontroll is changed
		float m_LateralControlSpeed;

		// Automatically stabilize the craft with the upper thrusters? Defaults to yes.
		int m_AutoStabilize;

		// Maximum engine rotation in degrees
		float m_MaxEngineAngle;

		float m_HoverHeightModifier; //!< The modifier for the height at which this ACDropShip should hover above terrain when releasing its cargo. Used in cpp and Lua AI.

		/// Private member variable and method declarations
	private:
		/// Clears all the member variables of this ACDropShip, effectively
		/// resetting the members of this abstraction level only.
		void Clear();

		// Disallow the use of some implicit methods.
		ACDropShip(const ACDropShip& reference) = delete;
		ACDropShip& operator=(const ACDropShip& rhs) = delete;
	};

} // namespace RTE
