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
		void SetSingleModuleToLoad(const std::string_view &moduleName) { m_SingleModuleToLoad = moduleName; }

		/// <summary>
		/// Gets the total number of modules loaded so far, official or not.
		/// </summary>
		/// <returns>The number of modules loaded so far, both official and non.</returns>
		size_t GetTotalModuleCount() const { return m_LoadedDataModules.size(); }







		/// <summary>
		/// Gets all the loaded DataModules.
		/// </summary>
		/// <returns>All the loaded DataModules.</returns>
		std::unordered_map<int, DataModule *> & GetLoadedDataModules() { return m_LoadedDataModules; }








		/// <summary>
		/// Gets the map of mods which are disabled.
		/// </summary>
		/// <returns>Map of mods which are disabled.</returns>
		std::map<std::string, bool> & GetDisabledModsMap() { return m_DisabledMods; }

		/// <summary>
		/// Gets whether the specified mod is disabled in the settings.
		/// </summary>
		/// <param name="modModule">Mod to check.</param>
		/// <returns>Whether the mod is disabled via settings.</returns>
		bool IsModDisabled(const std::string &modModule) const;



		/// <summary>
		/// Returns whether or not the module is vanilla.
		/// </summary>
		/// <param name="moduleName">The name of the module to check, in the form "[moduleName].rte"</param>
		/// <returns>True if the module is an official data module, otherwise false.</returns>
		bool IsModuleOfficial(const std::string &moduleName) const;

		/// <summary>
		/// Returns whether or not the module is vanilla.
		/// </summary>
		/// <param name="moduleName">The name of the module to check, in the form "[moduleName].rte"</param>
		/// <returns>True if the module is a listed user data module, otherwise false.</returns>
		bool IsModuleUserdata(const std::string &moduleName) const;





		/// <summary>
		/// Gets a specific loaded DataModule.
		/// </summary>
		/// <param name="whichModule">The ID of the module to get.</param>
		/// <returns>The requested DataModule. Ownership is NOT transferred!</returns>
		const DataModule * GetDataModule(int whichModule = 0);













		/// <summary>
		/// Gets the ID of a loaded DataModule.
		/// </summary>
		/// <param name="moduleName">The name of the DataModule to get the ID from, including the ".rte".</param>
		/// <returns>The requested ID. If no module of the name was found, -1 will be returned.</returns>
		int GetModuleID(std::string moduleName);

		/// <summary>
		/// Gets the ID of a loaded DataModule from a full data file path.
		/// </summary>
		/// <param name="dataPath">The full path to a data file inside the data module ID you want to get.</param>
		/// <returns>The requested ID. If no module of the name was found, -1 will be returned.</returns>
		int GetModuleIDFromPath(const std::string &dataPath);

		/// <summary>
		/// Gets a name specific loaded DataModule.
		/// </summary>
		/// <param name="whichModule">The ID of the module to get.</param>
		/// <returns>The requested DataModule name.</returns>
		const std::string GetModuleName(int whichModule = 0);

		/// <summary>
		///  Gets the Name of a loaded DataModule, from a full data file path.
		/// </summary>
		/// <param name="dataPath">The full path to a data file inside the data module id you want to get.</param>
		/// <returns>The requested Name. If no module of the name was found, "" will be returned.</returns>
		std::string GetModuleNameFromPath(const std::string &dataPath) const;

		/// <summary>
		/// Returns the Full path to the module including Data/, Userdata/ or Mods/.
		/// </summary>
		/// <param name="modulePath">The Path to be completed.</param>
		/// <returns>The complete path to the file, including Data/, Userdata/ or Mods/ based on whether or not it's part of an official module or userdata.</returns>
		std::string GetFullModulePath(const std::string &modulePath) const;













#pragma endregion

#pragma region Contrete Methods
		/// <summary>
		/// Reads an entire DataModule and adds it to this. NOTE that official modules can't be loaded after any non-official ones!
		/// </summary>
		/// <param name="moduleName">The module name to read, e.g. "Base.rte".</param>
		/// <param name="moduleType">The type of module that is being read. See DataModule::DataModuleType enumeration.</param>
		/// <param name="progressCallback">A function pointer to a function that will be called and sent a string with information about the progress of this DataModule's creation.</param>
		/// <returns>Whether the DataModule was read and added correctly.</returns>
		bool LoadDataModule(const std::string &moduleName, DataModule::DataModuleType moduleType, const ProgressCallback &progressCallback = nullptr);

		/// <summary>
		/// Loads all the official data modules individually with LoadDataModule, then proceeds to look for any non-official modules and loads them as well.
		/// </summary>
		/// <returns></returns>
		bool LoadAllDataModules();
#pragma endregion

	private:

		static const std::string c_ClassName; //!< A string with the friendly-formatted type name of this.

		static const std::array<std::string, 10> c_OfficialModules; //!< Array storing the names of all the official modules.
		static const std::array<std::pair<std::string, std::string>, 3> c_UserdataModules; //!< Array storing the names of all the userdata modules.

		std::unordered_map<int, DataModule *> m_LoadedDataModules; //!< Map of all loaded DataModules by their ID. Owned by this.

		std::string m_SingleModuleToLoad; //!< Name of the single module to load after the official modules.

		std::map<std::string, bool> m_DisabledMods; //!< Map of the module names that are disabled.

#pragma region Module Loading Breakdown
		/// <summary>
		///
		/// </summary>
		/// <returns></returns>
		void LoadOfficialModules();

		/// <summary>
		/// Loads all the userdata DataModules individually with LoadDataModule. Creates any missing userdata DataModules on disk if necessary.
		/// </summary>
		/// <returns></returns>
		bool LoadUserdataModules();

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
		ModuleMan(const ModuleMan &reference) = delete;
		ModuleMan & operator=(const ModuleMan &rhs) = delete;
	};
}
#endif