// Suppress compiler warning about unrecognized escape sequence in PresetMan::LoadAllDataModules.
#pragma warning( disable : 4129 )

#include "PresetMan.h"

#include "SettingsMan.h"
#include "ModuleMan.h"
#include "ConsoleMan.h"
#include "LoadingScreen.h"

#include "DataModule.h"
#include "SceneObject.h"
#include "Loadout.h"
#include "ACraft.h"

namespace RTE {

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PresetMan::Clear() {
		m_TotalGroupRegister.clear();
		m_ReloadEntityPresetCalledThisUpdate = false;
		m_LastReloadedEntityPresetInfo.fill("");
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool PresetMan::AddEntityPreset(Entity *pEntToAdd, int whichModule, bool overwriteSame, std::string readFromFile) {
		RTEAssert(whichModule >= 0 && whichModule < m_pDataModules.size(), "Tried to access an out of bounds data module number!");

		return m_pDataModules[whichModule]->AddEntityPreset(pEntToAdd, overwriteSame, readFromFile);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	const Entity * PresetMan::GetEntityPreset(std::string type, std::string preset, int whichModule) {
		RTEAssert(whichModule < (int)m_pDataModules.size(), "Tried to access an out of bounds data module number!");

		const Entity *pRetEntity = 0;

		// Preset name might have "[ModuleName]/" preceding it, detect it here and select proper module!
		int slashPos = preset.find_first_of('/');
		if (slashPos != std::string::npos) {
			// Get the module ID and cut off the module specifier in the string
			whichModule = GetModuleID(preset.substr(0, slashPos));
			preset = preset.substr(slashPos + 1);
		}

		// All modules
		if (whichModule < 0) {
			// Search all modules
			for (int i = 0; i < m_pDataModules.size() && !pRetEntity; ++i) {
				pRetEntity = m_pDataModules[i]->GetEntityPreset(type, preset);
			}
		} else {
			// Try to get it from the asked for module
			pRetEntity = m_pDataModules[whichModule]->GetEntityPreset(type, preset);

			// If couldn't find it in there, then try all the official modules!
			if (!pRetEntity) {
				RTEAssert(m_OfficialModuleCount <= m_pDataModules.size(), "More official modules than modules loaded?!");
				for (int i = 0; i < m_OfficialModuleCount && !pRetEntity; ++i) {
					pRetEntity = m_pDataModules[i]->GetEntityPreset(type, preset);
				}
			}
		}

		return pRetEntity;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	const Entity * PresetMan::GetEntityPreset(Reader &reader) {
		// The reader is aware of which DataModule it is reading within
		int whichModule = reader.GetReadModuleID();
		RTEAssert(whichModule >= 0 && whichModule < m_pDataModules.size(), "Reader has an out of bounds module number!");

		std::string ClassName;
		const Entity::ClassInfo *pClass = 0;
		Entity *pNewInstance = 0;
		const Entity *pReturnPreset = 0;
		// Load class name and then preset instance
		reader >> ClassName;
		pClass = Entity::ClassInfo::GetClass(ClassName);

		if (pClass && pClass->IsConcrete()) {
			// Instantiate
			pNewInstance = pClass->NewInstance();

			// Get this before reading entity, since if it's the last one in its datafile, the stream will show the parent file instead
			std::string entityFilePath = reader.GetCurrentFilePath();

			// Try to read in the preset instance's data from the reader
			if (pNewInstance && pNewInstance->Create(reader, false) < 0) {
				// Abort loading if we can't create entity and it's not in a module that allows ignoring missing items.
				if (!g_PresetMan.GetDataModule(whichModule)->GetIgnoreMissingItems()) {
					RTEAbort("Reading of a preset instance \"" + pNewInstance->GetPresetName() + "\" of class " + pNewInstance->GetClassName() + " failed in file " + reader.GetCurrentFilePath() + ", shortly before line #" + reader.GetCurrentFileLine());
				}
			} else if (pNewInstance) {
				// Try to add the instance to the collection
				m_pDataModules[whichModule]->AddEntityPreset(pNewInstance, reader.GetPresetOverwriting(), entityFilePath);

				// Regardless of whether there was a collision or not, use whatever now exists in the instance map of that class and name
				pReturnPreset = m_pDataModules[whichModule]->GetEntityPreset(pNewInstance->GetClassName(), pNewInstance->GetPresetName());
				// If the instance wasn't found in the specific DataModule, try to find it in all the official ones instead
				if (!pReturnPreset) {
					RTEAssert(m_OfficialModuleCount <= m_pDataModules.size(), "More official modules than modules loaded?!");
					for (int i = 0; i < m_OfficialModuleCount && !pReturnPreset; ++i) {
						pReturnPreset = m_pDataModules[i]->GetEntityPreset(pNewInstance->GetClassName(), pNewInstance->GetPresetName());
					}
				}
			}
			// Get rid of the read-in instance as its copy is now either added to the map, or discarded as there already was somehting in there of the same name.
			delete pNewInstance; pNewInstance = 0;
		} else {
			pReturnPreset = 0;
		}
		return pReturnPreset;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	Entity * PresetMan::ReadReflectedPreset(Reader &reader) {
		// The reader is aware of which DataModule it's reading within
		int whichModule = reader.GetReadModuleID();
		RTEAssert(whichModule >= 0 && whichModule < m_pDataModules.size(), "Reader has an out of bounds module number!");

		std::string ClassName;
		const Entity::ClassInfo *pClass = 0;
		Entity *pNewInstance = 0;
		// Load class name and then preset instance
		reader >> ClassName;
		pClass = Entity::ClassInfo::GetClass(ClassName);

		if (pClass && pClass->IsConcrete()) {
			// Instantiate
			pNewInstance = pClass->NewInstance();

			// Get this before reading entity, since if it's the last one in its datafile, the stream will show the parent file instead
			std::string entityFilePath = reader.GetCurrentFilePath();

			// Try to read in the preset instance's data from the reader
			if (pNewInstance && pNewInstance->Create(reader, false) < 0) {
				if (!g_PresetMan.GetDataModule(whichModule)->GetIgnoreMissingItems()) {
					RTEAbort("Reading of a preset instance \"" + pNewInstance->GetPresetName() + "\" of class " + pNewInstance->GetClassName() + " failed in file " + reader.GetCurrentFilePath() + ", shortly before line #" + reader.GetCurrentFileLine());
				}
			} else {
				// Try to add the instance to the collection.
				// Note that we'll return this instance regardless of whether the adding was succesful or not
				m_pDataModules[whichModule]->AddEntityPreset(pNewInstance, reader.GetPresetOverwriting(), entityFilePath);
				return pNewInstance;
			}
		}

		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool PresetMan::GetAllOfType(std::list<Entity *> &entityList, std::string type, int whichModule) {
		if (type.empty()) {
			return false;
		}

		bool foundAny = false;

		// All modules
		if (whichModule < 0) {
			// Send the list to each module
			for (int i = 0; i < m_pDataModules.size(); ++i) {
				foundAny = m_pDataModules[i]->GetAllOfType(entityList, type) || foundAny;
			}
		}
		// Specific module
		else {
			RTEAssert(whichModule < m_pDataModules.size(), "Trying to get from an out of bounds DataModule ID!");
			foundAny = m_pDataModules[whichModule]->GetAllOfType(entityList, type);
		}

		return foundAny;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool PresetMan::GetAllOfTypeInModuleSpace(std::list<Entity *> &entityList, std::string type, int whichModuleSpace) {
		if (type.empty()) {
			return false;
		}

		bool foundAny = false;

		// All modules
		if (whichModuleSpace < 0) {
			foundAny = GetAllOfType(entityList, type, whichModuleSpace);
		} else {
			// Get all entitys of the specific type in the official modules loaded before the specified one
			for (int module = 0; module < m_OfficialModuleCount && module < whichModuleSpace; ++module) {
				foundAny = GetAllOfType(entityList, type, module) || foundAny;
			}
			// Now get the groups of the specified module (official or not)
			foundAny = GetAllOfType(entityList, type, whichModuleSpace) || foundAny;
		}

		return foundAny;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool PresetMan::GetAllOfGroups(std::list<Entity *> &entityList, const std::vector<std::string> &groups, const std::string &type, int whichModule) {
		RTEAssert(!groups.empty(), "Looking for empty groups in PresetMan::GetAllOfGroups!");
		bool foundAny = false;

		if (whichModule < 0) {
			for (DataModule *dataModule : m_pDataModules) {
				foundAny = dataModule->GetAllOfGroups(entityList, groups, type) || foundAny;
			}
		} else {
			RTEAssert(whichModule < m_pDataModules.size(), "Trying to get from an out of bounds DataModule ID in PresetMan::GetAllOfGroups!");
			foundAny = m_pDataModules[whichModule]->GetAllOfGroups(entityList, groups, type);
		}
		return foundAny;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool PresetMan::GetAllNotOfGroups(std::list<Entity *> &entityList, const std::vector<std::string> &groups, const std::string &type, int whichModule) {
		if (groups.empty()) {
			RTEAbort("Looking for empty groups in PresetMan::GetAllNotOfGroups!");
		} else if (std::find(groups.begin(), groups.end(), "All") != groups.end()) {
			RTEAbort("Trying to exclude all groups while looking for presets in PresetMan::GetAllNotOfGroups!");
		}

		bool foundAny = false;

		if (whichModule < 0) {
			for (DataModule *dataModule : m_pDataModules) {
				foundAny = dataModule->GetAllNotOfGroups(entityList, groups, type) || foundAny;
			}
		} else {
			RTEAssert(whichModule < m_pDataModules.size(), "Trying to get from an out of bounds DataModule ID in PresetMan::GetAllNotOfGroups!");
			foundAny = m_pDataModules[whichModule]->GetAllNotOfGroups(entityList, groups, type);
		}
		return foundAny;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	Entity * PresetMan::GetRandomOfGroup(std::string group, std::string type, int whichModule) {
		RTEAssert(!group.empty(), "Looking for empty group!");

		bool foundAny = false;
		// The total list we'll select a random one from
		std::list<Entity *> entityList;

		// All modules
		if (whichModule < 0) {
			// Get from all modules
			for (int i = 0; i < m_pDataModules.size(); ++i) {
				// Send the list to each module, let them add
				foundAny = m_pDataModules[i]->GetAllOfGroups(entityList, { group }, type) || foundAny;
			}
		} else {
			RTEAssert(whichModule < m_pDataModules.size(), "Trying to get from an out of bounds DataModule ID!");
			foundAny = m_pDataModules[whichModule]->GetAllOfGroups(entityList, { group }, type);
		}

		// Didn't find any of that group in those module(s)
		if (!foundAny) {
			return 0;
		}

		// Pick one and return it
		int current = 0;
		int selection = RandomNum<int>(0, entityList.size() - 1);
		for (std::list<Entity *>::iterator itr = entityList.begin(); itr != entityList.end(); ++itr) {
			if (current == selection) {
				return (*itr);
			}
			current++;
		}

		RTEAssert(0, "Tried selecting randomly but didn't?");
		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	Entity * PresetMan::GetRandomBuyableOfGroupFromTech(std::string group, std::string type, int whichModule) {
		RTEAssert(!group.empty(), "Looking for empty group!");

		bool foundAny = false;
		// The total list we'll select a random one from
		std::list<Entity *> entityList;
		std::list<Entity *> tempList;

		// All modules
		if (whichModule < 0) {
			for (DataModule *dataModule : m_pDataModules) {
				if (dataModule->IsFaction()) {
					foundAny = dataModule->GetAllOfGroups(tempList, { group }, type) || foundAny;
				}
			}
		} else {
			RTEAssert(whichModule < m_pDataModules.size(), "Trying to get from an out of bounds DataModule ID!");
			foundAny = m_pDataModules[whichModule]->GetAllOfGroups(tempList, { group }, type);
		}

		//Filter found entities, we need only buyables
		if (foundAny) {
			//Do not filter anything if we're looking for brains
			if (group == "Brains") {
				foundAny = false;
				for (std::list<Entity *>::iterator oItr = tempList.begin(); oItr != tempList.end(); ++oItr) {
					entityList.push_back(*oItr);
					foundAny = true;
				}
			} else {
				foundAny = false;
				for (std::list<Entity *>::iterator oItr = tempList.begin(); oItr != tempList.end(); ++oItr) {
					SceneObject * pSObject = dynamic_cast<SceneObject *>(*oItr);
					// Buyable and not brain?
					if (pSObject && pSObject->IsBuyable() && !pSObject->IsBuyableInObjectPickerOnly() && !pSObject->IsInGroup("Brains")) {
						entityList.push_back(*oItr);
						foundAny = true;
					}
				}
			}
		}

		// Didn't find any of that group in those module(s)
		if (!foundAny) {
			return 0;
		}

		// Pick one and return it
		int current = 0;
		int selection = RandomNum<int>(0, entityList.size() - 1);

		int totalWeight = 0;
		for (std::list<Entity *>::iterator itr = entityList.begin(); itr != entityList.end(); ++itr) {
			totalWeight += (*itr)->GetRandomWeight();
		}

		// Use random weights if looking in specific modules
		if (whichModule >= 0) {
			if (totalWeight == 0) {
				return 0;
			}

			selection = RandomNum(0, totalWeight - 1);

			for (std::list<Entity *>::iterator itr = entityList.begin(); itr != entityList.end(); ++itr) {
				bool found = false;
				int bucketCounter = 0;

				if ((*itr)->GetRandomWeight() > 0) {
					while (bucketCounter < (*itr)->GetRandomWeight()) {
						if (current == selection) {
							found = true;
							break;
						}

						current++;
						bucketCounter++;
					}
				}

				if (found) {
					return (*itr);
				}
			}
		} else {
			for (std::list<Entity *>::iterator itr = entityList.begin(); itr != entityList.end(); ++itr) {
				if (current == selection) {
					return (*itr);
				}

				current++;
			}
		}

		RTEAssert(0, "Tried selecting randomly but didn't?");
		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool PresetMan::GetAllOfGroupInModuleSpace(std::list<Entity *> &entityList, std::string group, std::string type, int whichModuleSpace) {
		RTEAssert(!group.empty(), "Looking for empty group!");

		bool foundAny = false;

		// All modules
		if (whichModuleSpace < 0) {
			foundAny = GetAllOfGroup(entityList, group, type, whichModuleSpace);
		} else {
			// Get all entitys of the specific group the official modules loaded before the specified one
			for (int module = 0; module < m_OfficialModuleCount && module < whichModuleSpace; ++module) {
				foundAny = GetAllOfGroup(entityList, group, type, module) || foundAny;
			}

			// Now get the groups of the specified module (official or not)
			foundAny = GetAllOfGroup(entityList, group, type, whichModuleSpace) || foundAny;
		}

		return foundAny;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	Entity * PresetMan::GetRandomOfGroupInModuleSpace(std::string group, std::string type, int whichModuleSpace) {
		RTEAssert(!group.empty(), "Looking for empty group!");

		bool foundAny = false;
		// The total list we'll select a random one from
		std::list<Entity *> entityList;

		// All modules
		if (whichModuleSpace < 0) {
			foundAny = GetAllOfGroup(entityList, group, type, whichModuleSpace);
		} else {
			// Get all entitys of the specific group the official modules loaded before the specified one
			for (int module = 0; module < m_OfficialModuleCount && module < whichModuleSpace; ++module) {
				foundAny = GetAllOfGroup(entityList, group, type, module) || foundAny;
			}

			// Now get the groups of the specified module (official or not)
			foundAny = GetAllOfGroup(entityList, group, type, whichModuleSpace) || foundAny;
		}

		// Didn't find any of that group in those module(s)
		if (!foundAny) {
			return 0;
		}

		// Pick one and return it
		int current = 0;
		int selection = RandomNum<int>(0, entityList.size() - 1);
		for (std::list<Entity *>::iterator itr = entityList.begin(); itr != entityList.end(); ++itr) {
			if (current == selection) {
				return (*itr);
			}
			current++;
		}

		RTEAssert(0, "Tried selecting randomly but didn't?");
		return 0;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	std::string PresetMan::GetEntityDataLocation(std::string type, std::string preset, int whichModule) {
		RTEAssert(whichModule < (int)m_pDataModules.size(), "Tried to access an out of bounds data module number!");

		std::string pRetPath = "";

		// All modules
		if (whichModule < 0) {
			// Search all modules
			for (int i = 0; i < m_pDataModules.size() && pRetPath.empty(); ++i) {
				pRetPath = m_pDataModules[i]->GetEntityDataLocation(type, preset);
			}
		} else {
			// Try to get it from the asked for module
			pRetPath = m_pDataModules[whichModule]->GetEntityDataLocation(type, preset);

			// If couldn't find it in there, then try all the official modules!
			if (pRetPath.empty()) {
				RTEAssert(m_OfficialModuleCount <= m_pDataModules.size(), "More official modules than modules loaded?!");
				for (int i = 0; i < m_OfficialModuleCount && pRetPath.empty(); ++i) {
					pRetPath = m_pDataModules[i]->GetEntityDataLocation(type, preset);
				}
			}
		}

		return pRetPath;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PresetMan::ReloadAllScripts() const {
		g_LuaMan.ClearUserModuleCache();
		for (const DataModule *dataModule : m_pDataModules) {
			dataModule->ReloadAllScripts();
		}
		g_ConsoleMan.PrintString("SYSTEM: Scripts reloaded!");
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool PresetMan::ReloadEntityPreset(const std::string &presetName, const std::string &className, const std::string &moduleName, bool storeReloadedPresetDataForQuickReloading) {
		if (className.empty() || presetName.empty()) {
			g_ConsoleMan.PrintString("ERROR: Trying to reload Entity preset without specifying preset name or type!");
			return false;
		}

		int moduleId = -1;
		if (moduleName != "") {
			moduleId = GetModuleID(moduleName);
			if (moduleId < 0) {
				g_ConsoleMan.PrintString("ERROR: Failed to find data module with name \"" + moduleName + "\" while attempting to reload an Entity preset with name \"" + presetName + "\" of type \"" + className + "\"!");
				return false;
			}
		}
		std::string actualDataModuleOfPreset = moduleName;

		std::string presetDataLocation = GetEntityDataLocation(className, presetName, moduleId);
		if (presetDataLocation.empty()) {
			g_ConsoleMan.PrintString("ERROR: Failed to locate data of Entity preset with name \"" + presetName + "\" of type \"" + className + "\" in \"" + moduleName + "\" or any official module! The preset might not exist!");
			return false;
		}

		// GetEntityDataLocation will attempt to locate the preset in the official modules if it fails to locate it in the specified module. Warn and correct the result string.
		if (std::string presetDataLocationModuleName = presetDataLocation.substr(0, presetDataLocation.find_first_of("/\\")); presetDataLocationModuleName != actualDataModuleOfPreset) {
			actualDataModuleOfPreset = presetDataLocationModuleName;
			if (moduleName != "") {
				g_ConsoleMan.PrintString("WARNING: Failed to locate data of Entity preset with name \"" + presetName + "\" of type \"" + className + "\" in \"" + moduleName + "\"! Entity preset data matching the name and type was found in \"" + actualDataModuleOfPreset + "\"!");
			}
		}

		m_ReloadEntityPresetCalledThisUpdate = true;

		Reader reader(presetDataLocation.c_str(), true);
		while (reader.NextProperty()) {
			reader.ReadPropName();
			g_PresetMan.GetEntityPreset(reader);
		}
		g_ConsoleMan.PrintString("SYSTEM: Entity preset with name \"" + presetName + "\" of type \"" + className + "\" defined in \"" + actualDataModuleOfPreset + "\" was successfully reloaded");

		if (storeReloadedPresetDataForQuickReloading) {
			m_LastReloadedEntityPresetInfo[0] = presetName;
			m_LastReloadedEntityPresetInfo[1] = className;
			m_LastReloadedEntityPresetInfo[2] = moduleName == "" ? actualDataModuleOfPreset : moduleName; // If there was a module name, store it as-is so that if there's a data location warning, it persists on every quick reload.
		}
		return true;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool PresetMan::QuickReloadEntityPreset() {
		for (const std::string &entityPresetInfoEntry : m_LastReloadedEntityPresetInfo) {
			if (entityPresetInfoEntry.empty()) {
				g_ConsoleMan.PrintString("ERROR: Trying to quick reload Entity preset when there is nothing set to reload!");
				return false;
			}
		}
		return ReloadEntityPreset(m_LastReloadedEntityPresetInfo[0], m_LastReloadedEntityPresetInfo[1], m_LastReloadedEntityPresetInfo[2]);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool PresetMan::AddMaterialMapping(int fromID, int toID, int whichModule) {
		RTEAssert(whichModule >= m_OfficialModuleCount && whichModule < m_pDataModules.size(), "Tried to make a material mapping in an offical or out-of-bounds DataModule!");

		return m_pDataModules[whichModule]->AddMaterialMapping(fromID, toID);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PresetMan::RegisterGroup(std::string newGroup, int whichModule) {
		RTEAssert(whichModule >= 0 && whichModule < (int)m_pDataModules.size(), "Tried to access an out of bounds data module number!");

		// Register in the handy total list
		m_TotalGroupRegister.push_back(newGroup);
		m_TotalGroupRegister.sort();
		m_TotalGroupRegister.unique();

		// Register in the specified module too
		m_pDataModules[whichModule]->RegisterGroup(newGroup);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool PresetMan::GetGroups(std::list<std::string> &groupList, int whichModule, std::string withType) const {
		RTEAssert(whichModule < (int)m_pDataModules.size(), "Tried to access an out of bounds data module number!");

		bool foundAny = false;

		// Asked for ALL groups ever registered
		if (whichModule < 0) {
			// Get all applicable groups
			if (withType == "All" || withType.empty()) {
				for (std::list<std::string>::const_iterator gItr = m_TotalGroupRegister.begin(); gItr != m_TotalGroupRegister.end(); ++gItr) {
					groupList.push_back(*gItr);
				}

				foundAny = !m_TotalGroupRegister.empty();
			} else {
				// Filter out groups without any entitys of the specified type
				for (int module = 0; module < (int)m_pDataModules.size(); ++module) {
					foundAny = m_pDataModules[module]->GetGroupsWithType(groupList, withType) || foundAny;
				}
			}
		} else if (!m_pDataModules[whichModule]->GetGroupRegister()->empty()) {
			// Asked for specific DataModule's groups
			// Get ALL groups of that module
			if (withType == "All" || withType.empty()) {
				const std::list<std::string> *pGroupList = m_pDataModules[whichModule]->GetGroupRegister();
				for (std::list<std::string>::const_iterator gItr = pGroupList->begin(); gItr != pGroupList->end(); ++gItr) {
					groupList.push_back(*gItr);
				}

				foundAny = !pGroupList->empty();
			} else {
				// Get only modules that contain an entity of valid type
				foundAny = m_pDataModules[whichModule]->GetGroupsWithType(groupList, withType) || foundAny;
			}
		}

		return foundAny;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool PresetMan::GetModuleSpaceGroups(std::list<std::string> &groupList, int whichModule, std::string withType) const {
		RTEAssert(whichModule < (int)m_pDataModules.size(), "Tried to access an out of bounds data module number!");

		bool foundAny = false;

		// If all, then just copy the total register
		if (whichModule < 0) {
			// Just get all groups ever registered
			if (withType == "All" || withType.empty()) {
				for (std::list<std::string>::const_iterator gItr = m_TotalGroupRegister.begin(); gItr != m_TotalGroupRegister.end(); ++gItr) {
					groupList.push_back(*gItr);
				}

				foundAny = !m_TotalGroupRegister.empty();
			} else {
				// Get type filtered groups from ALL data modules
				for (int module = 0; module < (int)m_pDataModules.size(); ++module) {
					foundAny = GetGroups(groupList, module, withType) || foundAny;
				}
			}
		} else {
			// Getting modulespace of specific module
			// Get all groups of the official modules that are loaded before the specified one
			for (int module = 0; module < m_OfficialModuleCount && module < whichModule; ++module)
				foundAny = GetGroups(groupList, module, withType) || foundAny;

			// Now get the groups of the specified module (official or not)
			foundAny = GetGroups(groupList, whichModule, withType) || foundAny;

			// Make sure there are no dupe groups in the list
			groupList.sort();
			groupList.unique();
		}

		return foundAny;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	Actor * PresetMan::GetLoadout(std::string loadoutName, std::string module, bool spawnDropShip) {
		return GetLoadout(loadoutName, GetModuleID(module), spawnDropShip);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	Actor * PresetMan::GetLoadout(std::string loadoutName, int moduleNumber, bool spawnDropShip) {
		if (spawnDropShip) {
			// Find the Loadout that this Deployment is referring to
			const Loadout *pLoadout = dynamic_cast<const Loadout *>(g_PresetMan.GetEntityPreset("Loadout", loadoutName, moduleNumber));
			if (pLoadout) {
				const ACraft * pCraftPreset = pLoadout->GetDeliveryCraft();
				if (pCraftPreset) {
					ACraft * pCraft = dynamic_cast<ACraft *>(pCraftPreset->Clone());
					if (pCraft) {
						float tally = 0;
						// Create and pass along the first Actor and his inventory defined in the Loadout
						Actor * pActor = pLoadout->CreateFirstActor(moduleNumber, 1, 1, tally);
						// Set the position and team etc for the Actor we are prepping to spawn
						if (pActor)
							pCraft->AddInventoryItem(pActor);
					}
					return pCraft;
				}
			}
		} else {
			// Find the Loadout that this Deployment is referring to
			const Loadout *pLoadout = dynamic_cast<const Loadout *>(g_PresetMan.GetEntityPreset("Loadout", loadoutName, moduleNumber));
			if (pLoadout) {
				float tally = 0;
				// Create and pass along the first Actor and his inventory defined in the Loadout
				Actor * pReturnActor = pLoadout->CreateFirstActor(moduleNumber, 1, 1, tally);
				// Set the position and team etc for the Actor we are prepping to spawn
				return pReturnActor;
			}
		}

		return 0;
	}
}