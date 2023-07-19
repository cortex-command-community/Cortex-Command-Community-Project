// Suppress compiler warning about unrecognized escape sequence in LoadAllDataModules.
#pragma warning( disable : 4129 )

#include "ModuleMan.h"
#include "SettingsMan.h"
#include "ConsoleMan.h"

#include "LoadingScreen.h"
#include "RTETools.h"

namespace RTE {

	const std::string ModuleMan::c_ClassName = "ModuleMan";

	const std::array<std::string, 10> ModuleMan::c_OfficialModules = { "Base.rte", "Coalition.rte", "Imperatus.rte", "Techion.rte", "Dummy.rte", "Ronin.rte", "Browncoats.rte", "Uzira.rte", "MuIlaak.rte", "Missions.rte" };
	const std::array<std::pair<std::string, std::string>, 3> ModuleMan::c_UserdataModules = {{
		{c_UserScenesModuleName, "User Scenes"},
		{c_UserConquestSavesModuleName, "Conquest Saves"},
		{c_UserScriptedSavesModuleName, "Scripted Activity Saves"}
	}};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ModuleMan::Clear() {
		m_LoadedDataModules.clear();
		m_DisabledMods.clear();
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ModuleMan::Destroy() {
		for (const DataModule *dataModule : m_LoadedDataModules) {
			delete dataModule;
		}
		Clear();
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool ModuleMan::IsModDisabled(const std::string &modModule) const {
		return (m_DisabledMods.find(modModule) != m_DisabledMods.end()) ? m_DisabledMods.at(modModule) : false;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool ModuleMan::IsModuleOfficial(const std::string &moduleName) const {
		for (const std::string officialModuleName : c_OfficialModules) {
			// Module name coming from Lua might be with different casing.
			if (StringsEqualCaseInsensitive(officialModuleName, moduleName)) {
				return true;
			}
		}
		return false;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool ModuleMan::IsModuleUserdata(const std::string &moduleName) const {
		auto userdataModuleItr = std::find_if(c_UserdataModules.begin(), c_UserdataModules.end(),
			[&moduleName](const auto &userdataModulesEntry) {
				return userdataModulesEntry.first == moduleName;
			}
		);
		return userdataModuleItr != c_UserdataModules.end();
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	const DataModule * ModuleMan::GetDataModule(int whichModule) {
		RTEAssert(whichModule < (int)m_LoadedDataModules.size(), "Tried to access an out of bounds data module number!");
		return m_LoadedDataModules[whichModule];
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	const std::string ModuleMan::GetModuleName(int whichModule) {
		RTEAssert(whichModule < (int)m_LoadedDataModules.size(), "Tried to access an out of bounds data module number!");
		return m_LoadedDataModules[whichModule]->GetFileName();
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int ModuleMan::GetModuleID(std::string moduleName) {
		// Lower-case search name so we can match up against the already-lowercase names in m_DataModuleIDs
		std::transform(moduleName.begin(), moduleName.end(), moduleName.begin(), ::tolower);

		// First pass
		std::map<std::string, size_t>::iterator itr = m_DataModuleIDs.find(moduleName);
		if (itr != m_DataModuleIDs.end()) {
			return (*itr).second;
		}

		// Try with or without the .rte on the end before giving up
		int dotPos = moduleName.find_last_of('.');
		// Wasnt, so try adding it
		if (dotPos == std::string::npos) {
			moduleName = moduleName + System::GetModulePackageExtension();
			// There was ".rte", so try to shave it off the name
		} else {
			moduleName = moduleName.substr(0, dotPos);
		}

		// Try to find the module again!
		itr = m_DataModuleIDs.find(moduleName);
		if (itr != m_DataModuleIDs.end()) {
			return (*itr).second;
		}
		return -1;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	std::string ModuleMan::GetModuleNameFromPath(const std::string &dataPath) const {
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

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	int ModuleMan::GetModuleIDFromPath(const std::string &dataPath) {
		if (dataPath.empty()) {
			return -1;
		}
		return GetModuleID(GetModuleNameFromPath(dataPath));
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	std::string ModuleMan::GetFullModulePath(const std::string &modulePath) const {
		const std::string modulePathGeneric = std::filesystem::path(modulePath).generic_string();
		const std::string pathTopDir = modulePathGeneric.substr(0, modulePathGeneric.find_first_of("/\\") + 1);
		const std::string moduleName = GetModuleNameFromPath(modulePathGeneric);

		std::string moduleTopDir = System::GetModDirectory();

		if (IsModuleOfficial(moduleName)) {
			moduleTopDir = System::GetDataDirectory();
		} else if (IsModuleUserdata(moduleName)) {
			moduleTopDir = System::GetUserdataDirectory();
		}
		return (pathTopDir == moduleTopDir) ? modulePathGeneric : moduleTopDir + modulePathGeneric;
	}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool ModuleMan::LoadDataModule(const std::string &moduleName, DataModule::DataModuleType moduleType, const ProgressCallback &progressCallback) {
		if (moduleName.empty()) {
			return false;
		}
		// Make a lowercase-version of the module name so it makes it easier to compare to and find case-agnostically.
		std::string lowercaseName = moduleName;
		std::transform(lowercaseName.begin(), lowercaseName.end(), lowercaseName.begin(), ::tolower);

		// Make sure we don't add the same module twice.
		for (const DataModule *dataModule : m_LoadedDataModules) {
			if (dataModule->GetFileName() == moduleName) {
				return false;
			}
		}

		// Only instantiate it here, because it needs to be in the lists of this before being created.
		DataModule *newModule = new DataModule();

		// Official modules are stacked in the beginning of the vector.
		if (official && !userdata) {
			// Halt if an official module is being loaded after any non-official ones!
			//RTEAssert(m_LoadedDataModules.size() == m_OfficialModuleCount, "Trying to load an official module after a non-official one has been loaded!");

			// Find where the official modules end in the vector.
			std::vector<DataModule *>::iterator moduleItr = m_LoadedDataModules.begin();
			size_t newModuleID = 0;
			for (; newModuleID < m_OfficialModuleCount; ++newModuleID) {
				moduleItr++;
			}
			// Insert into after the last official one.
			m_LoadedDataModules.emplace(moduleItr, newModule);
			m_DataModuleIDs.try_emplace(lowercaseName, newModuleID);
			m_OfficialModuleCount++;
		} else {
			if (userdata) {
				newModule->SetAsUserdata();
			}
			m_LoadedDataModules.emplace_back(newModule);
			m_DataModuleIDs.try_emplace(lowercaseName, m_LoadedDataModules.size() - 1);
		}

		if (newModule->Create(moduleName, progressCallback) < 0) {
			RTEAbort("Failed to find the " + moduleName + " Data Module!");
			return false;
		}
		newModule = nullptr;
		return true;
	}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ModuleMan::LoadOfficialModules() {
		for (const std::string &officialModule : c_OfficialModules) {
			if (!LoadDataModule(officialModule, DataModule::DataModuleType::Official, LoadingScreen::LoadingSplashProgressReport)) {
				RTEAbort("Failed to load official DataModule \"" + officialModule + "\"!");
			}
		}
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool ModuleMan::LoadAllDataModules() {
		// Destroy any possible loaded modules.
		Destroy();

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

			std::vector<std::filesystem::directory_entry> modDirectoryFolders;
			const std::string modDirectory = System::GetWorkingDirectory() + System::GetModDirectory();
			std::copy_if(std::filesystem::directory_iterator(modDirectory), std::filesystem::directory_iterator(), std::back_inserter(modDirectoryFolders),
				[](auto dirEntry) {
					return std::filesystem::is_directory(dirEntry);
				}
			);
			std::sort(modDirectoryFolders.begin(), modDirectoryFolders.end());

			for (const std::filesystem::directory_entry &directoryEntry : modDirectoryFolders) {
				std::string directoryEntryPath = directoryEntry.path().generic_string();
				if (std::regex_match(directoryEntryPath, std::regex(".*\.rte"))) {
					std::string moduleName = directoryEntryPath.substr(directoryEntryPath.find_last_of('/') + 1, std::string::npos);
					if (!g_ModuleMan.IsModDisabled(moduleName) && !IsModuleOfficial(moduleName) && !IsModuleUserdata(moduleName)) {
						// NOTE: LoadDataModule can return false (especially since it may try to load already loaded modules, which is okay) and shouldn't cause stop, so we can ignore its return value here.
						LoadDataModule(moduleName, DataModule::DataModuleType::Unofficial, LoadingScreen::LoadingSplashProgressReport);
					}
				}
			}

			// Load userdata modules AFTER all other techs etc are loaded; might be referring to stuff in user mods.
			return LoadUserdataModules();
		}
		return true;
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	bool ModuleMan::LoadUserdataModules() {
		for (const auto &[userdataModuleName, userdataModuleFriendlyName] : c_UserdataModules) {
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



/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void ModuleMan::FindAndExtractZippedModules() const {
		for (const std::filesystem::directory_entry &directoryEntry : std::filesystem::directory_iterator(System::GetWorkingDirectory() + System::GetModDirectory())) {
			std::string zippedModulePath = std::filesystem::path(directoryEntry).generic_string();
			if (zippedModulePath.ends_with(System::GetZippedModulePackageExtension())) {
				LoadingScreen::LoadingSplashProgressReport("Extracting Data Module from: " + directoryEntry.path().filename().generic_string(), true);
				LoadingScreen::LoadingSplashProgressReport(System::ExtractZippedDataModule(zippedModulePath), true);
			}
		}
	}
}