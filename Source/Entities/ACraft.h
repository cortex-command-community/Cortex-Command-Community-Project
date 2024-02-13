#pragma once

/// Header file for the ACraft class.
/// @author Daniel Tabar
/// data@datarealms.com
/// http://www.datarealms.com
/// Inclusions of header files
#include "Actor.h"
#include "LimbPath.h"

struct BITMAP;

namespace RTE {

	enum class PieSliceType : int;

	/// A flying Actor which carries other things and can drop them.
	class ACraft : public Actor {
		friend struct EntityLuaBindings;

		/// Public member variable, method and friend function declarations
	public:
		SerializableOverrideMethods;
		ClassInfoGetters;
		DefaultPieMenuNameGetter("Default Craft Pie Menu");

		enum HatchState {
			CLOSED = 0,
			OPENING,
			OPEN,
			CLOSING,
			HatchStateCount
		};

		enum Side {
			RIGHT = 0,
			LEFT
		};

		/// Something to bundle the properties of ACraft exits together.
		class Exit :
		    public Serializable {

			friend class ACraft;

			/// Public member variable, method and friend function declarations
		public:
			SerializableClassNameGetter;
			SerializableOverrideMethods;

			/// Constructor method used to instantiate a Exit object in system
			/// memory. Create() should be called before using the object.
			Exit() { Clear(); }

			/// Makes the Exit object ready for use.
			/// @return An error return value signaling sucess or any particular failure.
			/// Anything below 0 is an error signal.
			int Create() override;

			/// Creates a Exit to be identical to another, by deep copy.
			/// @param reference A reference to the Exit to deep copy.
			/// @return An error return value signaling sucess or any particular failure.
			/// Anything below 0 is an error signal.
			int Create(const Exit& reference);

			/// Resets the entire Serializable, including its inherited members, to their
			/// default settings or values.
			void Reset() override { Clear(); }

			/// Gets the position offset of this exit from the position of its ACraft.
			/// @return The coordinates relative to the m_Pos of this' ACraft.
			Vector GetOffset() const { return m_Offset; }

			/// Gets the velocity of anything that exits through this.
			/// @return The velocity vector for anything exiting through this.
			Vector GetVelocity() const { return m_Velocity * (1.0F + m_VelSpread * RandomNormalNum()); }

			/// Gets the width from the center tanget created by the velocity vector
			/// out from the offet point. This times two gives the total width of the
			/// opening.
			/// @return Half the total width of the opening.
			float GetRadius() const { return m_Radius; }

			/// Gets the distance this exit can suck in objects from.
			/// @return The sucking range of this.
			float GetRange() const { return m_Range; }

			/// Calculates whether this exit is currently clear enough of terrain to
			/// safely put things through without them ending up in the terrain.
			/// @param pos The position of the parent ACraft of this.
			/// @param rot The rotation of the parent ACraft of this.
			/// @param size How large (radius) the item is that is supposed to fit. (default: 20)
			/// @return If this has been determined clear to put anything through.
			bool CheckIfClear(const Vector& pos, Matrix& rot, float size = 20);

			/// Tells if this is clear of the terrain to put things through. Faster than
			/// CheckIfClear().
			/// @return If this has been determined clear to put anything through.
			bool IsClear() const { return m_Clear; }

			/// Uses cast MO rays to see if anyhting is able to be drawn into this
			/// exit. If so, it will alter the positiona nd velocity of the objet so
			/// it flies into the exit until it is sufficiently inside and then it'll
			/// return the MO here, OWNERHIP NOT TRANSFERRED! It is still in MovableMan!
			/// @param pExitOwner A pointer to the ACraft owner of this Exit. OWNERSHIP IS NOT TRANSFERRED!
			/// @return If an MO has been fully drawn into the exit, it will be returned here,
			/// OWNERSHIP NOT TRANSFERRED!
			MOSRotating* SuckInMOs(ACraft* pExitOwner);

			/// Protected member variable and method declarations
		protected:
			// The offset of this exit relative the position of its ACraft
			Vector m_Offset;
			// The exiting velocity of anyhting exiting through this
			Vector m_Velocity;
			// The spread in velocity, ratio
			float m_VelSpread;
			// The width from the center tanget created by the velocity vector out from the offet point. This times two gives the total width of the opening.
			float m_Radius;
			// How far away the exit cna suck objects in from
			float m_Range;
			// Temporary var to check if this is clear of terrain for putting things through
			bool m_Clear;
			// Movable Object that is being drawn into this exit
			MOSRotating* m_pIncomingMO;

			/// Private member variable and method declarations
		private:
			static const std::string c_ClassName; //!< A string with the friendly-formatted type name of this object.

			/// Clears all the member variables of this Exit, effectively
			/// resetting the members of this abstraction level only.
			void Clear();
		};

		// Concrete allocation and cloning definitions
		// EntityAllocation(ACraft)

		/// Constructor method used to instantiate a ACraft object in system
		/// memory. Create() should be called before using the object.
		ACraft();

		/// Destructor method used to clean up a ACraft object before deletion
		/// from system memory.
		~ACraft() override;

		/// Makes the ACraft object ready for use.
		/// @param A Reader that the ACraft will create itself with.
		/// Whether there is a class name in the stream to check against to make
		/// sure the correct type is being read from the stream.
		/// @return An error return value signaling sucess or any particular failure.
		/// Anything below 0 is an error signal.
		int Create() override;

		/// Creates a ACraft to be identical to another, by deep copy.
		/// @param reference A reference to the ACraft to deep copy.
		/// @return An error return value signaling sucess or any particular failure.
		/// Anything below 0 is an error signal.
		int Create(const ACraft& reference);

		/// Resets the entire ACraft, including its inherited members, to their
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

		/// Gets the current state of the hatch.
		/// @return An int encoding the hatch state. See the HatchState enum.
		unsigned int GetHatchState() const { return m_HatchState; }

		/// Sets which team this belongs to, and all its inventory too.
		/// @param team The assigned team number.
		void SetTeam(int team) override;

		/// Tries to handle the activated PieSlice in this object's PieMenu, if there is one, based on its SliceType.
		/// @param pieSliceType The SliceType of the PieSlice being handled.
		/// @return Whether or not the activated PieSlice SliceType was able to be handled.
		bool HandlePieCommand(PieSliceType pieSliceType) override;

		/// Tells whether this has the means and will try to right itself, or if
		/// that's up to the Controller to do.
		/// @return Wheter this will try to auto stabilize.
		virtual bool AutoStabilizing() { return false; }

		/// Opens the hatch doors, if they're closed or closing.
		void OpenHatch();

		/// Closes the hatch doors, if they're open or opening.
		void CloseHatch();

		/// Adds an inventory item to this Actor.
		/// @param pItemToAdd An pointer to the new item to add. Ownership IS TRANSFERRED!
		/// @return None..
		void AddInventoryItem(MovableObject* pItemToAdd) override;

		/// Opens the hatches and makes everything in the Rocket fly out, including
		/// the passenger Actors, one after another. It may not happen
		/// instantaneously, so check for ejection being complete with
		/// IsInventoryEmpty().
		void DropAllInventory() override;

		/// Gets the mass of this ACraft's inventory of newly collected items.
		/// @return The mass of this ACraft's newly collected inventory.
		float GetCollectedInventoryMass() const;

		/// Gets the mass of this ACraft, including the mass of its Attachables, wounds and inventory.
		/// @return The mass of this ACraft, its inventory and all its Attachables and wounds in Kilograms (kg).
		float GetMass() const override { return Actor::GetMass() + GetCollectedInventoryMass(); }

		/// Indicated whether this has landed and delivered yet on its current run.
		/// @return Whether this has delivered yet.
		bool HasDelivered() { return m_HasDelivered; }

		/// Resets all the timers related to this, including the scuttle timer.
		void ResetAllTimers() override;

		/// Updates this MovableObject. Supposed to be done every frame.
		void Update() override;

		/// Draws this Actor's current graphical HUD overlay representation to a
		/// BITMAP of choice.
		/// @param pTargetBitmap A pointer to a BITMAP to draw on.
		/// @param targetPos The absolute position of the target bitmap's upper left corner in the Scene. (default: Vector())
		/// @param whichScreen Which player's screen this is being drawn to. May affect what HUD elements (default: 0)
		/// get drawn etc.
		void DrawHUD(BITMAP* pTargetBitmap, const Vector& targetPos = Vector(), int whichScreen = 0, bool playerControlled = false) override;

		/// The recomended, not absolute, maximum number of actors that fit in the
		/// invetory. Used by the activity AI.
		/// @return An integer with the recomended number of actors that fit in the craft.
		/// Default is -1 (unknown).
		virtual int GetMaxPassengers() const { return m_MaxPassengers; }

		/// Sets the recomended, not absolute, maximum number of actors that fit in the
		/// invetory. Used by the activity AI.
		/// @param max An integer with the recomended number of actors that fit in the craft.
		/// Default is -1 (unknown).
		virtual void SetMaxPassengers(int max) { m_MaxPassengers = max; }

		/// Description:		Returns delivery delay multiplier.
		/// @return Delivery delay multiplier.
		float GetDeliveryDelayMultiplier() const { return m_DeliveryDelayMultiplier; }

		/// Description:		Sets delivery delay multiplier.
		/// @param newValue Delivery delay multiplier.
		void SetDeliveryDelayMultiplier(float newValue) { m_DeliveryDelayMultiplier = newValue; }

		/// Gets whether this ACraft will scuttle automatically on death.
		/// @return Whether this ACraft will scuttle automatically on death.
		bool GetScuttleOnDeath() const { return m_ScuttleOnDeath; }

		/// Sets whether this ACraft will scuttle automatically on death.
		/// @param scuttleOnDeath Whether this ACraft will scuttle automatically on death.
		void SetScuttleOnDeath(bool scuttleOnDeath) { m_ScuttleOnDeath = scuttleOnDeath; }

		/// Gets the hatch opening/closing delay of this ACraft.
		/// @return The hatch delay of this ACraft.
		int GetHatchDelay() const { return m_HatchDelay; }

		/// Sets the hatch opening/closing delay of this ACraft.
		/// @param newDelay The new hatch delay of this ACraft.
		void SetHatchDelay(int newDelay) { m_HatchDelay = newDelay; }

		/// Destroys this ACraft and creates its specified Gibs in its place with appropriate velocities. Any Attachables are removed and also given appropriate velocities.
		/// @param impactImpulse The impulse (kg * m/s) of the impact causing the gibbing to happen.
		/// @param movableObjectToIgnore A pointer to an MO which the Gibs and Attachables should not be colliding with.
		void GibThis(const Vector& impactImpulse = Vector(), MovableObject* movableObjectToIgnore = nullptr) override;

		/// Gets this ACraft's hatch opening sound. Ownership is NOT transferred!
		/// @return The SoundContainer for this ACraft's hatch opening sound.
		SoundContainer* GetHatchOpenSound() const { return m_HatchOpenSound; }

		/// Sets this ACraft's hatch opening sound. Ownership IS transferred!
		/// @param newSound The new SoundContainer for this ACraft's hatch opening sound.
		void SetHatchOpenSound(SoundContainer* newSound) { m_HatchOpenSound = newSound; }

		/// Gets this ACraft's hatch closing sound. Ownership is NOT transferred!
		/// @return The SoundContainer for this ACraft's hatch closing sound.
		SoundContainer* GetHatchCloseSound() const { return m_HatchCloseSound; }

		/// Sets this ACraft's hatch closing sound. Ownership IS transferred!
		/// @param newSound The new SoundContainer for this ACraft's hatch closing sound.
		void SetHatchCloseSound(SoundContainer* newSound) { m_HatchCloseSound = newSound; }

		/// Gets this ACraft's crash sound. Ownership is NOT transferred!
		/// @return The SoundContainer for this ACraft's crash sound.
		SoundContainer* GetCrashSound() const { return m_CrashSound; }

		/// Sets this ACraft's crash sound. Ownership IS transferred!
		/// @param newSound The new SoundContainer for this ACraft's crash sound.
		void SetCrashSound(SoundContainer* newSound) { m_CrashSound = newSound; }

		/// Protected member variable and method declarations
	protected:
		// Member variables
		static Entity::ClassInfo m_sClass;
		// Current movement state.
		unsigned int m_MoveState;
		// Current hatch action state.
		unsigned int m_HatchState;
		// Timer for opening and closing hatches
		Timer m_HatchTimer;
		// The time it takes to open or close the hatch, in ms.
		int m_HatchDelay;
		// Sound for opening the hatch
		SoundContainer* m_HatchOpenSound;
		// Sound for closing the hatch
		SoundContainer* m_HatchCloseSound;
		std::deque<MovableObject*> m_CollectedInventory; //!< A separate inventory to temporarily store newly collected items, so that they don't get immediately ejected from the main inventory while the hatch is still open.
		// All the possible exits for when ejecting stuff out of this.
		std::list<Exit> m_Exits;
		// Last used exit so we can alternate/cycle
		std::list<Exit>::iterator m_CurrentExit;
		// The delay between each exiting passenger Actor
		long m_ExitInterval;
		// Times the exit interval
		Timer m_ExitTimer;
		// The phase of the exit lines animation
		int m_ExitLinePhase;
		// Whether this has landed and delivered yet on its current run
		bool m_HasDelivered;
		// Whether this is capable of landing on the ground at all
		bool m_LandingCraft;
		// Timer for checking if craft is hopelessly flipped and should die
		Timer m_FlippedTimer;
		// Timer to measure how long ago a crash sound was played
		Timer m_CrashTimer;
		// Crash sound
		SoundContainer* m_CrashSound;
		// The maximum number of actors that fit in the inventory
		int m_MaxPassengers;
		int m_ScuttleIfFlippedTime; //!< The time after which the craft will scuttle automatically, if tipped over.
		bool m_ScuttleOnDeath; //!< Whether the craft will self-destruct at zero health.

		static bool s_CrabBombInEffect; //!< Flag to determine if a craft is triggering the Crab Bomb effect.

		////////
		// AI states

		enum CraftDeliverySequence {
			FALL = 0,
			LAND,
			STANDBY,
			UNLOAD,
			LAUNCH,
			UNSTICK
		};

		enum AltitudeMoveState {
			HOVER = 0,
			DESCEND,
			ASCEND
		};

		// What the rocket/ship is currently doing in an AI landing sequence
		int m_DeliveryState;
		// Whether the AI is trying to go higher, lower, or stand still in altitude
		int m_AltitudeMoveState;
		// Controls the altitude gain/loss the AI is trying to achieve. Normalized -1.0 (max rise) to 1.0 (max drop). 0 is approximate hover.
		float m_AltitudeControl;
		// Mutliplier to apply to default delivery time
		float m_DeliveryDelayMultiplier;

		/// Private member variable and method declarations
	private:
		/// Clears all the member variables of this ACraft, effectively
		/// resetting the members of this abstraction level only.
		void Clear();

		// Disallow the use of some implicit methods.
		ACraft(const ACraft& reference) = delete;
		ACraft& operator=(const ACraft& rhs) = delete;
	};

} // namespace RTE
