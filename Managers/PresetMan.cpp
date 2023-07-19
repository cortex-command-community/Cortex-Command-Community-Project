#include "PresetMan.h"

#include "ModuleMan.h"
#include "SettingsMan.h"
#include "ConsoleMan.h"

#include "SceneObject.h"
#include "Loadout.h"
#include "ACraft.h"

namespace RTE {

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PresetMan::Clear() {
		m_ReloadEntityPresetCalledThisUpdate = false;
		m_LastReloadedEntityPresetInfo.fill("");
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PresetMan::RegisterGroup(const std::string &newGroup, int whichModule) {
		RTEAssert(whichModule >= 0 && whichModule < g_ModuleMan.GetTotalModuleCount(), "Tried to access an out of bounds data module number!");

		g_ModuleMan.GetLoadedDataModules()[whichModule]->RegisterGroup(newGroup);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool PresetMan::GetGroups(std::list<std::string> &groupList, int whichModule, const std::string &withType, bool moduleSpace) const {
		RTEAssert(whichModule < g_ModuleMan.GetTotalModuleCount(), "Tried to access an out of bounds data module number!");

		bool foundAny = false;
		auto &loadedModules = g_ModuleMan.GetLoadedDataModules();

		if (whichModule < 0) {
			for (DataModule *dataModule : loadedModules) {
				foundAny = dataModule->GetGroupsWithType(groupList, withType) || foundAny;
			}
		} else {
			if (moduleSpace) {
				for (size_t officialModuleID = 0; officialModuleID < g_ModuleMan.GetOfficialModuleCount(); ++officialModuleID) {
					foundAny = loadedModules[officialModuleID]->GetGroupsWithType(groupList, withType) || foundAny;
				}
			}
			foundAny = loadedModules[whichModule]->GetGroupsWithType(groupList, withType) || foundAny;
		}
		if (!groupList.empty() && (withType == "All" || withType.empty())) {
			// DataModule::GetGroupsWithType doesn't sort and filter dupes when getting all groups, so do it here.
			groupList.sort();
			groupList.unique();
		}
		return foundAny;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	const Entity * PresetMan::GetEntityPreset(Reader &reader) {
		// The reader is aware of which DataModule it is reading within
		int whichModule = reader.GetReadModuleID();
		RTEAssert(whichModule >= 0 && whichModule < g_ModuleMan.GetTotalModuleCount(), "Reader has an out of bounds module number!");

		std::string ClassName;
		const Entity::ClassInfo *pClass = nullptr;
		Entity *newInstance = nullptr;
		const Entity *pReturnPreset = nullptr;
		// Load class name and then preset instance
		reader >> ClassName;
		pClass = Entity::ClassInfo::GetClass(ClassName);

		auto &loadedModules = g_ModuleMan.GetLoadedDataModules();

		if (pClass && pClass->IsConcrete()) {
			// Instantiate
			newInstance = pClass->NewInstance();

			// Get this before reading entity, since if it's the last one in its datafile, the stream will show the parent file instead
			std::string entityFilePath = reader.GetCurrentFilePath();

			// Try to read in the preset instance's data from the reader
			if (newInstance && newInstance->Create(reader, false) < 0) {
				// Abort loading if we can't create entity and it's not in a module that allows ignoring missing items.
				if (!g_ModuleMan.GetDataModule(whichModule)->GetIgnoreMissingItems()) {
					RTEAbort("Reading of a preset instance \"" + newInstance->GetPresetName() + "\" of class " + newInstance->GetClassName() + " failed in file " + reader.GetCurrentFilePath() + ", shortly before line #" + reader.GetCurrentFileLine());
				}
			} else if (newInstance) {
				// Try to add the instance to the collection
				loadedModules[whichModule]->AddEntityPreset(newInstance, reader.GetPresetOverwriting(), entityFilePath);

				// Regardless of whether there was a collision or not, use whatever now exists in the instance map of that class and name
				pReturnPreset = loadedModules[whichModule]->GetEntityPreset(newInstance->GetClassName(), newInstance->GetPresetName());
				// If the instance wasn't found in the specific DataModule, try to find it in all the official ones instead
				if (!pReturnPreset) {
					RTEAssert(g_ModuleMan.GetOfficialModuleCount() <= loadedModules.size(), "More official modules than modules loaded?!");
					for (int i = 0; i < g_ModuleMan.GetOfficialModuleCount() && !pReturnPreset; ++i) {
						pReturnPreset = loadedModules[i]->GetEntityPreset(newInstance->GetClassName(), newInstance->GetPresetName());
					}
				}
			}
			// Get rid of the read-in instance as its copy is now either added to the map, or discarded as there already was somehting in there of the same name.
			delete newInstance;
			newInstance = nullptr;
		} else {
			pReturnPreset = nullptr;
		}
		return pReturnPreset;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	const Entity * PresetMan::GetEntityPreset(const std::string &typeName, std::string presetName, int whichModule) const {
		RTEAssert(whichModule < g_ModuleMan.GetTotalModuleCount(), "Tried to access an out of bounds data module number!");

		const Entity *pRetEntity = nullptr;

		auto &loadedModules = g_ModuleMan.GetLoadedDataModules();

		// Preset name might have "[ModuleName]/" preceding it, detect it here and select proper module!
		int slashPos = presetName.find_first_of('/');
		if (slashPos != std::string::npos) {
			// Get the module ID and cut off the module specifier in the string
			whichModule = g_ModuleMan.GetModuleID(presetName.substr(0, slashPos));
			presetName = presetName.substr(slashPos + 1);
		}

		// All modules
		if (whichModule < 0) {
			// Search all modules
			for (int i = 0; i < loadedModules.size() && !pRetEntity; ++i) {
				pRetEntity = loadedModules[i]->GetEntityPreset(typeName, presetName);
			}
		} else {
			// Try to get it from the asked for module
			pRetEntity = loadedModules[whichModule]->GetEntityPreset(typeName, presetName);

			// If couldn't find it in there, then try all the official modules!
			if (!pRetEntity) {
				RTEAssert(g_ModuleMan.GetOfficialModuleCount() <= loadedModules.size(), "More official modules than modules loaded?!");
				for (int i = 0; i < g_ModuleMan.GetOfficialModuleCount() && !pRetEntity; ++i) {
					pRetEntity = loadedModules[i]->GetEntityPreset(typeName, presetName);
				}
			}
		}

		return pRetEntity;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	const Entity * PresetMan::GetEntityPreset(const std::string &typeName, const std::string &presetName, const std::string &whichModule) const {
		return GetEntityPreset(typeName, presetName, g_ModuleMan.GetModuleID(whichModule));
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	std::string PresetMan::GetEntityDataLocation(const std::string &type, const std::string &preset, int whichModule) const {
		RTEAssert(whichModule < g_ModuleMan.GetTotalModuleCount(), "Tried to access an out of bounds data module number!");

		std::string pRetPath = "";

		auto &loadedModules = g_ModuleMan.GetLoadedDataModules();

		// All modules
		if (whichModule < 0) {
			// Search all modules
			for (int i = 0; i < loadedModules.size() && pRetPath.empty(); ++i) {
				pRetPath = loadedModules[i]->GetEntityDataLocation(type, preset);
			}
		} else {
			// Try to get it from the asked for module
			pRetPath = loadedModules[whichModule]->GetEntityDataLocation(type, preset);

			// If couldn't find it in there, then try all the official modules!
			if (pRetPath.empty()) {
				RTEAssert(g_ModuleMan.GetOfficialModuleCount() <= loadedModules.size(), "More official modules than modules loaded?!");
				for (int i = 0; i < g_ModuleMan.GetOfficialModuleCount() && pRetPath.empty(); ++i) {
					pRetPath = loadedModules[i]->GetEntityDataLocation(type, preset);
				}
			}
		}

		return pRetPath;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool PresetMan::GetAllOfType(std::list<Entity *> &entityList, const std::string &typeName, int whichModule, bool moduleSpace) const {
		RTEAssert(whichModule < g_ModuleMan.GetTotalModuleCount(), "Trying to get from an out of bounds DataModule ID!");

		if (typeName.empty()) {
			return false;
		}

		bool foundAny = false;
		auto &loadedModules = g_ModuleMan.GetLoadedDataModules();

		if (whichModule < 0) {
			for (DataModule *dataModule : loadedModules) {
				foundAny = dataModule->GetAllOfType(entityList, typeName) || foundAny;
			}
		} else {
			if (moduleSpace) {
				for (size_t officialModuleID = 0; officialModuleID < g_ModuleMan.GetOfficialModuleCount(); ++officialModuleID) {
					foundAny = loadedModules[officialModuleID]->GetAllOfType(entityList, typeName) || foundAny;
				}
			}
			foundAny = loadedModules[whichModule]->GetAllOfType(entityList, typeName);
		}
		return foundAny;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool PresetMan::GetAllOfGroups(std::list<Entity *> &entityList, const std::vector<std::string> &groupNames, const std::string &typeName, int whichModule, bool moduleSpace) const {
		RTEAssert(!groupNames.empty(), "Looking for empty groups in PresetMan::GetAllOfGroups!");

		bool foundAny = false;
		auto &loadedModules = g_ModuleMan.GetLoadedDataModules();

		if (whichModule < 0) {
			for (DataModule *dataModule : loadedModules) {
				foundAny = dataModule->GetAllOfGroups(entityList, groupNames, typeName) || foundAny;
			}
		} else {
			RTEAssert(whichModule < loadedModules.size(), "Trying to get from an out of bounds DataModule ID in PresetMan::GetAllOfGroups!");

			if (moduleSpace) {
				for (size_t officialModuleID = 0; officialModuleID < g_ModuleMan.GetOfficialModuleCount(); ++officialModuleID) {
					foundAny = loadedModules[officialModuleID]->GetAllOfGroups(entityList, groupNames, typeName) || foundAny;
				}
			}
			foundAny = loadedModules[whichModule]->GetAllOfGroups(entityList, groupNames, typeName);
		}
		return foundAny;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool PresetMan::GetAllNotOfGroups(std::list<Entity *> &entityList, const std::vector<std::string> &groupNames, const std::string &typeName, int whichModule) const {
		if (groupNames.empty()) {
			RTEAbort("Looking for empty groups in PresetMan::GetAllNotOfGroups!");
		} else if (std::find(groupNames.begin(), groupNames.end(), "All") != groupNames.end()) {
			RTEAbort("Trying to exclude all groups while looking for presets in PresetMan::GetAllNotOfGroups!");
		}

		bool foundAny = false;
		auto &loadedModules = g_ModuleMan.GetLoadedDataModules();

		if (whichModule < 0) {
			for (DataModule *dataModule : loadedModules) {
				foundAny = dataModule->GetAllNotOfGroups(entityList, groupNames, typeName) || foundAny;
			}
		} else {
			RTEAssert(whichModule < loadedModules.size(), "Trying to get from an out of bounds DataModule ID in PresetMan::GetAllNotOfGroups!");
			foundAny = loadedModules[whichModule]->GetAllNotOfGroups(entityList, groupNames, typeName);
		}
		return foundAny;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	Entity * PresetMan::GetRandomOfGroup(const std::string &groupName, const std::string &typeName, int whichModule, bool moduleSpace) {
		RTEAssert(!groupName.empty(), "Looking for empty group!");

		if (std::list<Entity *> entityList; GetAllOfGroups(entityList, { groupName }, typeName, whichModule, moduleSpace)) {
			auto entityItr = entityList.begin();
			std::advance(entityItr, RandomNum<int>(0, entityList.size() - 1));

			return (*entityItr);
		}
		return nullptr;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	Entity * PresetMan::GetRandomBuyableOfGroupFromTech(const std::string &groupName, const std::string &typeName, int whichModule) {
		RTEAssert(!groupName.empty(), "Looking for empty group!");

		bool foundAny = false;
		std::list<Entity *> entityList;
		std::list<Entity *> tempList;

		auto &loadedModules = g_ModuleMan.GetLoadedDataModules();

		if (whichModule < 0) {
			for (DataModule *dataModule : loadedModules) {
				if (dataModule->IsFaction()) {
					foundAny = dataModule->GetAllOfGroups(tempList, { groupName }, typeName) || foundAny;
				}
			}
		} else {
			RTEAssert(whichModule < loadedModules.size(), "Trying to get from an out of bounds DataModule ID!");
			foundAny = loadedModules[whichModule]->GetAllOfGroups(tempList, { groupName }, typeName);
		}

		// Filter found entities, we need only buyables.
		if (foundAny) {
			foundAny = false;

			if (groupName == "Brains") {
				// Do not filter anything if we're looking for brains.
				for (Entity *entity : tempList) {
					entityList.emplace_back(entity);
					foundAny = true;
				}
			} else {
				for (Entity *entity : tempList) {
					const SceneObject *entityAsSceneObject = dynamic_cast<SceneObject *>(entity);
					if (entityAsSceneObject && entityAsSceneObject->IsBuyable() && !entityAsSceneObject->IsBuyableInObjectPickerOnly() && !entityAsSceneObject->IsInGroup("Brains")) {
						entityList.emplace_back(entity);
						foundAny = true;
					}
				}
			}
		}

		if (foundAny) {
			if (whichModule < 0) {
				auto entityItr = entityList.begin();
				std::advance(entityItr, RandomNum<int>(0, entityList.size() - 1));
				return (*entityItr);
			} else {
				// Use random weights if looking in specific modules.
				int totalWeight = 0;
				for (const Entity *entity : entityList) {
					totalWeight += entity->GetRandomWeight();
				}
				if (totalWeight == 0) {
					return nullptr;
				}

				int current = 0;
				int selection = RandomNum(0, totalWeight - 1);
				for (Entity *entity : entityList) {
					bool found = false;

					if (entity->GetRandomWeight() > 0) {
						int bucketCounter = 0;

						while (bucketCounter < entity->GetRandomWeight()) {
							if (current == selection) {
								found = true;
								break;
							}
							current++;
							bucketCounter++;
						}
					}
					if (found) {
						return entity;
					}
				}
			}
		}
		return nullptr;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	Actor * PresetMan::GetLoadout(const std::string &loadoutName, int whichModule, bool spawnDeliveryCraft) {
		if (const Loadout *loadout = dynamic_cast<const Loadout *>(GetEntityPreset("Loadout", loadoutName, whichModule)); loadout) {
			float costTally = 0.0F;

			// Create and pass along the first Actor and his inventory defined in the Loadout.
			Actor *newActor = loadout->CreateFirstActor(whichModule, 1.0F, 1.0F, costTally);

			if (spawnDeliveryCraft) {
				const ACraft *craftPreset = loadout->GetDeliveryCraft();
				if (craftPreset) {
					ACraft *deliveryCraft = dynamic_cast<ACraft *>(craftPreset->Clone());
					if (deliveryCraft) {
						if (newActor) {
							deliveryCraft->AddInventoryItem(newActor);
						}
						return deliveryCraft;
					}
				}
			} else {
				return newActor;
			}
		}
		return nullptr;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	Actor * PresetMan::GetLoadout(const std::string &loadoutName, const std::string &moduleName, bool spawnDeliveryCraft) {
		return GetLoadout(loadoutName, g_ModuleMan.GetModuleID(moduleName), spawnDeliveryCraft);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void PresetMan::ReloadAllScripts() const {
		g_LuaMan.ClearUserModuleCache();
		for (const DataModule *dataModule : g_ModuleMan.GetLoadedDataModules()) {
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
			moduleId = g_ModuleMan.GetModuleID(moduleName);
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
			GetEntityPreset(reader);
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

	bool PresetMan::AddMaterialMapping(uint8_t fromID, uint8_t toID, int whichModule) const {
		//RTEAssert(whichModule >= g_ModuleMan.GetOfficialModuleCount() && whichModule < g_ModuleMan.GetTotalModuleCount(), "Tried to make a material mapping in an official or out-of-bounds DataModule!");

		return g_ModuleMan.GetLoadedDataModules()[whichModule]->AddMaterialMapping(fromID, toID);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool PresetMan::AddEntityPreset(Entity *entityToAdd, int whichModule, bool overwriteSame, const std::string &readFromFile) const {
		RTEAssert(whichModule >= 0 && whichModule < g_ModuleMan.GetTotalModuleCount(), "Tried to access an out of bounds data module number!");

		return g_ModuleMan.GetLoadedDataModules()[whichModule]->AddEntityPreset(entityToAdd, overwriteSame, readFromFile);
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	Entity * PresetMan::ReadReflectedPreset(Reader &reader) {
		int whichModule = reader.GetReadModuleID();
		RTEAssert(whichModule >= 0 && whichModule < g_ModuleMan.GetTotalModuleCount(), "Reader has an out of bounds module number!");

		std::string className;
		const Entity::ClassInfo *classInfo = nullptr;
		Entity *newInstance = nullptr;
		reader >> className;
		classInfo = Entity::ClassInfo::GetClass(className);

		if (classInfo && classInfo->IsConcrete()) {
			// Instantiate
			newInstance = classInfo->NewInstance();

			// Get this before reading entity, since if it's the last one in its datafile, the stream will show the parent file instead.
			std::string entityFilePath = reader.GetCurrentFilePath();

			if (newInstance && newInstance->Create(reader, false) < 0) {
				if (!g_ModuleMan.GetDataModule(whichModule)->GetIgnoreMissingItems()) {
					RTEAbort("Reading of a preset instance \"" + newInstance->GetPresetName() + "\" of class " + newInstance->GetClassName() + " failed in file " + reader.GetCurrentFilePath() + ", shortly before line #" + reader.GetCurrentFileLine());
				}
			} else {
				// Note that we'll return this instance regardless of whether the adding was successful or not.
				g_ModuleMan.GetLoadedDataModules()[whichModule]->AddEntityPreset(newInstance, reader.GetPresetOverwriting(), entityFilePath);
				return newInstance;
			}
		}
		return nullptr;
	}
}