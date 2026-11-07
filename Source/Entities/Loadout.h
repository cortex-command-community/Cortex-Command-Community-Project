#pragma once

/// Loadout class
/// @author Daniel Tabar
/// dtabar@datarealms.com
/// http://www.datarealms.com
/// Inclusions of header files
// #include "FrameMan.h"
#include "Entity.h"

namespace RTE {

	class SceneObject;
	class MovableObject;
	class ACraft;
	class Actor;

	/// Defines a delivery of Actors, with all their equipment etc.
	class Loadout : public Entity {

		/// Public member variable, method and friend function declarations
	public:
		// Concrete allocation and cloning definitions
		EntityAllocation(Loadout);
		SerializableOverrideMethods;
		ClassInfoGetters;

		/// Constructor method used to instantiate a Loadout object in system
		/// memory. Create() should be called before using the object.
		Loadout() { Clear(); }

		/// Copy constructor method used to instantiate a Loadout object
		/// identical to an already existing one.
		/// @param reference A Loadout object which is passed in by reference.
		Loadout(const Loadout& reference) {
			if (this != &reference) {
				Clear();
				Create(reference);
			}
		}

		/// An assignment operator for setting one Loadout equal to another.
		/// @param rhs A Loadout reference.
		/// @return A reference to the changed Loadout.
		Loadout& operator=(const Loadout& rhs) {
			if (this != &rhs) {
				Destroy();
				Create(rhs);
			}
			return *this;
		}

		/// Creates a Loadout to be identical to another, by deep copy.
		/// @param reference A reference to the Loadout to deep copy.
		/// @return An error return value signaling sucess or any particular failure.
		/// Anything below 0 is an error signal.
		int Create(const Loadout& reference);

		/// Resets the entire Entity, including its inherited members, to their
		/// default settings or values.
		void Reset() override {
			Clear();
			Entity::Reset();
		}

#pragma region INI Handling
		/// Records an Entity against a reference, expected to be its preset.
		/// @param writer A Writer that the Entity will save itself to.
		/// @param entityReference A reference preset to write properties against.
		/// @param hashData A clump of hash data for cheap property comparisons.
		/// @return An error return value signaling success or any particular failure. Anything below 0 is an error signal.
		int Write(Writer& writer, const Entity& entityReference, HashingData& hashData) const override;
#pragma endregion

		/// Shows whether this loadout is complete, or something it refers to
		/// can't be used right now. What can't be used is still saved with it.
		/// @return Complete or not.
		bool IsComplete() const { return m_UnavailableDeliveryCraft.m_Value.empty() && m_UnavailableCargoItems.empty(); }

		/// Creates and returns the first Actor that this Loadout has and equips.
		/// Ownership IS transferred!! All items of the Loadout of this Deployment
		/// will be added to the Actor's inventory as well (and also owned by it)
		/// @param nativeModule Which in-game player to create the Actor for.
		/// @param foreignMult A float which will be added to with the cost of the stuff returned here.
		/// @return The Actor instance, if any, that is first defined in this Loadout.
		/// OWNERSHIP IS TRANSFERRED!
		Actor* CreateFirstActor(int nativeModule, float foreignMult, float nativeMult, float& costTally) const;

		/// Creates and returns the first Device that is defined in this Loadout.
		/// Ownership IS transferred!! Only the first Device is created.
		/// @param nativeModule Which in-game player to create the device for.
		/// @param foreignMult A float which will be added to with the cost of the stuff returned here.
		/// @return The SceneObject instance, if any, that this Loadout defines first.
		/// OWNERSHIP IS TRANSFERRED!
		SceneObject* CreateFirstDevice(int nativeModule, float foreignMult, float nativeMult, float& costTally) const;

		/// Gets the preset of the delivery craft set for this loadout. Owenership
		/// is NOT transferred!
		/// @return A pointer to the ACraft preset instance. OWNERSHIP IS NOT TRANSFERRED!
		const ACraft* GetDeliveryCraft() const { return m_pDeliveryCraft; }

		/// Sets the preset of the delivery craft set for this loadout. Owenership
		/// is NOT transferred!
		/// @param pCraft A pointer to the ACraft preset instance. OWNERSHIP IS NOT TRANSFERRED!
		void SetDeliveryCraft(const ACraft* pCraft) {
			m_pDeliveryCraft = pCraft;
			m_UnavailableDeliveryCraft = UnavailableReference();
		}

		/// Gets the list of cargo Entity items this Loadout represents.
		/// @return A pointer to the list of cargo Entity items. OWNERSHIP IS NOT TRANSFERRED!
		std::list<const SceneObject*>* GetCargoList() { return &m_CargoItems; }

		/// Adds a new Preset to the list of cargo items to be included in this.
		/// @param pNewItem A const pointer to the ScneObject preset we want to add to this loadout.
		void AddToCargoList(const SceneObject* pNewItem) {
			if (pNewItem)
				m_CargoItems.push_back(pNewItem);
		}

		/// Protected member variable and method declarations
	protected:
		// Member variables
		static Entity::ClassInfo m_sClass;
		// Preset instance of the delivery craft, not owned by this.
		const ACraft* m_pDeliveryCraft;
		// The cargo of this loadout, all preset instances not owned by this
		std::list<const SceneObject*> m_CargoItems;

		/// Private member variable and method declarations
	private:
		/// A reference this Loadout read but can't use right now, such as into a mod that isn't loaded. It's kept as it was written, as this Loadout isn't what should decide that it's gone.
		struct UnavailableReference {
			std::string m_Value; //!< What the property held, which is empty if there's no reference at all.
			std::string m_NestedPresetName; //!< The preset's name, for an old reference with no module, which can't be put in the current form. Empty otherwise.
			size_t m_Position = 0; //!< For a cargo item, how many usable cargo items were ahead of it, as what's in a Loadout depends on its order.
		};

		UnavailableReference m_UnavailableDeliveryCraft; //!< The delivery craft, if there's one and it can't be used right now.
		std::vector<UnavailableReference> m_UnavailableCargoItems; //!< The cargo items that can't be used right now, in order.

		/// Reads what a Loadout property refers to.
		/// A player's own file is never reported on, as whatever's wrong with it is most likely just old, and it may be in the form from before preset references. A module's file is held to what the game expects of any data, so anything wrong with it is an error.
		/// @param reader The Reader which is about to read the property's value.
		/// @param asWritten Set to what was read, for keeping it if it turns out to be unusable.
		/// @return The preset referred to, or nullptr if it couldn't be found.
		static const Entity* ReadReference(Reader& reader, UnavailableReference& asWritten);

		/// Writes a reference that can't be used right now, as it was read.
		/// @param writer The Writer to write the property with.
		/// @param propName The name of the property.
		/// @param reference The reference to write.
		static void SaveUnavailableReference(Writer& writer, const std::string& propName, const UnavailableReference& reference);

		/// Clears all the member variables of this Loadout, effectively
		/// resetting the members of this abstraction level only.
		void Clear();
	};

} // namespace RTE
