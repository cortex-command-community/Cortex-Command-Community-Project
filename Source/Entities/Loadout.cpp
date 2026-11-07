#include "Loadout.h"
#include "PresetMan.h"
#include "MovableObject.h"
#include "ACraft.h"

using namespace RTE;

ConcreteClassInfo(Loadout, Entity, 0);

void Loadout::Clear() {
	m_pDeliveryCraft = 0;
	m_CargoItems.clear();
	m_UnavailableDeliveryCraft = UnavailableReference();
	m_UnavailableCargoItems.clear();
}

/*
int Loadout::Create()
{
    if (Entity::Create() < 0)
        return -1;

    return 0;
}
*/

int Loadout::Create(const Loadout& reference) {
	Entity::Create(reference);

	m_UnavailableDeliveryCraft = reference.m_UnavailableDeliveryCraft;
	m_UnavailableCargoItems = reference.m_UnavailableCargoItems;
	// These are preset instances, not owned by the reference or this.
	m_pDeliveryCraft = reference.m_pDeliveryCraft;

	m_CargoItems.clear();
	for (std::list<const SceneObject*>::const_iterator itr = reference.m_CargoItems.begin(); itr != reference.m_CargoItems.end(); ++itr)
		m_CargoItems.push_back(*itr);

	return 0;
}

const Entity* Loadout::ReadReference(Reader& reader, UnavailableReference& asWritten) {
	std::string value;
	reader >> value;

	if (value == "None") {
		return nullptr;
	}

	const bool playersFile = reader.GetNonModulePath();

	// This is how a player's loadouts were saved before preset references, as a class with the preset's name nested under it. Anyone's old loadouts should keep working whenever they turn up.
	if (value.find('/') == std::string::npos && Entity::ClassInfo::GetClass(value) != nullptr) {
		reader.ReadPropName();
		const std::string presetName = reader.ReadPropValue();
		reader.NextProperty();

		if (!playersFile) {
			reader.ReportError("A Loadout in a module should refer to a preset in the form <class/module/preset name>, not with \"" + presetName + "\" nested under " + value);
		}

		if (presetName.find('/') == std::string::npos) {
			asWritten.m_Value = value;
			asWritten.m_NestedPresetName = presetName;
			return g_PresetMan.GetEntityPreset(value, presetName);
		}

		value += "/" + presetName;
	}

	asWritten.m_Value = value;
	return playersFile ? g_PresetMan.FindPresetForUserdata(value) : g_PresetMan.GetEntityPresetFromCharacteristic(value, reader);
}

void Loadout::SaveUnavailableReference(Writer& writer, const std::string& propName, const UnavailableReference& reference) {
	if (reference.m_NestedPresetName.empty()) {
		writer.NewPropertyWithValue(propName, reference.m_Value);
	} else {
		writer.NewProperty(propName);
		writer.ObjectStart(reference.m_Value);
		writer.NewPropertyWithValue("PresetName", reference.m_NestedPresetName);
		writer.ObjectEnd();
	}
}

int Loadout::ReadProperty(const std::string_view& propName, Reader& reader) {
	StartPropertyList(return Entity::ReadProperty(propName, reader));

	MatchProperty("DeliveryCraft", {
		UnavailableReference asWritten;
		const Entity* entityReference = ReadReference(reader, asWritten);
		const ACraft* reference = dynamic_cast<const ACraft*>(entityReference);
		if (entityReference != nullptr && reference == nullptr && !reader.GetNonModulePath()) {
			reader.ReportError("Tried to point DeliveryCraft at a " + entityReference->GetClassName() + "!");
		}
		SetDeliveryCraft(reference);
		if (reference == nullptr) {
			m_UnavailableDeliveryCraft = asWritten;
		}
	});
	MatchProperty("_ClearCargoItems", {
		reader.ReadPropValue();
		m_CargoItems.clear();
		m_UnavailableCargoItems.clear();
	});
	MatchProperty("AddCargoItem", {
		UnavailableReference asWritten;
		const Entity* entityReference = ReadReference(reader, asWritten);
		const MovableObject* reference = dynamic_cast<const MovableObject*>(entityReference);
		if (reference != nullptr) {
			m_CargoItems.push_back(reference);
		} else {
			if (entityReference != nullptr && !reader.GetNonModulePath()) {
				reader.ReportError("Tried to point a cargo item at a " + entityReference->GetClassName() + "!");
			}
			if (!asWritten.m_Value.empty()) {
				asWritten.m_Position = m_CargoItems.size();
				m_UnavailableCargoItems.push_back(asWritten);
			}
		}
	});

	EndPropertyList;
}

int Loadout::Save(Writer& writer) const {
	Entity::Save(writer);

	if (m_pDeliveryCraft) {
		writer.NewPropertyWithValue("DeliveryCraft", m_pDeliveryCraft->GetEntityCharacteristic());
	} else if (!m_UnavailableDeliveryCraft.m_Value.empty()) {
		SaveUnavailableReference(writer, "DeliveryCraft", m_UnavailableDeliveryCraft);
	}

	// Everything goes back where it was, as a mod coming back should find its items still in order among the rest.
	std::vector<UnavailableReference>::const_iterator unavailable = m_UnavailableCargoItems.begin();
	size_t position = 0;
	for (std::list<const SceneObject*>::const_iterator itr = m_CargoItems.begin(); itr != m_CargoItems.end(); ++itr, ++position) {
		for (; unavailable != m_UnavailableCargoItems.end() && unavailable->m_Position <= position; ++unavailable) {
			SaveUnavailableReference(writer, "AddCargoItem", *unavailable);
		}
		writer.NewPropertyWithValue("AddCargoItem", (*itr)->GetEntityCharacteristic());
	}
	for (; unavailable != m_UnavailableCargoItems.end(); ++unavailable) {
		SaveUnavailableReference(writer, "AddCargoItem", *unavailable);
	}

	return 0;
}

int Loadout::Write(Writer& writer, const Entity& entityReference, HashingData& hashData) const {
	Entity::Write(writer, entityReference, hashData);

	const Loadout& reference = static_cast<const Loadout&>(entityReference);

	writer.NewPresetReferenceProperty("DeliveryCraft", m_pDeliveryCraft, reference.m_pDeliveryCraft);
	writer.NewPresetReferenceSequence("_ClearCargoItems", "AddCargoItem", m_CargoItems, hashData);

	return 0;
}


HashingData Loadout::Hash() const {
	HashingData hashData(Entity::Hash());
	uint64_t& hash = hashData.m_Hash;
	
	AddToHash(hash, (m_pDeliveryCraft ? RTE::Hash(m_pDeliveryCraft->GetEntityCharacteristic()) : 0));

	int i = 0;

	for (const SceneObject* pCargo: m_CargoItems) {
		uint64_t cargoHash = RTE::Hash(pCargo->GetEntityCharacteristic());
		hashData.m_Constituents.push_back(cargoHash);
		AddToHash(hash, cargoHash);
		i++;
	}

	hashData.m_ParseValues.push_back(i);

	return hashData;
}

/*
void Loadout::Destroy(bool notInherited)
{
//  delete;

    if (!notInherited)
        Entity::Destroy();
    Clear();
}
*/

Actor* Loadout::CreateFirstActor(int nativeModule, float foreignMult, float nativeMult, float& costTally) const {
	// The Actor instance we return and pass ownership of
	Actor* pReturnActor = 0;

	// Now go through the cargo list of items and load new instances of all
	// Devices into inventory of an instance of the first Actor found in the list
	if (!m_CargoItems.empty()) {
		// Go through the list of things ordered, and give any actors all the items that is present after them,
		// until the next actor. Also, the first actor gets all stuff in the list above him.
		MovableObject* pInventoryObject = 0;
		Actor* pActor = nullptr;
		std::list<MovableObject*> tempCargo;
		for (std::list<const SceneObject*>::const_iterator itr = m_CargoItems.begin(); itr != m_CargoItems.end(); ++itr) {
			// Add to the total cost tally
			costTally += (*itr)->GetGoldValue(nativeModule, foreignMult, nativeMult);
			// Make copy of the preset instance in the list
			pInventoryObject = dynamic_cast<MovableObject*>((*itr)->Clone());
			// See if it's actually a passenger, as opposed to a regular item
			pActor = dynamic_cast<Actor*>(pInventoryObject);
			// If it's an actor, then set its team and add it to the Craft's inventory!
			if (pActor) {
				// If this is the first passenger, then give him all the shit found in the list before him
				if (!pReturnActor) {
					for (std::list<MovableObject*>::iterator iItr = tempCargo.begin(); iItr != tempCargo.end(); ++iItr)
						pActor->AddInventoryItem(*iItr);
				}
				// This isn't the first passenger, so give the previous guy all the stuff that was found since processing him
				else {
					// Go through the temporary list and give the previous, real first actor all the stuff
					for (std::list<MovableObject*>::iterator iItr = tempCargo.begin(); iItr != tempCargo.end(); ++iItr)
						pReturnActor->AddInventoryItem(*iItr);
					// Clear out the temporary cargo list since we've assigned all the stuff in it to the return Actor
					tempCargo.clear();

					// REMOVE the value of this bookend actor from the total cost tally - he is not included!
					costTally -= pActor->GetGoldValue(nativeModule, foreignMult, nativeMult);
					// Also stop going through the list; we only need ONE Actor.. so destroy the instance we jsut created of a second one.
					delete pActor;
					pActor = nullptr;
					// STOP!
					break;
				}
				// Clear out the temporary cargo list since we've assigned all the stuff in it to the return Actor
				tempCargo.clear();

				// Now set the current Actor as the one we return; he'll eventually get everything found after him as well
				pReturnActor = pActor;
				// Set the position and team etc for the Actor we are prepping to spawn
				pReturnActor->SetControllerMode(Controller::CIM_AI);
				pReturnActor->SetAIMode(Actor::AIMODE_SENTRY);
			}
			// If not an Actor, then add it to the temp list of items which will be added to the last passenger's inventory
			else
				tempCargo.push_back(pInventoryObject);
		}
		pActor = 0;

		// If there was a last passenger and only things after him, stuff all the items into his inventory
		if (pReturnActor) {
			// Passing ownership
			for (std::list<MovableObject*>::iterator iItr = tempCargo.begin(); iItr != tempCargo.end(); ++iItr)
				pReturnActor->AddInventoryItem(*iItr);
			tempCargo.clear();
		}
		// If there wa NO actor in the Loadout's cargo list, then see if there's a craft we can stuff thigns into instead
		else if (m_pDeliveryCraft) {
			// The craft is now the Actor we are spawning, so make an instance
			pReturnActor = dynamic_cast<Actor*>(m_pDeliveryCraft->Clone());
			// Add the cost of the ship
			costTally += pReturnActor->GetGoldValue(nativeModule, foreignMult, nativeMult);
			// Fill it with the stuff, passing ownership
			for (std::list<MovableObject*>::iterator iItr = tempCargo.begin(); iItr != tempCargo.end(); ++iItr)
				pReturnActor->AddInventoryItem(*iItr);
			tempCargo.clear();
		}
	}

	// PASSING OWNERSHIP
	return pReturnActor;
}

SceneObject* Loadout::CreateFirstDevice(int nativeModule, float foreignMult, float nativeMult, float& costTally) const {
	// The Actor instance we return and pass ownership of
	SceneObject* pReturnObject = 0;

	// Now go through the Loadout list of items and load new instances of all
	// Devices into inventory of an instance of the first Actor found in the list
	if (!m_CargoItems.empty()) {
		for (std::list<const SceneObject*>::const_iterator itr = m_CargoItems.begin(); itr != m_CargoItems.end(); ++itr) {
			// If not an Actor, then we should create and return it.
			if (!dynamic_cast<const Actor*>(*itr)) {
				pReturnObject = dynamic_cast<SceneObject*>((*itr)->Clone());
				// Add to the total cost tally
				costTally += pReturnObject->GetGoldValue(nativeModule, foreignMult, nativeMult);
				// We're done
				break;
			}
		}
	}

	// PASSING OWNERSHIP
	return pReturnObject;
}
