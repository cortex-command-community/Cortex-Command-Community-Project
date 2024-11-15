#ifndef _RTEMODULEMAN_
#define _RTEMODULEMAN_

#include "Singleton.h"
#include "DataModule.h"

#include <unordered_set>

#define g_ModuleMan ModuleMan::Instance()

namespace RTE {

	/// Manager responsible for loading and unloading of DataModules.
	class ModuleMan : public Singleton<ModuleMan> {
		friend class SettingsMan;
		friend struct ManagerLuaBindings;

	public:
#pragma region Creation
		/// Constructor method used to instantiate a ModuleMan object in system memory. Initialize() should be called before using the object.
		ModuleMan() { Clear(); }
#pragma endregion

#pragma region Destruction
		/// Destroys and resets (through Clear()) the ModuleMan object.
		void Destroy();
#pragma endregion

#pragma region Getters and Setters
		/// Sets the single module to be loaded after the official modules. This will be the ONLY non-official module to be loaded.
		/// @param moduleName Name of the module to load.
		void SetSingleModuleToLoad(const std::string_view& moduleName) { m_SingleModuleToLoad = moduleName; }

		/// Gets the total number of loaded DataModules.
		/// @return The total number of loaded DataModules.
		int GetTotalModuleCount() const { return static_cast<int>(m_LoadedDataModules.size()); }

		/// Gets whether the specified mod is disabled in the settings.
		/// @param moduleName Mod to check.
		/// @return Whether the module is disabled via settings.
		bool IsModuleEnabled(const std::string_view& moduleName) const;

		/// Gets whether a module name is of an official DataModule.
		/// @param moduleName The name of the module to check, in the form "moduleName.rte"
		/// @return True if the module is an official DataModule, otherwise false.
		bool IsModuleOfficial(const std::string_view& moduleName) const;

		/// Gets whether a module name is of a userdata DataModule.
		/// @param moduleName The name of the module to check, in the form "moduleName.rte"
		/// @return True if the module is a userdata DataModule, otherwise false.
		bool IsModuleUserdata(const std::string_view& moduleName) const;
#pragma endregion

#pragma region DataModule Getters
		/// Gets all the loaded DataModules.
		/// @return Reference to the loaded DataModules map.
		std::unordered_map<int, DataModule*>& GetLoadedDataModules() { return m_LoadedDataModules; }

		/// Gets a specific loaded DataModule.
		/// @param whichModule The ID of the module to get.
		/// @return The requested DataModule. Ownership is NOT transferred!
		DataModule* GetDataModule(int whichModule = 0);

		/// Gets a specific loaded DataModule.
		/// @param whichModule The name of the DataModule to get, including the ".rte".
		/// @return The requested DataModule. Ownership is NOT transferred!
		DataModule* GetDataModule(const std::string_view& moduleName) { return GetDataModule(GetModuleID(moduleName)); }
#pragma endregion

#pragma region DataModule Info Getters
		/// Gets the DataModule names that are disabled and should not be loaded at startup.
		/// @return Map of mods which are disabled.
		std::unordered_set<std::string>& GetDisabledDataModuleNames() { return m_DisabledDataModuleNames; }

		/// Gets the ID of a loaded DataModule.
		/// @param moduleName The name of the DataModule to get the ID from, including the ".rte".
		/// @return The requested ID. If no module of the name was found, -1 will be returned.
		int GetModuleID(const std::string_view& moduleName) const;

		/// Gets the name of a specific loaded DataModule.
		/// @param whichModule The ID of the module to get.
		/// @return The requested DataModule name.
		std::string GetModuleName(int whichModule = 0);

		/// Gets the ID of a loaded DataModule from a full data file path.
		/// @param dataPath The full path to a data file inside the data module ID you want to get.
		/// @return The requested ID. If no module of the name was found, -1 will be returned.
		int GetModuleIDFromPath(const std::string& dataPath) const { return GetModuleID(GetModuleNameFromPath(dataPath)); }

		/// Gets the name of a loaded DataModule from a full data file path.
		/// @param dataPath The full path to a data file inside the data module id you want to get.
		/// @return The requested Name. If no module of the name was found, "" will be returned.
		std::string GetModuleNameFromPath(const std::string& dataPath) const;

		/// Gets the full path to a DataModule including Data/, Userdata/ or Mods/.
		/// @param modulePath The Path to be completed.
		/// @return The complete path to the file, including Data/, Userdata/ or Mods/ based on whether or not it's part of an official module or userdata.
		std::string GetFullModulePath(const std::string& modulePath) const;
#pragma endregion

#pragma region Contrete Methods
		/// Reads an entire DataModule and adds it to this. NOTE that official modules can't be loaded after any non-official ones!
		/// @param moduleName The module name to read, e.g. "Base.rte".
		/// @param moduleType The type of module that is being read. See DataModule::DataModuleType enumeration.
		/// @param progressCallback A function pointer to a function that will be called and sent a string with information about the progress of this DataModule's creation.
		/// @return Whether the DataModule was read and added correctly.
		bool LoadDataModule(const std::string& moduleName, DataModule::DataModuleType moduleType, const ProgressCallback& progressCallback = nullptr);

		/// Unloads a DataModule by removing it from the loaded modules list. Does not actually erase it from memory.
		/// @param moduleName The DataModule name to unload.
		/// @return Whether the DataModule was successfully unloaded.
		bool UnloadDataModule(const std::string& moduleName);

		/// Loads all the official data modules individually with LoadDataModule, then proceeds to look for any non-official modules and loads them as well.
		bool LoadAllDataModules();
#pragma endregion

	private:
		static const std::array<std::string, 10> c_OfficialModules; //!< Array storing the names of all the official modules.
		static const std::array<std::pair<std::string, std::string>, 3> c_UserdataModules; //!< Array storing the names of all the userdata modules.

		std::unordered_map<int, DataModule*> m_LoadedDataModules; //!< Map of all loaded DataModules by their ID. Owned by this.
		std::unordered_map<int, DataModule*> m_UnloadedDataModules; //!< Map of DataModules that are loaded but disabled through the Mod Manager GUI. Treated as not loaded at all.

		std::unordered_set<std::string> m_DisabledDataModuleNames; //!< The DataModule names that are disabled and should not be loaded at startup.
		std::string m_SingleModuleToLoad; //!< Name of the single module to load after the official modules.

#pragma region Module Loading Breakdown
		/// Loads all the official DataModules individually with LoadDataModule.
		void LoadOfficialModules();

		/// Loads all the userdata DataModules individually with LoadDataModule. Creates any missing userdata DataModules on disk if necessary.
		/// @return
		bool LoadUserdataModules();

		/// Loads all the unofficial DataModules individually with LoadDataModule.
		void LoadUnofficialModules();

		/// Iterates through the working directory to find any files matching the zipped module package extension (.rte.zip) and proceeds to extract them.
		void FindAndExtractZippedModules() const;
#pragma endregion

		/// Clears all the member variables of this ModuleMan.
		void Clear();

		// Disallow the use of some implicit methods.
		ModuleMan(const ModuleMan& reference) = delete;
		ModuleMan& operator=(const ModuleMan& rhs) = delete;
	};
} // namespace RTE
#endif
