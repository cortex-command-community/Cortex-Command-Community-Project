#ifndef _RTEMODULEMAN_
#define _RTEMODULEMAN_

#include "Singleton.h"
#include "DataModule.h"

#define g_ModuleMan ModuleMan::Instance()

namespace RTE {

	/// <summary>
	/// Manager responsible for loading and unloading of DataModules.
	/// </summary>
	class ModuleMan : public Singleton<ModuleMan> {
		friend class SettingsMan;
		friend struct ManagerLuaBindings;

	public:
#pragma region Creation
		/// <summary>
		/// Constructor method used to instantiate a ModuleMan object in system memory. Initialize() should be called before using the object.
		/// </summary>
		ModuleMan() { Clear(); }
#pragma endregion

#pragma region Destruction
		/// <summary>
		/// Destroys and resets (through Clear()) the ModuleMan object.
		/// </summary>
		void Destroy();
#pragma endregion

#pragma region Getters and Setters
		/// <summary>
		/// Sets the single module to be loaded after the official modules. This will be the ONLY non-official module to be loaded.
		/// </summary>
		/// <param name="moduleName">Name of the module to load.</param>
		void SetSingleModuleToLoad(const std::string_view& moduleName) { m_SingleModuleToLoad = moduleName; }

		/// <summary>
		/// Gets the total number of loaded DataModules.
		/// </summary>
		/// <returns>The total number of loaded DataModules.</returns>
		int GetTotalModuleCount() const { return static_cast<int>(m_LoadedDataModules.size()); }

		/// <summary>
		/// Gets whether the specified mod is disabled in the settings.
		/// </summary>
		/// <param name="moduleName">Mod to check.</param>
		/// <returns>Whether the module is disabled via settings.</returns>
		bool IsModuleEnabled(const std::string_view& moduleName) const;

		/// <summary>
		/// Gets whether a module name is of an official DataModule.
		/// </summary>
		/// <param name="moduleName">The name of the module to check, in the form "moduleName.rte"</param>
		/// <returns>True if the module is an official DataModule, otherwise false.</returns>
		bool IsModuleOfficial(const std::string_view& moduleName) const;

		/// <summary>
		/// Gets whether a module name is of a userdata DataModule.
		/// </summary>
		/// <param name="moduleName">The name of the module to check, in the form "moduleName.rte"</param>
		/// <returns>True if the module is a userdata DataModule, otherwise false.</returns>
		bool IsModuleUserdata(const std::string_view& moduleName) const;
#pragma endregion

#pragma region DataModule Getters
		/// <summary>
		/// Gets all the loaded DataModules.
		/// </summary>
		/// <returns>Reference to the loaded DataModules map.</returns>
		std::unordered_map<int, DataModule*>& GetLoadedDataModules() { return m_LoadedDataModules; }

		/// <summary>
		/// Gets a specific loaded DataModule.
		/// </summary>
		/// <param name="whichModule">The ID of the module to get.</param>
		/// <returns>The requested DataModule. Ownership is NOT transferred!</returns>
		DataModule* GetDataModule(int whichModule = 0);

		/// <summary>
		/// Gets a specific loaded DataModule.
		/// </summary>
		/// <param name="whichModule">The name of the DataModule to get, including the ".rte".</param>
		/// <returns>The requested DataModule. Ownership is NOT transferred!</returns>
		DataModule* GetDataModule(const std::string_view& moduleName) { return GetDataModule(GetModuleID(moduleName)); }
#pragma endregion

#pragma region DataModule Info Getters
		/// <summary>
		/// Gets the DataModule names that are disabled and should not be loaded at startup.
		/// </summary>
		/// <returns>Map of mods which are disabled.</returns>
		std::unordered_set<std::string>& GetDisabledDataModuleNames() { return m_DisabledDataModuleNames; }

		/// <summary>
		/// Gets the ID of a loaded DataModule.
		/// </summary>
		/// <param name="moduleName">The name of the DataModule to get the ID from, including the ".rte".</param>
		/// <returns>The requested ID. If no module of the name was found, -1 will be returned.</returns>
		int GetModuleID(const std::string_view& moduleName) const;

		/// <summary>
		/// Gets the name of a specific loaded DataModule.
		/// </summary>
		/// <param name="whichModule">The ID of the module to get.</param>
		/// <returns>The requested DataModule name.</returns>
		std::string GetModuleName(int whichModule = 0);

		/// <summary>
		/// Gets the ID of a loaded DataModule from a full data file path.
		/// </summary>
		/// <param name="dataPath">The full path to a data file inside the data module ID you want to get.</param>
		/// <returns>The requested ID. If no module of the name was found, -1 will be returned.</returns>
		int GetModuleIDFromPath(const std::string& dataPath) const { return GetModuleID(GetModuleNameFromPath(dataPath)); }

		/// <summary>
		/// Gets the name of a loaded DataModule from a full data file path.
		/// </summary>
		/// <param name="dataPath">The full path to a data file inside the data module id you want to get.</param>
		/// <returns>The requested Name. If no module of the name was found, "" will be returned.</returns>
		std::string GetModuleNameFromPath(const std::string& dataPath) const;

		/// <summary>
		/// Gets the full path to a DataModule including Data/, Userdata/ or Mods/.
		/// </summary>
		/// <param name="modulePath">The Path to be completed.</param>
		/// <returns>The complete path to the file, including Data/, Userdata/ or Mods/ based on whether or not it's part of an official module or userdata.</returns>
		std::string GetFullModulePath(const std::string& modulePath) const;
#pragma endregion

#pragma region Contrete Methods
		/// <summary>
		/// Reads an entire DataModule and adds it to this. NOTE that official modules can't be loaded after any non-official ones!
		/// </summary>
		/// <param name="moduleName">The module name to read, e.g. "Base.rte".</param>
		/// <param name="moduleType">The type of module that is being read. See DataModule::DataModuleType enumeration.</param>
		/// <param name="progressCallback">A function pointer to a function that will be called and sent a string with information about the progress of this DataModule's creation.</param>
		/// <returns>Whether the DataModule was read and added correctly.</returns>
		bool LoadDataModule(const std::string& moduleName, DataModule::DataModuleType moduleType, const ProgressCallback& progressCallback = nullptr);

		/// <summary>
		/// Unloads a DataModule by removing it from the loaded modules list. Does not actually erase it from memory.
		/// </summary>
		/// <param name="moduleName">The DataModule name to unload.</param>
		/// <returns>Whether the DataModule was successfully unloaded.</returns>
		bool UnloadDataModule(const std::string& moduleName);

		/// <summary>
		/// Loads all the official data modules individually with LoadDataModule, then proceeds to look for any non-official modules and loads them as well.
		/// </summary>
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
		/// <summary>
		/// Loads all the official DataModules individually with LoadDataModule.
		/// </summary>
		void LoadOfficialModules();

		/// <summary>
		/// Loads all the userdata DataModules individually with LoadDataModule. Creates any missing userdata DataModules on disk if necessary.
		/// </summary>
		/// <returns></returns>
		bool LoadUserdataModules();

		/// <summary>
		/// Loads all the unofficial DataModules individually with LoadDataModule.
		/// </summary>
		void LoadUnofficialModules();

		/// <summary>
		/// Iterates through the working directory to find any files matching the zipped module package extension (.rte.zip) and proceeds to extract them.
		/// </summary>
		void FindAndExtractZippedModules() const;
#pragma endregion

		/// <summary>
		/// Clears all the member variables of this ModuleMan.
		/// </summary>
		void Clear();

		// Disallow the use of some implicit methods.
		ModuleMan(const ModuleMan& reference) = delete;
		ModuleMan& operator=(const ModuleMan& rhs) = delete;
	};
} // namespace RTE
#endif