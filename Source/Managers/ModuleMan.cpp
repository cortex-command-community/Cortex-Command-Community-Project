#include "ModuleMan.h"
#include "SettingsMan.h"
#include "ConsoleMan.h"

#include "LoadingScreen.h"
#include "RTETools.h"
#include "System.h"

using namespace RTE;

const std::array<std::string, 10> ModuleMan::c_OfficialModules = {"Base.rte", "Coalition.rte", "Imperatus.rte", "Techion.rte", "Dummy.rte", "Ronin.rte", "Browncoats.rte", "Uzira.rte", "MuIlaak.rte", "Missions.rte"};
const std::array<std::pair<std::string, std::string>, 3> ModuleMan::c_UserdataModules = {{{c_UserScenesModuleName, "User Scenes"},
                                                                                          {c_UserConquestSavesModuleName, "Conquest Saves"},
                                                                                          {c_UserScriptedSavesModuleName, "Scripted Activity Saves"}}};

void ModuleMan::Clear() {
	m_LoadedDataModules.clear();
	m_UnloadedDataModules.clear();
	m_DisabledDataModuleNames.clear();
}

void ModuleMan::Destroy() {
	for (const auto& [moduleID, dataModule]: m_LoadedDataModules) {
		delete dataModule;
	}
	for (const auto& [moduleID, dataModule]: m_UnloadedDataModules) {
		delete dataModule;
	}
	Clear();
}

bool ModuleMan::IsModuleEnabled(const std::string_view& moduleName) const {
	return std::none_of(m_DisabledDataModuleNames.begin(), m_DisabledDataModuleNames.end(),
	                    [&moduleName](const std::string_view& disabledModuleName) {
		                    return disabledModuleName == moduleName;
	                    });
}

bool ModuleMan::IsModuleOfficial(const std::string_view& moduleName) const {
	return std::any_of(c_OfficialModules.begin(), c_OfficialModules.end(),
	                   [&moduleName](const std::string_view& officialModuleName) {
		                   return officialModuleName == moduleName;
	                   });
}

bool ModuleMan::IsModuleUserdata(const std::string_view& moduleName) const {
	return std::any_of(c_UserdataModules.begin(), c_UserdataModules.end(),
	                   [&moduleName](const std::pair<std::string_view, std::string_view>& userdataModuleEntry) {
		                   return userdataModuleEntry.first == moduleName;
	                   });
}

DataModule* ModuleMan::GetDataModule(int whichModule) {
	if (whichModule < 0 || whichModule >= GetTotalModuleCount()) {
		return nullptr;
	}

	auto loadedModulesItr = std::find_if(m_LoadedDataModules.begin(), m_LoadedDataModules.end(),
	                                     [&whichModule](const auto& loadedModuleEntry) {
		                                     return loadedModuleEntry.first == whichModule;
	                                     });
	return (*loadedModulesItr).second;
}

int ModuleMan::GetModuleID(const std::string_view& moduleName) const {
	if (moduleName.empty()) {
		return -1;
	}

	auto loadedModulesItr = std::find_if(m_LoadedDataModules.begin(), m_LoadedDataModules.end(),
	                                     [&moduleName](const auto& loadedModuleEntry) {
		                                     return loadedModuleEntry.second->GetFileName() == moduleName;
	                                     });
	return (loadedModulesItr != m_LoadedDataModules.end()) ? (*loadedModulesItr).first : -1;
}

std::string ModuleMan::GetModuleName(int whichModule) {
	if (const DataModule* dataModule = GetDataModule(whichModule)) {
		return dataModule->GetFileName();
	}
	return "";
}

std::string ModuleMan::GetModuleNameFromPath(const std::string& dataPath) const {
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

std::string ModuleMan::GetFullModulePath(const std::string& modulePath) const {
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

bool ModuleMan::LoadDataModule(const std::string& moduleName, DataModule::DataModuleType moduleType, const ProgressCallback& progressCallback) {
	if (moduleName.empty()) {
		return false;
	}

	// First check that we aren't trying to load another module with the same name. If not, check the unloaded map and load from there if it exists.
	if (const DataModule* dataModule = GetDataModule(GetModuleID(moduleName)); dataModule) {
		return true;
	} else if (moduleType == DataModule::DataModuleType::Unofficial) {
		auto unloadedModulesItr = std::find_if(m_UnloadedDataModules.begin(), m_UnloadedDataModules.end(),
		                                       [&moduleName](const auto& unloadedModuleEntry) {
			                                       return unloadedModuleEntry.second->GetFileName() == moduleName;
		                                       });
		if (unloadedModulesItr != m_UnloadedDataModules.end()) {
			m_LoadedDataModules.emplace(*unloadedModulesItr);
			m_UnloadedDataModules.erase(unloadedModulesItr);
			return true;
		}
	}

	// Only instantiate it here, because it needs to be in the lists of this before being created.
	DataModule* newModule = new DataModule();
	newModule->m_ModuleType = moduleType;
	newModule->m_ModuleID = static_cast<int>(m_LoadedDataModules.size());

	m_LoadedDataModules.try_emplace(newModule->m_ModuleID, newModule);

	if (newModule->Create(moduleName, progressCallback) < 0) {
		RTEAbort("Failed to find the " + moduleName + " Data Module!");
		return false;
	}
	return true;
}

bool ModuleMan::UnloadDataModule(const std::string& moduleName) {
	if (!IsModuleOfficial(moduleName) && !IsModuleUserdata(moduleName)) {
		auto loadedModulesItr = std::find_if(m_LoadedDataModules.begin(), m_LoadedDataModules.end(),
		                                     [&moduleName](const auto& loadedModuleEntry) {
			                                     return loadedModuleEntry.second->GetFileName() == moduleName;
		                                     });

		if (loadedModulesItr != m_LoadedDataModules.end()) {
			m_UnloadedDataModules.emplace(*loadedModulesItr);
			m_LoadedDataModules.erase(loadedModulesItr);
			return true;
		}
	} else {
		g_ConsoleMan.PrintString("Official and Userdata modules cannot be unloaded!");
	}
	return false;
}

bool ModuleMan::LoadAllDataModules() {
	LoadOfficialModules();

	// If a single module is specified, skip loading all other unofficial modules and load specified module only.
	if (!m_SingleModuleToLoad.empty() && !IsModuleOfficial(m_SingleModuleToLoad) && !IsModuleUserdata(m_SingleModuleToLoad)) {
		if (!LoadDataModule(m_SingleModuleToLoad, DataModule::DataModuleType::Unofficial, LoadingScreen::LoadingSplashProgressReport)) {
			g_ConsoleMan.PrintString("ERROR: Failed to load DataModule \"" + m_SingleModuleToLoad + "\"! Only official modules were loaded!");
			g_ConsoleMan.SetEnabled(true);
			return false;
		}
	} else {
		FindAndExtractZippedModules();
		LoadUnofficialModules();

		// Load userdata modules AFTER all other techs etc are loaded; might be referring to stuff in user mods.
		return LoadUserdataModules();
	}
	return true;
}

void ModuleMan::LoadOfficialModules() {
	for (const std::string& officialModule: c_OfficialModules) {
		if (!LoadDataModule(officialModule, DataModule::DataModuleType::Official, LoadingScreen::LoadingSplashProgressReport)) {
			RTEAbort("Failed to load official DataModule \"" + officialModule + "\"!");
		}
	}
}

bool ModuleMan::LoadUserdataModules() {
	for (const auto& [userdataModuleName, userdataModuleFriendlyName]: c_UserdataModules) {
		if (std::string userDataModulePath = System::GetWorkingDirectory() + System::GetUserdataDirectory() + userdataModuleName; !std::filesystem::exists(userDataModulePath)) {
			if (Writer writer(userDataModulePath + "/Index.ini", false, true); writer.WriterOK()) {
				DataModule newUserdataModule;
				newUserdataModule.m_FriendlyName = userdataModuleFriendlyName;
				newUserdataModule.m_ModuleType = DataModule::DataModuleType::Userdata;

				if (userdataModuleName == c_UserScenesModuleName) {
					newUserdataModule.m_IgnoreMissingItems = true;
					newUserdataModule.m_ScanFolderContents = true;
				}
				newUserdataModule.Save(writer);
				writer.EndWrite();
			}
		}
		if (!LoadDataModule(userdataModuleName, DataModule::DataModuleType::Userdata, LoadingScreen::LoadingSplashProgressReport)) {
			RTEAbort("Failed to load userdata DataModule \"" + userdataModuleName + "\"!\n\nThis generally shouldn't happen, but it looks like it did.\nPlease delete this module in the Userdata directory to rebuild it.");
		}
	}
	return true;
}

void ModuleMan::LoadUnofficialModules() {
	std::vector<std::filesystem::directory_entry> modDirectoryFolders;
	const std::string modDirectory = System::GetWorkingDirectory() + System::GetModDirectory();
	std::copy_if(std::filesystem::directory_iterator(modDirectory), std::filesystem::directory_iterator(), std::back_inserter(modDirectoryFolders),
	             [](auto dirEntry) {
		             return std::filesystem::is_directory(dirEntry);
	             });
	std::sort(modDirectoryFolders.begin(), modDirectoryFolders.end());

	for (const std::filesystem::directory_entry& directoryEntry: modDirectoryFolders) {
		std::string directoryEntryPath = directoryEntry.path().generic_string();
		if (directoryEntryPath.ends_with(".rte")) {
			std::string moduleName = directoryEntryPath.substr(directoryEntryPath.find_last_of('/') + 1, std::string::npos);
			if (g_ModuleMan.IsModuleEnabled(moduleName) && !IsModuleOfficial(moduleName) && !IsModuleUserdata(moduleName)) {
				// NOTE: LoadDataModule can return false (especially since it may try to load already loaded modules, which is okay) and shouldn't cause stop, so we can ignore its return value here.
				LoadDataModule(moduleName, DataModule::DataModuleType::Unofficial, LoadingScreen::LoadingSplashProgressReport);
			}
		}
	}
}

void ModuleMan::FindAndExtractZippedModules() const {
	const std::string modDirectory = System::GetWorkingDirectory() + System::GetModDirectory();
	for (const std::filesystem::directory_entry& directoryEntry: std::filesystem::directory_iterator(modDirectory)) {
		std::string zippedModulePath = std::filesystem::path(directoryEntry).generic_string();
		if (zippedModulePath.ends_with(System::GetZippedModulePackageExtension())) {
			LoadingScreen::LoadingSplashProgressReport("Extracting Data Module from: " + directoryEntry.path().filename().generic_string(), true);
			LoadingScreen::LoadingSplashProgressReport(System::ExtractZippedDataModule(zippedModulePath), true);
		}
	}
}
