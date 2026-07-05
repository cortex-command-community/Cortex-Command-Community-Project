// Suppress compiler warning about unrecognized escape sequence on line 183
#pragma warning(disable : 4129)

#include "PresetMan.h"
#include "DataModule.h"
#include "SceneObject.h"
#include "Loadout.h"
#include "ACraft.h"
#include "LuaMan.h"
// #include "AHuman.h"
// #include "MOPixel.h"
// #include "SLTerrain.h"
// #include "AtomGroup.h"
// #include "Atom.h"

#include "ConsoleMan.h"
#include "ThreadMan.h"
#include "LoadingScreen.h"
#include "SettingsMan.h"
#include "System.h"

#include <array>

using namespace RTE;

const std::array<std::string, 10> PresetMan::c_OfficialModules = {"Base.rte", "Coalition.rte", "Imperatus.rte", "Techion.rte", "Dummy.rte", "Ronin.rte", "Browncoats.rte", "Uzira.rte", "MuIlaak.rte", "Missions.rte"};
const std::array<std::pair<std::string, std::string>, 3> PresetMan::c_UserdataModules = {{{c_UserScenesModuleName, "User Scenes"},
                                                                                          {c_UserConquestSavesModuleName, "Conquest Saves"},
                                                                                          {c_UserScriptedSavesModuleName, "Scripted Activity Saves"}}};

PresetMan::PresetMan() {
	Clear();
}

PresetMan::~PresetMan() {
	Destroy();
}

void PresetMan::Clear() {
	m_pDataModules.clear();
	m_DataModuleIDs.clear();
	m_OfficialModuleCount = 0;
	m_TotalGroupRegister.clear();
	m_LastReloadedEntityPresetInfo.fill("");
	m_ReloadEntityPresetCalledThisUpdate = false;
}

/*
int PresetMan::Save(Writer &writer) const
{
    writer << m_Actors.size();
    for (list<Actor *>::const_iterator itr = m_Actors.begin(); itr != m_Actors.end(); ++itr)
        writer << **itr;

    writer << m_Particles.size();
    for (list<MovableEntity *>::const_iterator itr2 = m_Particles.begin(); itr2 != m_Particles.end(); ++itr2)
        writer << **itr2;

    return 0;
}
*/

void PresetMan::Destroy() {
	for (std::vector<DataModule*>::iterator dmItr = m_pDataModules.begin(); dmItr != m_pDataModules.end(); ++dmItr) {
		delete (*dmItr);
	}

	Clear();
}

DataModule* PresetMan::InitDataModule(const std::string& moduleName, bool official, bool userdata) {
	if (moduleName.empty()) {
		RTEAbort("Trying to init a module with an empty module name!");
	}
	// Make a lowercase-version of the module name so it makes it easier to compare to and find case-agnostically.
	std::string lowercaseName = moduleName; //gtodo do this in the other ::transform place too
	std::transform(lowercaseName.begin(), lowercaseName.end(), lowercaseName.begin(), ::tolower);

	// Make sure we don't add the same module twice.
	for (const DataModule* dataModule: m_pDataModules) {
		if (dataModule->GetFileName() == moduleName) {
			RTEAbort("Trying to init module with a name (" + moduleName + ") that's already registered!");
		}
	}

	// Only instantiate it here, because it needs to be in the lists of this before being created.
	DataModule* newModule = new DataModule();

	// Official modules are stacked in the beginning of the vector.
	if (official && !userdata) {
		// Find where the official modules end in the vector.
		std::vector<DataModule*>::iterator moduleItr = m_pDataModules.begin();
		size_t newModuleID = 0;
		for (; newModuleID < m_OfficialModuleCount; ++newModuleID) {
			moduleItr++;
		}
		// Insert into after the last official one.
		m_pDataModules.emplace(moduleItr, newModule);
		m_DataModuleIDs.try_emplace(lowercaseName, newModuleID);
		m_OfficialModuleCount++;
	} else {
		if (userdata) {
			newModule->SetAsUserdata();
		}
		m_pDataModules.emplace_back(newModule);
		m_DataModuleIDs.try_emplace(lowercaseName, m_pDataModules.size() - 1);
	}

	newModule->Init(moduleName); //gtodo, init should be private
	return newModule;
}

bool PresetMan::LoadAllDataModules(std::function<void()> PollSDLEventsCallback) {\
	auto timerTotalFunctionStart = std::chrono::steady_clock::now();
	std::chrono::milliseconds moduleLoadElapsedTime = {};

	// Destroy any possible loaded modules
	Destroy();

	FindAndExtractZippedModules();

	// gtodo
	/* if (!g_SettingsMan.GetLoadingScreenProgressReportDisabled()) {
		//Reader::PushToProgressDisplayQueue = PushToProgressDisplayQueue;
		//DataModule::PushToProgressDisplayQueue = PushToProgressDisplayQueue;
	}*/

	m_GameInitModuleLoadingIsHappening = true;

	// Module loading Thread
	m_GameInitModuleLoadingStatus = GameInitModuleLoadingStatus::StillWorking;
	bool toDoProgressPrintOut = !g_SettingsMan.GetLoadingScreenProgressReportDisabled();
	std::jthread moduleLoadingThread([&](std::stop_token st) {
		ModuleLoadingThreadFunction(st, moduleLoadElapsedTime);
	});		

	// Spinlock watchdog thread
	std::atomic<int> mainThreadHeartbeat = 0;
	std::atomic<bool> spinlockDetected = false;
	std::jthread spinlockWatchdogThread([&](std::stop_token st) {
		SpinlockWatchdogThreadFunction(st, mainThreadHeartbeat, spinlockDetected);
	});
	
	// Main thread - we drain display queue and poll SDL events
	//
	// We do this so the window does not freeze, and to handle
	// the close button on the window being hit / Alt+F4 pressed
	{
		while (1) {
			mainThreadHeartbeat++;
			PollSDLEventsCallback();
			if (true) {
				ProgressDisplayEntry entry;
				{
					std::unique_lock lk(m_ProgressDisplayMutex);

					// Sleep until there is work to do or 16 ms pass
					m_ProgressDisplayCv.wait_for(lk, std::chrono::milliseconds(16), [&] {
						return 
							!m_ProgressDisplayDeque.empty() 
							|| m_GameInitModuleLoadingStatus != GameInitModuleLoadingStatus::StillWorking
							|| m_GameInitModuleLoadingThreadFailed 
							|| spinlockDetected
							|| System::IsSetToQuit();
					});

					if (System::IsSetToQuit() 
						|| m_GameInitModuleLoadingStatus == GameInitModuleLoadingStatus::Failure
						|| m_GameInitModuleLoadingThreadFailed)
					{
						moduleLoadingThread.request_stop();
						break;
					}

					if (m_GameInitModuleLoadingStatus == GameInitModuleLoadingStatus::Success
						&& m_ProgressDisplayDeque.empty()) //gtodo repurpose deque
					{
						break;
					}

					if (!m_ProgressDisplayDeque.empty()) {
						entry = std::move(m_ProgressDisplayDeque.front());
						m_ProgressDisplayDeque.pop_front();
					} else if (spinlockDetected) {
						spinlockDetected = false;
						SpinlockAssert(toDoProgressPrintOut, m_GameInitModuleLoadingStatus);
						RTEAssert(false, to_string(mainThreadHeartbeat));
					}
				}
				LoadingScreen::LoadingSplashProgressReport(entry.first, entry.second);
			}
		}
	}

	spinlockWatchdogThread.request_stop();
	m_SpinlockWdCv.notify_all();
	spinlockWatchdogThread.join();
	m_GameInitModuleLoadingIsHappening = false;

	if (m_GameInitModuleLoadingStatus == GameInitModuleLoadingStatus::Failure) {
		RTEAbort(m_GameInitModuleLoadingErrorMessage);
	}

	moduleLoadingThread.join();

	if (System::IsSetToQuit()) {
		return false;
	}

	// Compile the shaders we've deferred
	for (auto* shader: m_ShadersToCompile) {
		shader->Create();
	}

	if (g_SettingsMan.IsMeasuringModuleLoadTime()) {
		std::chrono::milliseconds totalFunctionElapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - timerTotalFunctionStart);
		std::string coutString = "Total loading time was " + std::to_string(totalFunctionElapsedTime.count()) + "ms";
		if (!g_SettingsMan.GetLoadingScreenProgressReportDisabled()) {
			coutString += " (module load duration: " + std::to_string(moduleLoadElapsedTime.count()) + "ms)";
		}
		g_ConsoleMan.PrintString(coutString);
	}

	return true;
}

const DataModule* PresetMan::GetDataModule(int whichModule) {
	RTEAssert(whichModule >= 0 && whichModule < (int)m_pDataModules.size(), "Tried to access an out of bounds data module number!");
	return m_pDataModules[whichModule];
}

const std::string PresetMan::GetDataModuleName(int whichModule) {
	RTEAssert(whichModule >= 0 && whichModule < (int)m_pDataModules.size(), "Tried to access an out of bounds data module number!");
	return m_pDataModules[whichModule]->GetFileName();
}

int PresetMan::GetModuleID(std::string moduleName) {
	// Lower-case search name so we can match up against the already-lowercase names in m_DataModuleIDs
	std::transform(moduleName.begin(), moduleName.end(), moduleName.begin(), ::tolower);

	// Look for match in m_DataModuleIDs
	std::map<std::string, size_t>::iterator itr = m_DataModuleIDs.find(moduleName);
	if (itr != m_DataModuleIDs.end()) {
		return (*itr).second;
	}

	// Try again! Now with/without .rte at the end
	size_t dotPos = moduleName.find_last_of('.');
	if (dotPos == std::string::npos) {
		moduleName = moduleName + System::GetModulePackageExtension();
	} else {
		moduleName.resize(dotPos);
	}
	
	itr = m_DataModuleIDs.find(moduleName);
	if (itr != m_DataModuleIDs.end()) {
		return (*itr).second;
	}

	// Oop, module wasn't found
	return -1;
}

std::string PresetMan::GetModuleNameFromPath(const std::string& dataPath) const {
	if (dataPath.empty()) {
		return "";
	}
	size_t slashPos = dataPath.find_first_of("/\\");

	// Include trailing slash in the substring range in case we need to match against the Data/Mods/Userdata directory.
	std::string moduleName = (slashPos != std::string::npos) ? dataPath.substr(0, slashPos + 1) : dataPath;

	// Check if path starts with Data/ or the Mods/Userdata dir names and remove that part to get to the actual module name.
	if (moduleName == System::GetDataDirectory() || moduleName == System::GetModDirectory() || moduleName == System::GetUserdataDirectory()) {
		std::string shortenPath = dataPath.substr(slashPos + 1);
		slashPos = shortenPath.find_first_of("/\\");
		moduleName = shortenPath.substr(0, slashPos + 1);
	}

	if (!moduleName.empty() && moduleName.back() == '/') {
		moduleName.pop_back();
	}
	return moduleName;
}

int PresetMan::GetModuleIDFromPath(const std::string& dataPath) {
	if (dataPath.empty()) {
		return -1;
	}
	return GetModuleID(GetModuleNameFromPath(dataPath));
}

bool PresetMan::IsModuleOfficial(const std::string& moduleName) const {
	return std::find(c_OfficialModules.begin(), c_OfficialModules.end(), moduleName) != c_OfficialModules.end();
}

bool PresetMan::IsModuleUserdata(const std::string& moduleName) const {
	auto userdataModuleItr = std::find_if(c_UserdataModules.begin(), c_UserdataModules.end(),
	                                      [&moduleName](const auto& userdataModulesEntry) {
		                                      return userdataModulesEntry.first == moduleName;
	                                      });
	return userdataModuleItr != c_UserdataModules.end();
}

std::string PresetMan::GetFullModulePath(const std::string& modulePath) const {
	// Note: Mods may use mixed path separators, which aren't supported on non Windows systems.
	// Since Windows supports both forward and backslash separators it's safe to replace all backslashes with forward slashes.
	std::string modulePathGeneric = std::filesystem::path(modulePath).generic_string();
	std::replace(modulePathGeneric.begin(), modulePathGeneric.end(), '\\', '/');

	const std::string pathTopDir = modulePathGeneric.substr(0, modulePathGeneric.find_first_of("/") + 1);
	const std::string moduleName = GetModuleNameFromPath(modulePathGeneric);

	std::string moduleTopDir = System::GetModDirectory();

	if (IsModuleOfficial(moduleName)) {
		moduleTopDir = System::GetDataDirectory();
	} else if (IsModuleUserdata(moduleName)) {
		moduleTopDir = System::GetUserdataDirectory();
	}
	return (pathTopDir == moduleTopDir) ? modulePathGeneric : moduleTopDir + modulePathGeneric;
}

bool PresetMan::AddEntityPreset(Entity* pEntToAdd, int whichModule, bool overwriteSame, const std::string& readFromFile) {
	RTEAssert(whichModule >= 0 && whichModule < (int)m_pDataModules.size(), "Tried to access an out of bounds data module number!");

	return m_pDataModules[whichModule]->AddEntityPreset(pEntToAdd, overwriteSame, readFromFile);
}

const Entity* PresetMan::GetEntityPreset(const std::string& type, std::string preset, int whichModule) {
	RTEAssert(whichModule < (int)m_pDataModules.size(), "Tried to access an out of bounds data module number!");

	const Entity* pRetEntity = 0;

	// Preset name might have "[ModuleName]/" preceding it, detect it here and select proper module!
	size_t slashPos = preset.find_first_of('/');
	if (slashPos != std::string::npos) {
		// Get the module ID and cut off the module specifier in the string
		whichModule = GetModuleID(preset.substr(0, slashPos));
		preset = preset.substr(slashPos + 1);
	}

	// All modules
	if (whichModule < 0) {
		// Search all modules
		for (int i = 0; i < m_pDataModules.size() && !pRetEntity; ++i)
			pRetEntity = m_pDataModules[i]->GetEntityPreset(type, preset);
	}
	// Specific module
	else {
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

const Entity* PresetMan::GetEntityPreset(Reader& reader) {
	// The reader is aware of which DataModule it is reading within
	int whichModule = reader.GetReadModuleID();
	RTEAssert(whichModule >= 0 && whichModule < (int)m_pDataModules.size(), "Reader has an out of bounds module number!");

	std::string ClassName;
	const Entity::ClassInfo* pClass = 0;
	Entity* pNewInstance = 0;
	const Entity* pReturnPreset = 0;
	// Load class name and then preset instance
	reader >> ClassName;
	pClass = Entity::ClassInfo::GetClass(ClassName);
	if (ClassName == "Shader") {
		int a = 5;
	}

	if (pClass && pClass->IsConcrete()) {
		// Instantiate
		pNewInstance = pClass->NewInstance();

		// Get this before reading entity, since if it's the last one in its datafile, the stream will show the parent file instead
		std::string entityFilePath = reader.GetCurrentFilePath();

		// Try to read in the preset instance's data from the reader
		if (pNewInstance && pNewInstance->Create(reader, false) < 0) {
			// Abort loading if we can't create entity and it's not in a module that allows ignoring missing items.
			if (!g_PresetMan.GetDataModule(whichModule)->GetIgnoreMissingItems())
				RTEAbort("Reading of a preset instance \"" + pNewInstance->GetPresetName() + "\" of class " + pNewInstance->GetClassName() + " failed in file " + reader.GetCurrentFilePath() + ", shortly before line #" + reader.GetCurrentFileLine());
		} else if (pNewInstance) {
			// Try to add the instance to the collection
			m_pDataModules[whichModule]->AddEntityPreset(pNewInstance, reader.GetPresetOverwriting(), entityFilePath);

			// Regardless of whether there was a collision or not, use whatever now exists in the instance map of that class and name
			pReturnPreset = m_pDataModules[whichModule]->GetEntityPreset(pNewInstance->GetClassName(), pNewInstance->GetPresetName());
			// If the instance wasn't found in the specific DataModule, try to find it in all the official ones instead
			if (!pReturnPreset) {
				RTEAssert(m_OfficialModuleCount <= m_pDataModules.size(), "More official modules than modules loaded?!");
				for (int i = 0; i < m_OfficialModuleCount && !pReturnPreset; ++i)
					pReturnPreset = m_pDataModules[i]->GetEntityPreset(pNewInstance->GetClassName(), pNewInstance->GetPresetName());
			}
		}
		// Get rid of the read-in instance as its copy is now either added to the map, or discarded as there already was somehting in there of the same name.
		delete pNewInstance;
		pNewInstance = 0;
	} else
		pReturnPreset = 0;

	return pReturnPreset;
}

Entity* PresetMan::ReadReflectedPreset(Reader& reader) {
	// The reader is aware of which DataModule it's reading within
	int whichModule = reader.GetReadModuleID();
	RTEAssert(whichModule >= 0 && whichModule < (int)m_pDataModules.size(), "Reader has an out of bounds module number!");

	std::string ClassName;
	const Entity::ClassInfo* pClass = 0;
	Entity* pNewInstance = 0;
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
			if (!g_PresetMan.GetDataModule(whichModule)->GetIgnoreMissingItems())
				RTEAbort("Reading of a preset instance \"" + pNewInstance->GetPresetName() + "\" of class " + pNewInstance->GetClassName() + " failed in file " + reader.GetCurrentFilePath() + ", shortly before line #" + reader.GetCurrentFileLine());
		} else {
			// Try to add the instance to the collection.
			// Note that we'll return this instance regardless of whether the adding was succesful or not
			m_pDataModules[whichModule]->AddEntityPreset(pNewInstance, reader.GetPresetOverwriting(), entityFilePath);
			return pNewInstance;
		}
	}

	return 0;
}

bool PresetMan::GetAllOfType(std::list<Entity*>& entityList, const std::string& type, int whichModule) {
	if (type.empty())
		return false;

	bool foundAny = false;

	// All modules
	if (whichModule < 0) {
		// Send the list to each module
		for (int i = 0; i < m_pDataModules.size(); ++i)
			foundAny = m_pDataModules[i]->GetAllOfType(entityList, type) || foundAny;
	}
	// Specific module
	else {
		RTEAssert(whichModule < (int)m_pDataModules.size(), "Trying to get from an out of bounds DataModule ID!");
		foundAny = m_pDataModules[whichModule]->GetAllOfType(entityList, type);
	}

	return foundAny;
}

bool PresetMan::GetAllOfTypeInModuleSpace(std::list<Entity*>& entityList, const std::string& type, int whichModuleSpace) {
	if (type.empty())
		return false;

	bool foundAny = false;

	// All modules
	if (whichModuleSpace < 0)
		foundAny = GetAllOfType(entityList, type, whichModuleSpace);
	// Specific module space
	else {
		// Get all entitys of the specific type in the official modules loaded before the specified one
		for (int module = 0; module < m_OfficialModuleCount && module < whichModuleSpace; ++module)
			foundAny = GetAllOfType(entityList, type, module) || foundAny;

		// Now get the groups of the specified module (official or not)
		foundAny = GetAllOfType(entityList, type, whichModuleSpace) || foundAny;
	}

	return foundAny;
}

bool PresetMan::GetAllOfGroups(std::list<Entity*>& entityList, const std::vector<std::string>& groups, const std::string& type, int whichModule) {
	RTEAssert(!groups.empty(), "Looking for empty groups in PresetMan::GetAllOfGroups!");
	bool foundAny = false;

	if (whichModule < 0) {
		for (DataModule* dataModule: m_pDataModules) {
			foundAny = dataModule->GetAllOfGroups(entityList, groups, type) || foundAny;
		}
	} else {
		RTEAssert(whichModule < (int)m_pDataModules.size(), "Trying to get from an out of bounds DataModule ID in PresetMan::GetAllOfGroups!");
		foundAny = m_pDataModules[whichModule]->GetAllOfGroups(entityList, groups, type);
	}
	return foundAny;
}

bool PresetMan::GetAllNotOfGroups(std::list<Entity*>& entityList, const std::vector<std::string>& groups, const std::string& type, int whichModule) {
	if (groups.empty()) {
		RTEAbort("Looking for empty groups in PresetMan::GetAllNotOfGroups!");
	} else if (std::find(groups.begin(), groups.end(), "All") != groups.end()) {
		RTEAbort("Trying to exclude all groups while looking for presets in PresetMan::GetAllNotOfGroups!");
	}

	bool foundAny = false;

	if (whichModule < 0) {
		for (DataModule* dataModule: m_pDataModules) {
			foundAny = dataModule->GetAllNotOfGroups(entityList, groups, type) || foundAny;
		}
	} else {
		RTEAssert(whichModule < (int)m_pDataModules.size(), "Trying to get from an out of bounds DataModule ID in PresetMan::GetAllNotOfGroups!");
		foundAny = m_pDataModules[whichModule]->GetAllNotOfGroups(entityList, groups, type);
	}
	return foundAny;
}

Entity* PresetMan::GetRandomOfGroup(std::string group, const std::string& type, int whichModule) {
	RTEAssert(!group.empty(), "Looking for empty group!");

	bool foundAny = false;
	// The total list we'll select a random one from
	std::list<Entity*> entityList;

	// All modules
	if (whichModule < 0) {
		// Get from all modules
		for (int i = 0; i < m_pDataModules.size(); ++i)
			// Send the list to each module, let them add
			foundAny = m_pDataModules[i]->GetAllOfGroups(entityList, {group}, type) || foundAny;
	}
	// Specific one
	else {
		RTEAssert(whichModule < m_pDataModules.size(), "Trying to get from an out of bounds DataModule ID!");
		foundAny = m_pDataModules[whichModule]->GetAllOfGroups(entityList, {group}, type);
	}

	// Didn't find any of that group in those module(s)
	if (!foundAny)
		return 0;

	// Pick one and return it
	int current = 0;
	int selection = RandomNum<int>(0, entityList.size() - 1);
	for (std::list<Entity*>::iterator itr = entityList.begin(); itr != entityList.end(); ++itr) {
		if (current == selection)
			return (*itr);
		current++;
	}

	RTEAssert(0, "Tried selecting randomly but didn't?");
	return 0;
}

Entity* PresetMan::GetRandomBuyableOfGroupFromTech(std::string group, const std::string& type, int whichModule) {
	RTEAssert(!group.empty(), "Looking for empty group!");

	bool foundAny = false;
	// The total list we'll select a random one from
	std::list<Entity*> entityList;
	std::list<Entity*> tempList;

	// All modules
	if (whichModule < 0) {
		for (DataModule* dataModule: m_pDataModules) {
			if (dataModule->IsFaction()) {
				foundAny = dataModule->GetAllOfGroups(tempList, {group}, type) || foundAny;
			}
		}
	} else {
		RTEAssert(whichModule < m_pDataModules.size(), "Trying to get from an out of bounds DataModule ID!");
		foundAny = m_pDataModules[whichModule]->GetAllOfGroups(tempList, {group}, type);
	}

	// Filter found entities, we need only buyables
	if (foundAny) {
		// Do not filter anything if we're looking for brains
		if (group == "Brains") {
			foundAny = false;
			for (std::list<Entity*>::iterator oItr = tempList.begin(); oItr != tempList.end(); ++oItr) {
				entityList.push_back(*oItr);
				foundAny = true;
			}
		} else {
			foundAny = false;
			for (std::list<Entity*>::iterator oItr = tempList.begin(); oItr != tempList.end(); ++oItr) {
				SceneObject* pSObject = dynamic_cast<SceneObject*>(*oItr);
				// Buyable and not brain?
				if (pSObject && pSObject->IsBuyable() && !pSObject->IsBuyableInObjectPickerOnly() && !pSObject->IsInGroup("Brains")) {
					entityList.push_back(*oItr);
					foundAny = true;
				}
			}
		}
	}

	// Didn't find any of that group in those module(s)
	if (!foundAny)
		return 0;

	// Pick one and return it
	int current = 0;
	int selection = RandomNum<int>(0, entityList.size() - 1);

	int totalWeight = 0;
	for (std::list<Entity*>::iterator itr = entityList.begin(); itr != entityList.end(); ++itr)
		totalWeight += (*itr)->GetRandomWeight();

	// Use random weights if looking in specific modules
	if (whichModule >= 0) {
		if (totalWeight == 0)
			return 0;

		selection = RandomNum(0, totalWeight - 1);

		for (std::list<Entity*>::iterator itr = entityList.begin(); itr != entityList.end(); ++itr) {
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

			if (found)
				return (*itr);
		}
	} else {
		for (std::list<Entity*>::iterator itr = entityList.begin(); itr != entityList.end(); ++itr) {
			if (current == selection)
				return (*itr);

			current++;
		}
	}

	RTEAssert(0, "Tried selecting randomly but didn't?");
	return 0;
}

bool PresetMan::GetAllOfGroupInModuleSpace(std::list<Entity*>& entityList, const std::string& group, const std::string& type, int whichModuleSpace) {
	RTEAssert(!group.empty(), "Looking for empty group!");

	bool foundAny = false;

	// All modules
	if (whichModuleSpace < 0)
		foundAny = GetAllOfGroup(entityList, group, type, whichModuleSpace);
	// Specific module space
	else {
		// Get all entitys of the specific group the official modules loaded before the specified one
		for (int module = 0; module < m_OfficialModuleCount && module < whichModuleSpace; ++module)
			foundAny = GetAllOfGroup(entityList, group, type, module) || foundAny;

		// Now get the groups of the specified module (official or not)
		foundAny = GetAllOfGroup(entityList, group, type, whichModuleSpace) || foundAny;
	}

	return foundAny;
}

Entity* PresetMan::GetRandomOfGroupInModuleSpace(const std::string& group, const std::string& type, int whichModuleSpace) {
	RTEAssert(!group.empty(), "Looking for empty group!");

	bool foundAny = false;
	// The total list we'll select a random one from
	std::list<Entity*> entityList;

	// All modules
	if (whichModuleSpace < 0)
		foundAny = GetAllOfGroup(entityList, group, type, whichModuleSpace);
	// Specific module space
	else {
		// Get all entitys of the specific group the official modules loaded before the specified one
		for (int module = 0; module < m_OfficialModuleCount && module < whichModuleSpace; ++module)
			foundAny = GetAllOfGroup(entityList, group, type, module) || foundAny;

		// Now get the groups of the specified module (official or not)
		foundAny = GetAllOfGroup(entityList, group, type, whichModuleSpace) || foundAny;
	}

	// Didn't find any of that group in those module(s)
	if (!foundAny)
		return 0;

	// Pick one and return it
	int current = 0;
	int selection = RandomNum<int>(0, entityList.size() - 1);
	for (std::list<Entity*>::iterator itr = entityList.begin(); itr != entityList.end(); ++itr) {
		if (current == selection)
			return (*itr);
		current++;
	}

	RTEAssert(0, "Tried selecting randomly but didn't?");
	return 0;
}

std::string PresetMan::GetEntityDataLocation(const std::string& type, const std::string& preset, int whichModule) {
	RTEAssert(whichModule < (int)m_pDataModules.size(), "Tried to access an out of bounds data module number!");

	std::string pRetPath = "";

	// All modules
	if (whichModule < 0) {
		// Search all modules
		for (int i = 0; i < m_pDataModules.size() && pRetPath.empty(); ++i)
			pRetPath = m_pDataModules[i]->GetEntityDataLocation(type, preset);
	}
	// Specific module
	else {
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

void PresetMan::ReloadAllScripts() const {
	g_LuaMan.ClearUserModuleCache();
	for (const DataModule* dataModule: m_pDataModules) {
		dataModule->ReloadAllScripts();
	}
	g_MovableMan.ReloadLuaScripts();
	g_ConsoleMan.PrintString("SYSTEM: Scripts reloaded!");
}

bool PresetMan::ReloadEntityPreset(const std::string& presetName, const std::string& className, const std::string& moduleName, bool storeReloadedPresetDataForQuickReloading) {
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

	Reader reader(presetDataLocation, true);
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

bool PresetMan::QuickReloadEntityPreset() {
	for (const std::string& entityPresetInfoEntry: m_LastReloadedEntityPresetInfo) {
		if (entityPresetInfoEntry.empty()) {
			g_ConsoleMan.PrintString("ERROR: Trying to quick reload Entity preset when there is nothing set to reload!");
			return false;
		}
	}
	return ReloadEntityPreset(m_LastReloadedEntityPresetInfo[0], m_LastReloadedEntityPresetInfo[1], m_LastReloadedEntityPresetInfo[2]);
}

bool PresetMan::AddMaterialMapping(int fromID, int toID, int whichModule) {
	RTEAssert(whichModule >= m_OfficialModuleCount && whichModule < m_pDataModules.size(), "Tried to make a material mapping in an offical or out-of-bounds DataModule!");

	return m_pDataModules[whichModule]->AddMaterialMapping(fromID, toID);
}

void PresetMan::RegisterGroup(std::string newGroup, int whichModule) {
	RTEAssert(whichModule >= 0 && whichModule < (int)m_pDataModules.size(), "Tried to access an out of bounds data module number!");

	// Register in the handy total list
	m_TotalGroupRegister.push_back(newGroup);
	m_TotalGroupRegister.sort();
	m_TotalGroupRegister.unique();

	// Register in the specified module too
	m_pDataModules[whichModule]->RegisterGroup(newGroup);
}

bool PresetMan::GetGroups(std::list<std::string>& groupList, int whichModule, const std::string& withType) const {
	RTEAssert(whichModule < (int)m_pDataModules.size(), "Tried to access an out of bounds data module number!");

	bool foundAny = false;

	// Asked for ALL groups ever registered
	if (whichModule < 0) {
		// Get all applicable groups
		if (withType == "All" || withType.empty()) {
			for (std::list<std::string>::const_iterator gItr = m_TotalGroupRegister.begin(); gItr != m_TotalGroupRegister.end(); ++gItr)
				groupList.push_back(*gItr);

			foundAny = !m_TotalGroupRegister.empty();
		}
		// Filter out groups without any entitys of the specified type
		else {
			for (int module = 0; module < (int)m_pDataModules.size(); ++module)
				foundAny = m_pDataModules[module]->GetGroupsWithType(groupList, withType) || foundAny;
		}
	}
	// Asked for specific DataModule's groups
	else if (!m_pDataModules[whichModule]->GetGroupRegister()->empty()) {
		// Get ALL groups of that module
		if (withType == "All" || withType.empty()) {
			const std::list<std::string>* pGroupList = m_pDataModules[whichModule]->GetGroupRegister();
			for (std::list<std::string>::const_iterator gItr = pGroupList->begin(); gItr != pGroupList->end(); ++gItr)
				groupList.push_back(*gItr);

			foundAny = !pGroupList->empty();
		}
		// Get only modules that contain an entity of valid type
		else
			foundAny = m_pDataModules[whichModule]->GetGroupsWithType(groupList, withType) || foundAny;
	}

	return foundAny;
}

bool PresetMan::GetModuleSpaceGroups(std::list<std::string>& groupList, int whichModule, const std::string& withType) const {
	RTEAssert(whichModule < (int)m_pDataModules.size(), "Tried to access an out of bounds data module number!");

	bool foundAny = false;

	// If all, then just copy the total register
	if (whichModule < 0) {
		// Just get all groups ever registered
		if (withType == "All" || withType.empty()) {
			for (std::list<std::string>::const_iterator gItr = m_TotalGroupRegister.begin(); gItr != m_TotalGroupRegister.end(); ++gItr)
				groupList.push_back(*gItr);

			foundAny = !m_TotalGroupRegister.empty();
		}
		// Get type filtered groups from ALL data modules
		else {
			for (int module = 0; module < (int)m_pDataModules.size(); ++module)
				foundAny = GetGroups(groupList, module, withType) || foundAny;
		}
	}
	// Getting modulespace of specific module
	else {
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

Actor* PresetMan::GetLoadout(std::string loadoutName, std::string module, bool spawnDropShip) {
	return GetLoadout(loadoutName, GetModuleID(module), spawnDropShip);
}

Actor* PresetMan::GetLoadout(std::string loadoutName, int moduleNumber, bool spawnDropShip) {
	if (spawnDropShip) {
		// Find the Loadout that this Deployment is referring to
		const Loadout* pLoadout = dynamic_cast<const Loadout*>(g_PresetMan.GetEntityPreset("Loadout", loadoutName, moduleNumber));
		if (pLoadout) {
			const ACraft* pCraftPreset = pLoadout->GetDeliveryCraft();
			if (pCraftPreset) {
				ACraft* pCraft = dynamic_cast<ACraft*>(pCraftPreset->Clone());
				if (pCraft) {
					float tally = 0;
					// Create and pass along the first Actor and his inventory defined in the Loadout
					Actor* pActor = pLoadout->CreateFirstActor(moduleNumber, 1, 1, tally);
					// Set the position and team etc for the Actor we are prepping to spawn
					if (pActor)
						pCraft->AddInventoryItem(pActor);
				}
				return pCraft;
			}
		}
	} else {
		// Find the Loadout that this Deployment is referring to
		const Loadout* pLoadout = dynamic_cast<const Loadout*>(g_PresetMan.GetEntityPreset("Loadout", loadoutName, moduleNumber));
		if (pLoadout) {
			float tally = 0;
			// Create and pass along the first Actor and his inventory defined in the Loadout
			Actor* pReturnActor = pLoadout->CreateFirstActor(moduleNumber, 1, 1, tally);
			// Set the position and team etc for the Actor we are prepping to spawn
			return pReturnActor;
		}
	}

	return 0;
}

void RTE::PresetMan::MLTF_WorkerFunction(std::stop_token st) {
	while (!st.stop_requested()) {
		// Try to acquire a module to finalize
		DataModule* moduleToFinalize = nullptr;
		{
			std::lock_guard lg(m_MLTFMutex);
			MLTFWorkerStruct::Status& status = m_MLTFWorkerStruct.status;

			// Take first available base modules until all are finalized
			if (status == MLTFWorkerStruct::Status::FinalizingBaseModules) {
				bool allBaseModulesFinalized = true;
				for (auto& possibleModule: m_MLTFWorkerStruct.BaseGameModulesToFinalize) {
					if (!possibleModule.IsTaken) {
						moduleToFinalize = possibleModule.Module;
						possibleModule.IsTaken = true;
						goto found_a_module;
					} else {
						if (possibleModule.Module->GetCreationStatus() 
							!= DataModule::CreationStatus::FINALIZED) 
						{
							allBaseModulesFinalized = false;
						}
					}
				}
				if (allBaseModulesFinalized) {
					status = MLTFWorkerStruct::Status::AllBaseModulesFinalized;
				} else {
					using namespace std::chrono_literals;
					std::this_thread::sleep_for(2ms);
					continue;
				}
			}
			if (status == MLTFWorkerStruct::Status::AllBaseModulesFinalized) {
				return;
			}
			if (status == MLTFWorkerStruct::Status::EverythingDone) {
			}
			
		}
	found_a_module:
		moduleToFinalize->Finalize();
	}
}

void PresetMan::FindAndExtractZippedModules() const {
	for (const std::filesystem::directory_entry& directoryEntry: std::filesystem::directory_iterator(System::GetWorkingDirectory() + System::GetModDirectory())) {
		std::string zippedModulePath = std::filesystem::path(directoryEntry).generic_string();
		if (zippedModulePath.ends_with(System::GetZippedModulePackageExtension())) {
			LoadingScreen::LoadingSplashProgressReport("Extracting Data Module from: " + directoryEntry.path().filename().generic_string(), true);
			LoadingScreen::LoadingSplashProgressReport(System::ExtractZippedDataModule(zippedModulePath), true);
		}
	}
}

void PresetMan::DeferShaderCompilationToBeDoneOnMainThread(Shader* shader) {
	m_ShadersToCompile.push_back(shader);
}

void PresetMan::PushToProgressDisplayQueue(const std::string& string, bool newItem) {
	{
		std::lock_guard lg(g_PresetMan.m_ProgressDisplayMutex);
		g_PresetMan.m_ProgressDisplayDeque.push_back({string, newItem});
	}
	g_PresetMan.m_ProgressDisplayCv.notify_one();
}

//gtodo should this be spinlockabort?
void PresetMan::SpinlockAssert(bool toDoProgressPrintOut, GameInitModuleLoadingStatus loadingDone) {

	std::string loadingDoneValueString;
	switch (loadingDone) {
	case GameInitModuleLoadingStatus::Failure:
		loadingDoneValueString = "Failure";
		break;
	case GameInitModuleLoadingStatus::Success:
		loadingDoneValueString = "Success";
		break;
	case GameInitModuleLoadingStatus::StillWorking:
		loadingDoneValueString = "StillWorking";
		break;
	default:
		loadingDoneValueString = "Unknown";
		break;
	}
	//gtodo rename these
	const std::string assertString =
		"Main thread spinlock during module loading!\n"
		+ std::string("toDoProgressPrintOut = ") + std::to_string(toDoProgressPrintOut)
		+ std::string("\nm_GameInitModuleLoadingStatus = ") + loadingDoneValueString
		+ std::string("\nm_GameInitModuleLoadingThreadFailed = ") + std::to_string(m_GameInitModuleLoadingThreadFailed)
		+ std::string("\nm_ProgressDisplayDeque.size() = ") + std::to_string(m_ProgressDisplayDeque.size());
	RTEAssert(false, assertString);
}

void PresetMan::GameInitModuleLoadingAbort(const std::string& description, std::source_location srcLocation) {
	if (g_ThreadMan.IsMainThread()) {
		RTEAbort("PresetMan::ModuleLoadingThreadAbort called from the main thread! What!");
	}
	{
		std::lock_guard lg(m_GameInitModuleLoadingErrorMutex);
		if (!m_GameInitModuleLoadingIsHappening) {
			RTEError::AbortFunc("RTEAbort called from non-main thread NOT during game launch module loading: " + description, std::source_location::current());
		}
		if (!m_GameInitModuleLoadingThreadFailed) {
			m_GameInitModuleLoadingStatus = GameInitModuleLoadingStatus::Failure;
			m_GameInitModuleLoadingThreadFailed = true;
			m_ToStopSpinlockWatchdog = true;

			std::filesystem::path filePath = srcLocation.file_name();
			std::string fileName =
			    (filePath.has_root_name() || filePath.has_root_directory())
			        ? filePath.filename().generic_string()
			        : srcLocation.file_name();
			const std::string lineNum = std::to_string(srcLocation.line());
			const std::string funcName = srcLocation.function_name();

			m_GameInitModuleLoadingErrorMessage =
			    "RTEAbort called from one of the module loading threads, file '" 
				+ fileName + "', line " + lineNum + ",\nfunction '" 
				+ funcName + "':\n" + description;
			m_ProgressDisplayCv.notify_all();
		}
		while (true) {
			std::this_thread::yield();
		}
	}
}

void PresetMan::ModuleLoadingThreadFunction(std::stop_token st, std::chrono::milliseconds& moduleLoadElapsedTime) {
	if (g_ThreadMan.IsMainThread()) {
		RTEAbort("PresetMan::ModuleLoadingThreadFunction called from the main thread! Bad!");
	}
	auto timerModuleLoadingThreadStart = std::chrono::steady_clock::now();

	// Load Base.rte first!
	InitDataModule("Base.rte", true, false)->Finalize();

	// Init all the other official modules
	std::vector<DataModule*> BaseGameModulesToFinalize;
	for (auto officialModuleIt = c_OfficialModules.begin() + 1; 
		officialModuleIt != c_OfficialModules.end(); 
		++officialModuleIt) 
	{
		m_MLTFWorkerStruct.BaseGameModulesToFinalize
			.push_back(InitDataModule(*officialModuleIt, true, false));
	}
	m_MLTFWorkerStruct.PrecalculateModuleDependencyIndexesForMods();
	// And then finalize them
	// Dispatch worker threads
	const int moduleFinalizingWorkerThreadCount = 1;
	for (int i = 0; i < moduleFinalizingWorkerThreadCount; ++i) {
		m_MLTFThreads.emplace_back(
			std::thread([this](std::stop_token st) {
				MLTF_WorkerFunction(st);
			}, st)
		);
	}
	// Join worker threads
	for (int i = 0; i < moduleFinalizingWorkerThreadCount; ++i) {
		m_MLTFThreads[i].join();
	}
	RTEAbort("ass");

	// Load mod modules
	// If a single module is specified, skip loading all other unofficial modules and load specified module only.
	if (!m_SingleModuleToLoad.empty() && !IsModuleOfficial(m_SingleModuleToLoad)) {
		InitDataModule(m_SingleModuleToLoad, false, false)->Finalize();
	} else {
		// Gather mod folder names
		std::vector<std::string> modModuleNames;
		const std::string modDirectory = System::GetWorkingDirectory() + System::GetModDirectory();
		for (auto const& dirEntry: std::filesystem::directory_iterator{modDirectory}) {
			if (!std::filesystem::is_directory(dirEntry)) {
				continue;
			}
			const std::string dirEntryStr = dirEntry.path().generic_string();
			if (!dirEntryStr.ends_with(".rte")) {
				continue;
			}
			std::string moduleName = dirEntryStr.substr(dirEntryStr.find_last_of('/') + 1, std::string::npos);
			
			if (!g_SettingsMan.IsModDisabled(moduleName) 
				&& !IsModuleOfficial(moduleName) 
				&& !IsModuleUserdata(moduleName)) 
			{
				modModuleNames.push_back(moduleName);
			}
		}
		std::sort(modModuleNames.begin(), modModuleNames.end());

		// Now go over mod folders
		for (auto const& modModuleName: modModuleNames) {
			if (st.stop_requested()) {
				return;
			}
			InitDataModule(modModuleName, false, false)->Finalize();
		}

		// Load userdata modules AFTER all other techs etc are loaded; might be referring to stuff in user mods.
		for (const auto& [userdataModuleName, userdataModuleFriendlyName]: c_UserdataModules) {
			if (st.stop_requested()) {
				return;
			}
			if (!std::filesystem::exists(System::GetWorkingDirectory() + System::GetUserdataDirectory() + userdataModuleName)) {
				bool scanContentsAndIgnoreMissing = userdataModuleName == c_UserScenesModuleName;
				DataModule::CreateOnDiskAsUserdata(userdataModuleName, userdataModuleFriendlyName, scanContentsAndIgnoreMissing, scanContentsAndIgnoreMissing);
			}
			InitDataModule(userdataModuleName, false, true)->Finalize();
		}
	}

	moduleLoadElapsedTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - timerModuleLoadingThreadStart);

	m_GameInitModuleLoadingStatus = GameInitModuleLoadingStatus::Success;
	m_ProgressDisplayCv.notify_all();
	return;
}

void PresetMan::SpinlockWatchdogThreadFunction(std::stop_token st, std::atomic<int>& mainThreadHeartbeat, std::atomic<bool>& spinlockDetected) {
	int previousHeartbeat = 0;
	while (!st.stop_requested()) {
		if (m_ToStopSpinlockWatchdog) {
			return;
		}

		std::unique_lock lk(m_SpinlockWdMutex);
		m_SpinlockWdCv.wait_for(lk, std::chrono::milliseconds(1000), [&] {
			return st.stop_requested();
		});
		if (st.stop_requested()) {
			return;
		}

		int newHeartbeat = mainThreadHeartbeat;
		if (newHeartbeat == previousHeartbeat) {
			// Spinlock, report!
			spinlockDetected = true;
			m_ProgressDisplayCv.notify_all();
			return;
		}
		previousHeartbeat = newHeartbeat;
	}
}

void PresetMan::MLTFWorkerStruct::PrecalculateModuleDependencyIndexesForMods() {
	if (m_DependenciesPrecalced) {
		RTEAbort("PrecalculateModuleDependencyIndexesForMods() is called twice, shouldn't be.");
	}
	// For each of the mod modules to finalize
	for (auto& modModule: ModModulesToFinalize) {
		const std::vector<std::string>& requiredModuleNames = modModule.Module->GetRequiredModules();
		// We go through their required modules
		for (auto& requiredModuleName: requiredModuleNames) {
			const int requiredModuleID =
			    g_PresetMan.GetModuleID(requiredModuleName);
			if (requiredModuleID == -1) {
				RTEAbort("Mod module '" + modModule.Module->GetFileName() + "' sets a requirement for a missing module '" + requiredModuleName + "' in index.ini!");
			}
			if (requiredModuleID < g_PresetMan.GetOfficialModuleCount()) {
				RTEAbort("Mod module '" + modModule.Module->GetFileName() + "' sets a requirement for a base game module '" + requiredModuleName + "' in index.ini, shouldn't!");
			}
			if (requiredModuleID == modModule.Module->GetModuleID()) {
				RTEAbort("Mod module '" + modModule.Module->GetFileName() + "' requires itself in index.ini, what? '");
			}
			// And if valid, we put them in a vector
			modModule.RequiredModules.push_back(g_PresetMan.GetDataModule(requiredModuleID));
		}
	}
	m_DependenciesPrecalced = true;
}

PresetMan::MLTFWorkerStructModule::MLTFWorkerStructModule(DataModule* module) {
	if (module->GetCreationStatus() != DataModule::CreationStatus::INITIALIZED_NOT_FINALIZED) {
		RTEAbort("Trying to construct MLTFWorkerStructModule with a non-initialized or already finalized module!");
	}
	Module = module;
}