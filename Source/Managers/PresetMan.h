#pragma once

/// Header file for the PresetMan class.
/// @author Daniel Tabar
/// data@datarealms.com
/// http://www.datarealms.com
/// Inclusions of header files
#include "Entity.h"
#include "Singleton.h"

#include <array>
#include <list>
#include <map>
#include <string>
#include <vector>

#define g_PresetMan PresetMan::Instance()

namespace RTE {

	class Actor;
	class DataModule;

	/// The singleton manager of all presets of Entity:s in the RTE.
	/// The presets serve as a respository of Entity instances with specific
	/// and unique and initial runtime data.
	class PresetMan : public Singleton<PresetMan> {
		friend struct ManagerLuaBindings;

		/// Public member variable, method and friend function declarations
	public:
		/// Constructor method used to instantiate a PresetMan entity in system
		/// memory. Create() should be called before using the entity.
		PresetMan();

		/// Destructor method used to clean up a PresetMan entity before deletion
		/// from system memory.
		~PresetMan();

		/// Makes the PresetMan entity ready for use.
		/// @return An error return value signaling sucess or any particular failure.
		/// Anything below 0 is an error signal.
		int Initialize() { return 0; }

		/// Resets the entire PresetMan, including its inherited members, to
		/// their default settings or values.
		void Reset() { Clear(); }

		/// Destroys and resets (through Clear()) the PresetMan entity.
		void Destroy();

		/// Reads an entire DataModule and adds it to this. NOTE that official modules can't be loaded after any non-official ones!
		/// @param moduleName The module name to read, e.g. "Base.rte".
		/// @param official Whether this module is 'official' or third party. If official, it has to not have any name conflicts with any other official module.
		/// @param userdata Whether this module is a userdata module. If true, will be treated as an unofficial module.
		/// @param progressCallback A function pointer to a function that will be called and sent a string with information about the progress of this DataModule's creation.
		/// @return Whether the DataModule was read and added correctly.
		bool LoadDataModule(const std::string& moduleName, bool official, bool userdata = false, const ProgressCallback& progressCallback = nullptr);

		/// Reads an entire DataModule and adds it to this. NOTE that official modules can't be loaded after any non-official ones!
		/// @param moduleName The module name to read, e.g. "Base.rte".
		/// @return Whether the DataModule was read and added correctly.
		bool LoadDataModule(const std::string& moduleName) { return LoadDataModule(moduleName, false); }

		/// Loads all the official data modules individually with LoadDataModule, then proceeds to look for any non-official modules and loads them as well.
		/// @return
		bool LoadAllDataModules();

		/// Sets the single module to be loaded after the official modules. This will be the ONLY non-official module to be loaded.
		/// @param moduleName Name of the module to load.
		void SetSingleModuleToLoad(std::string moduleName) { m_SingleModuleToLoad = moduleName; }

		/// Gets a specific loaded DataModule
		/// @param whichModule The ID of the module to get. (default: 0)
		/// @return The requested DataModule. Ownership is NOT transferred!
		const DataModule* GetDataModule(int whichModule = 0);

		/// Gets a name specific loaded DataModule
		/// @param whichModule The ID of the module to get. (default: 0)
		/// @return The requested DataModule name.
		const std::string GetDataModuleName(int whichModule = 0);

		/// Gets the ID of a loaded DataModule.
		/// @param moduleName The name of the DataModule to get the ID from, including the ".rte"
		/// @return The requested ID. If no module of the name was found, -1 will be returned.
		int GetModuleID(std::string moduleName);

		///  Gets the Name of a loaded DataModule, from a full data file path.
		/// @param dataPath The full path to a data file inside the data module id you want to get.
		/// @return The requested Name. If no module of the name was found, "" will be returned.
		std::string GetModuleNameFromPath(const std::string& dataPath) const;

		/// Gets the ID of a loaded DataModule from a full data file path.
		/// @param dataPath The full path to a data file inside the data module ID you want to get.
		/// @return The requested ID. If no module of the name was found, -1 will be returned.
		int GetModuleIDFromPath(const std::string& dataPath);

		/// Returns whether or not the module is vanilla.
		/// @param moduleName The name of the module to check, in the form "[moduleName].rte"
		/// @return True if the module is an official data module, otherwise false.
		bool IsModuleOfficial(const std::string& moduleName) const;

		/// Returns whether or not the module is vanilla.
		/// @param moduleName The name of the module to check, in the form "[moduleName].rte"
		/// @return True if the module is a listed user data module, otherwise false.
		bool IsModuleUserdata(const std::string& moduleName) const;

		/// Returns the Full path to the module including Data/, Userdata/ or Mods/.
		/// @param modulePath The Path to be completed.
		/// @return The complete path to the file, including Data/, Userdata/ or Mods/ based on whether or not it's part of an official module or userdata.
		std::string GetFullModulePath(const std::string& modulePath) const;

		/// Gets the total number of modules loaded so far, official or not.
		/// @return The number of modules loaded so far, both official and non.
		int GetTotalModuleCount() { return m_pDataModules.size(); }

		/// Gets the total number of OFFICIAL modules loaded so far.
		/// @return The number of official modules loaded so far.
		int GetOfficialModuleCount() { return m_OfficialModuleCount; }

		/// Adds an Entity instance's pointer and name associations to the
		/// internal list of already read in Entity:s. Ownership is NOT transferred!
		/// If there already is an instance defined, nothing happens. If there
		/// is not, a clone is made of the passed-in Entity and added to the library.
		/// @param pEntToAdd A pointer to the Entity derived instance to add. It should be created
		/// from a Reader. Ownership is NOT transferred!
		/// @param whichModule Which module to add the entity to. (default: 0)
		/// @param overwriteSame Whether to overwrite if an instance of the EXACT same TYPE and name (default: false)
		/// was found. If one of the same name but not the exact type, false
		/// is returned regardless and nothing will have been added.
		/// @param readFromFile The file this instance was read from, or where it should be written. (default: "Same")
		/// If "Same" is passed as the file path read from, an overwritten instance
		/// will keep the old one's file location entry.
		/// @return Whether or not a copy of the passed-in instance was successfully inserted
		/// into the module. False will be returned if there already was an instance
		/// of that class and instance name inserted previously, unless overwritten.
		bool AddEntityPreset(Entity* pEntToAdd, int whichModule = 0, bool overwriteSame = false, std::string readFromFile = "Same");

		/// Gets a previously read in (defined) Entity, by type and instance name.
		/// @param type The type name of the derived Entity. Ownership is NOT transferred!
		/// @param preset The instance name of the derived Entity instance.
		/// @param whichModule Which module to try to get the entity from. If it's not found there, (default: -1)
		/// the official modules will be searched also. -1 means search ALL modules!
		/// @return A pointer to the requested Entity instance. 0 if no Entity with that
		/// derived type or instance name was found. Ownership is NOT transferred!
		const Entity* GetEntityPreset(std::string type, std::string preset, int whichModule = -1);
		// Helper for passing in string module name instead of ID
		const Entity* GetEntityPreset(std::string type, std::string preset, std::string module) { return GetEntityPreset(type, preset, GetModuleID(module)); }

		/// Reads an instance of an Entity that will be used as preset
		/// to later on be used to generate more instances of the same state.
		/// Will check if there was already one of the same class and instance
		/// name read in previously, and will return that one if found, or
		/// add the newly read in one to this PresetMan's list if not found to
		/// exist there previously. Ownership is NOT transferred!
		/// @param reader The Reader which is about to read in an instance reference. It'll make
		/// this look in the same module as it's reading from.
		/// @return A const pointer to the Entity instance read in. 0 if there was an
		/// error, or the instance name was 'None'. Ownership is NOT transferred!
		const Entity* GetEntityPreset(Reader& reader);

		/// Reads a preset of an Entity and tries to add it to the list of
		/// read-in instances. Regardless of whether there is a name collision,
		/// the read-in preset will be returned, ownership TRANSFERRED!
		/// @param reader The Reader which is about to read in a preset.
		/// @return A pointer to the Entity preset read in. 0 if there was an
		/// error, or the instance name was 'None'. Ownership IS transferred!
		Entity* ReadReflectedPreset(Reader& reader);

		/// Adds to a list all previously read in (defined) Entitys, by type.
		/// @param entityList Reference to a list which will get all matching Entity:s added to it.
		/// @param type Ownership of the list or the Entitys placed in it are NOT transferred!
		/// @param whichModule The type name of the Entitys you want. (default: -1)
		/// Whether to only get those of one specific DataModule (0-n), or all (-1).
		/// @return Whether any Entity:s were found and added to the list.
		bool GetAllOfType(std::list<Entity*>& entityList, std::string type, int whichModule = -1);

		/// Adds to a list all previously read in (defined) Entitys which are
		/// of a specific type, and only exist in a specific module space.
		/// @param entityList Reference to a list which will get all matching Entity:s added to it.
		/// @param type Ownership of the list or the Entitys placed in it are NOT transferred!
		/// @param whichModuleSpace The type name of the Entitys you want.
		/// Which module to get the instances for, in addition to all groups in
		/// official modules loaded earlier than the one specified here. -1 means
		/// get ALL groups ever reg'd.
		/// @return Whether any Entity:s were found and added to the list.
		bool GetAllOfTypeInModuleSpace(std::list<Entity*>& entityList, std::string type, int whichModuleSpace);

		/// Adds to a list all previously read in (defined) Entities which are associated with a specific group.
		/// @param entityList Reference to a list which will get all matching Entities added to it. Ownership of the list or the Entities placed in it are NOT transferred!
		/// @param group The group to look for. "All" will look in all.
		/// @param type The name of the least common denominator type of the Entities you want. "All" will look at all types.
		/// @param whichModule Whether to only get those of one specific DataModule (0-n), or all (-1).
		/// @return Whether any Entities were found and added to the list.
		bool GetAllOfGroup(std::list<Entity*>& entityList, const std::string& group, const std::string& type = "All", int whichModule = -1) { return GetAllOfGroups(entityList, {group}, type, whichModule); }

		/// Adds to a list all previously read in (defined) Entities which are associated with several specific groups.
		/// @param entityList Reference to a list which will get all matching Entities added to it. Ownership of the list or the Entities placed in it are NOT transferred!
		/// @param groups The groups to look for. "All" will look in all.
		/// @param type The name of the least common denominator type of the Entities you want. "All" will look at all types.
		/// @param whichModule Whether to only get those of one specific DataModule (0-n), or all (-1).
		/// @return Whether any Entities were found and added to the list.
		bool GetAllOfGroups(std::list<Entity*>& entityList, const std::vector<std::string>& groups, const std::string& type = "All", int whichModule = -1);

		/// Adds to a list all previously read in (defined) Entities which are not associated with a specific group.
		/// @param entityList Reference to a list which will get all matching Entities added to it. Ownership of the list or the Entities placed in it are NOT transferred!
		/// @param group The group to exclude.
		/// @param type The name of the least common denominator type of the Entities you want. "All" will look at all types.
		/// @param whichModule Whether to only get those of one specific DataModule (0-n), or all (-1).
		/// @return Whether any Entities were found and added to the list.
		bool GetAllNotOfGroup(std::list<Entity*>& entityList, const std::string& group, const std::string& type = "All", int whichModule = -1) { return GetAllNotOfGroups(entityList, {group}, type, whichModule); }

		/// Adds to a list all previously read in (defined) Entities which are not associated with several specific groups.
		/// @param entityList Reference to a list which will get all matching Entities added to it. Ownership of the list or the Entities placed in it are NOT transferred!
		/// @param groups The groups to exclude.
		/// @param type The name of the least common denominator type of the Entities you want. "All" will look at all types.
		/// @param whichModule Whether to only get those of one specific DataModule (0-n), or all (-1).
		/// @return Whether any Entities were found and added to the list.
		bool GetAllNotOfGroups(std::list<Entity*>& entityList, const std::vector<std::string>& groups, const std::string& type = "All", int whichModule = -1);

		/// Returns a previously read in (defined) Entity which is randomly
		/// selected from a specific group.
		/// @param group The group to randomly select an Entity from. "All" will look in all.
		/// @param type The name of the least common denominator type of the Entitys you want. (default: "All")
		/// "All" will look at all types.
		/// @param whichModule Whether to only get those of one specific DataModule (0-n), or all (-1). (default: -1)
		/// @return The Entity preset that was randomly selected. Ownership is NOT transferred!
		Entity* GetRandomOfGroup(std::string group, std::string type = "All", int whichModule = -1);

		/// Returns a previously read in (defined) Entity which is randomly
		/// selected from a specific group only if it belongs to some tech.
		/// @param group The group to randomly select an Entity from. "All" will look in all.
		/// @param type The name of the least common denominator type of the Entitys you want. (default: "All")
		/// "All" will look at all types.
		/// @param whichModule Whether to only get those of one specific DataModule (0-n), or all (-1) (default: -1)
		/// or all modules uncluding non-tech ones.
		/// @return The Entity preset that was randomly selected. Ownership is NOT transferred!
		Entity* GetRandomBuyableOfGroupFromTech(std::string group, std::string type = "All", int whichModule = -1);

		/// Adds to a list all previously read in (defined) Entitys which are
		/// associated with a specific group, and only exist in a specific module
		/// space.
		/// @param entityList Reference to a list which will get all matching Entity:s added to it.
		/// @param group Ownership of the list or the Entitys placed in it are NOT transferred!
		/// @param type The group to look for. "All" will look in all.
		/// @param whichModuleSpace The name of the least common denominator type of the Entitys you want.
		/// "All" will look at all types.
		/// Which module to get the instances for, in addition to all groups in
		/// official modules loaded earlier than the one specified here. -1 means
		/// get ALL groups ever reg'd.
		/// @return Whether any Entity:s were found and added to the list.
		bool GetAllOfGroupInModuleSpace(std::list<Entity*>& entityList, std::string group, std::string type, int whichModuleSpace);

		/// Returns a previously read in (defined) Entity which is associated with
		/// a specific group, randomly selected and only exist in a specific module
		/// space.
		/// @param group Ownership of the list or the Entitys placed in it are NOT transferred!
		/// @param type The group to randomly select from. "All" will look in all.
		/// @param whichModuleSpace The name of the least common denominator type of the Entity:s you want.
		/// "All" will look at all types.
		/// Which module to get the instances for, in addition to all groups in
		/// official modules loaded earlier than the one specified here. -1 means
		/// get ALL groups ever reg'd.
		/// @return The randomly select preset, if any was found with thse search params.
		/// Ownership is NOT transferred!
		Entity* GetRandomOfGroupInModuleSpace(std::string group, std::string type, int whichModuleSpace);

		/// Gets the data file path of a previously read in (defined) Entity.
		/// @param type The type name of the derived Entity. Ownership is NOT transferred!
		/// @param preset The preset name of the derived Entity preset.
		/// @param whichModule Which module to try to get the entity from. If it's not found there,
		/// the official modules will be searched also.
		/// @return The file path of the data file that the specified Entity was read from.
		/// If no Entity of that description was found, "" is returned.
		std::string GetEntityDataLocation(std::string type, std::string preset, int whichModule);

		/// Reloads all scripted Entity Presets with the latest version of their respective script files.
		void ReloadAllScripts() const;

		/// Reloads an Entity preset and all related presets with the latest version of their respective files.
		/// Stores the passed in Entity preset info for later re-use in PresetMan::QuickReloadEntityPreset.
		/// @param presetName The name of the preset to reload.
		/// @param className The type of the preset to reload, to avoid any ambiguity.
		/// @param dataModule The DataModule the preset to reload is defined in.
		/// @param storeReloadedPresetDataForQuickReloading Whether or not to store the reloaded entity preset data for quick reloading.
		/// @return Whether reloading the preset was successful.
		bool ReloadEntityPreset(const std::string& presetName, const std::string& className, const std::string& dataModule, bool storeReloadedPresetDataForQuickReloading = true);

		/// Reloads the previously reloaded Entity preset and all related presets with the latest version of their respective files.
		/// @return Whether reloading the preset was successful.
		bool QuickReloadEntityPreset();

		/// Gets whether or not ReloadEntityPreset was called this update.
		/// @return Whether or not ReloadEntityPreset was called this update.
		bool GetReloadEntityPresetCalledThisUpdate() const { return m_ReloadEntityPresetCalledThisUpdate; }

		/// Resets whether or not ReloadEntityPreset was called this update.
		void ClearReloadEntityPresetCalledThisUpdate() { m_ReloadEntityPresetCalledThisUpdate = false; }

		/// Adds a Material mapping local to a DataModule. This is used for when
		/// multiple DataModule:s are loading conflicting Material:s, and need to
		/// resolve the conflicts by mapping their materials to ID's different than
		/// those specified in the data files.
		/// @param fromID The material ID to map from.
		/// @param toID The material ID to map to.
		/// @param whichModule The ID of the  DataModule we are making the mapping for. This should be
		/// a non-official module as mapping shouldn't be needed in the official
		/// modules.
		/// @return Whether this created a new mapping which didn't override a previous
		/// material mapping.
		bool AddMaterialMapping(int fromID, int toID, int whichModule);

		/// Registers the existence of an Entity group, and in which module.
		/// @param newGroup The group to register.
		/// @param whichModule The ID of the module in which at least one entity of this group can be
		/// found.
		/// global register.
		void RegisterGroup(std::string newGroup, int whichModule);

		/// Fills out a list with all groups registered in a specific module.
		/// @param groupList The list that all found groups will be ADDED to. OWNERSHIP IS NOT TRANSFERRED!
		/// @param whichModule Which module to get the groups for. -1 means get ALL groups ever reg'd. (default: -1)
		/// @param withType Pass a type name here and only groups with entitys of that type will be (default: "All")
		/// be included. "All" means don't consider what types are in the groups.
		/// @return Whether any groups were found and thus added to the list.
		bool GetGroups(std::list<std::string>& groupList, int whichModule = -1, std::string withType = "All") const;

		/// Fills out a list with all groups registered in all official modules,
		/// PLUS a specific non-official module as well.
		/// @param groupList The list that all found groups will be ADDED to. OWNERSHIP IS NOT TRANSFERRED!
		/// @param whichModule Which module to get the groups for, in addition to all groups in
		/// official modules loaded earlier than the one specified here. -1 means
		/// get ALL groups ever reg'd.
		/// @param withType Pass a type name here and only groups with entitys of that type will be (default: "All")
		/// be included. "All" means don't consider what types are in the groups.
		/// @return Whether any groups were found and thus added to the list.
		bool GetModuleSpaceGroups(std::list<std::string>& groupList, int whichModule, std::string withType = "All") const;

		/// Creates and returns actor defined in the specified loadout.
		/// @param loadoutName Loadout preset name, module name, whether or not spawn delivery craft defined for that loadout
		/// @return Created actor if matching loadout was found or 0. OWNERSHIP IS TRANSFERED.
		Actor* GetLoadout(std::string loadoutName, std::string module, bool spawnDropShip);

		/// Creates and returns actor defined in the specified loadout.
		/// @param loadoutName Loadout preset name, module id, whether or not spawn delivery craft defined for that loadout
		/// @return Created actor if matching loadout was found or 0. OWNERSHIP IS TRANSFERED.
		Actor* GetLoadout(std::string loadoutName, int moduleNumber, bool spawnDropShip);

		/// Protected member variable and method declarations
	protected:
		// Owned and loaded DataModule:s
		std::vector<DataModule*> m_pDataModules;

		// Names of all DataModule:s mapped to indices into the m_pDataModules vector.
		// The names are all lowercase name so we can more easily find them in case-agnostic fashion
		std::map<std::string, size_t> m_DataModuleIDs;

		// How many modules are 'official' and shipped with the game, and guaranteed to not have name conflicts among them
		// All official modules are in the beginning of the m_TypeMap, so this count shows how many into that vector they represent
		int m_OfficialModuleCount;

		std::string m_SingleModuleToLoad; //!< Name of the single module to load after the official modules.

		// List of all Entity groups ever registered, all uniques
		// This is just a handy total of all the groups registered in all the individual DataModule:s
		std::list<std::string> m_TotalGroupRegister;

		/// Private member variable and method declarations
	private:
		static const std::array<std::string, 10> c_OfficialModules; // Array storing the names of all the official modules.
		static const std::array<std::pair<std::string, std::string>, 3> c_UserdataModules; // Array storing the names of all the userdata modules.

		std::array<std::string, 3> m_LastReloadedEntityPresetInfo; //!< Array storing the last reloaded Entity preset info (ClassName, PresetName and DataModule). Used for quick reloading via key combination.
		bool m_ReloadEntityPresetCalledThisUpdate; //!< A flag for whether or not ReloadEntityPreset was called this update.

		/// Iterates through the working directory to find any files matching the zipped module package extension (.rte.zip) and proceeds to extract them.
		void FindAndExtractZippedModules() const;

		/// Clears all the member variables of this PresetMan, effectively
		/// resetting the members of this abstraction level only.
		void Clear();

		// Disallow the use of some implicit methods.
		PresetMan(const PresetMan& reference) = delete;
		PresetMan& operator=(const PresetMan& rhs) = delete;
	};

} // namespace RTE
