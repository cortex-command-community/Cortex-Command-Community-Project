#ifndef _RTEPRESETMAN_
#define _RTEPRESETMAN_

#include "Singleton.h"
#include "Entity.h"

#define g_PresetMan PresetMan::Instance()

namespace RTE {

	class Actor;
	class DataModule;

	/// <summary>
	/// The singleton manager of all presets of Entities in the RTE.
	/// The presets serve as a repository of Entity instances with specific, unique and initial runtime data.
	/// </summary>
	class PresetMan : public Singleton<PresetMan> {
		friend struct ManagerLuaBindings;

	public:
#pragma region Creation
		/// <summary>
		/// Constructor method used to instantiate a PresetMan object in system memory. Initialize() should be called before using the object.
		/// </summary>
		PresetMan() { Clear(); }
#pragma endregion

#pragma region Destruction
		/// <summary>
		/// Resets the entire PresetMan, including its inherited members, to their default settings or values.
		/// </summary>
		void Reset() { Clear(); }
#pragma endregion

#pragma region Group Register Handling
		/// <summary>
		/// Registers the existence of an Entity group, and in which module.
		/// </summary>
		/// <param name="groupName">The group to register.</param>
		/// <param name="whichModule">The ID of the module in which at least one Entity of this group can be found.</param>
		void RegisterGroup(const std::string& groupName, int whichModule) const;

		/// <summary>
		/// Fills out a list with all groups registered in a specific module.
		/// </summary>
		/// <param name="groupList">The list that all found groups will be added to. OWNERSHIP IS NOT TRANSFERRED!</param>
		/// <param name="whichModule">Which module to get the groups for. -1 means get ALL groups ever registered.</param>
		/// <param name="withType">Pass a type name here and only groups with Entities of that type will be included. "All" means don't consider what types are in the groups.</param>
		/// <param name="moduleSpace">Whether to get all groups in official modules, in addition to the specified module.</param>
		/// <returns>Whether any groups were found and added to the list.</returns>
		bool GetGroups(std::list<std::string>& groupList, int whichModule = -1, const std::string& withType = "All", bool moduleSpace = false) const;

		/// <summary>
		/// Fills out a list with all groups registered in all official modules, PLUS a specific non-official module as well.
		/// </summary>
		/// <param name="groupList">The list that all found groups will be added to. OWNERSHIP IS NOT TRANSFERRED!</param>
		/// <param name="whichModule">Which module to get the groups for, in addition to all groups in official modules. -1 means get ALL groups ever registered.</param>
		/// <param name="withType">Pass a type name here and only groups with Entities of that type will be included. "All" means don't consider what types are in the groups.</param>
		/// <returns>Whether any groups were found and added to the list.</returns>
		bool GetModuleSpaceGroups(std::list<std::string>& groupList, int whichModuleSpace, const std::string& withType = "All") const { return GetGroups(groupList, whichModuleSpace, withType, true); }
#pragma endregion

#pragma region Preset Getters
		/// <summary>
		/// Reads an instance of an Entity that will be used as preset to later on be used to generate more instances of the same state. Ownership is NOT transferred!
		/// Will check if there was already one of the same class and instance name read in previously, and will return that one if found, or add the newly read in one to this PresetMan's list if not found to exist there previously.
		/// </summary>
		/// <param name="reader">The Reader which is about to read in an instance reference. It'll make this look in the same module as it's reading from.</param>
		/// <returns>A pointer to const of the Entity instance read in. nullptr if there was an error, or the instance name was 'None'. Ownership is NOT transferred!</returns>
		const Entity* GetEntityPreset(Reader& reader);

		/// <summary>
		/// Gets a previously read in (defined) Entity, by type and instance name. Ownership is NOT transferred!
		/// </summary>
		/// <param name="typeName">The type name of the derived Entity.</param>
		/// <param name="presetName">The instance name of the derived Entity instance.</param>
		/// <param name="whichModule">Which module to try to get the entity from. If it's not found there, the official modules will be searched also. -1 means search ALL modules!</param>
		/// <returns>A pointer to const of the requested Entity instance. nullptr if no Entity with that derived type or instance name was found. Ownership is NOT transferred!</returns>
		const Entity* GetEntityPreset(const std::string& typeName, std::string presetName, int whichModule = -1) const;

		/// <summary>
		/// Gets a previously read in (defined) Entity, by type and instance name. Ownership is NOT transferred!
		/// </summary>
		/// <param name="typeName">The type name of the derived Entity.</param>
		/// <param name="presetName">The instance name of the derived Entity instance.</param>
		/// <param name="whichModule">Which module to try to get the entity from. If it's not found there, the official modules will be searched also.</param>
		/// <returns>A pointer to const of the requested Entity instance. nullptr if no Entity with that derived type or instance name was found. Ownership is NOT transferred!</returns>
		const Entity* GetEntityPreset(const std::string& typeName, const std::string& presetName, const std::string& whichModule) const;

		/// <summary>
		/// Gets the data file path of a previously read in (defined) Entity. Ownership is NOT transferred!
		/// </summary>
		/// <param name="typeName">The type name of the derived Entity.</param>
		/// <param name="presetName">The preset name of the derived Entity preset.</param>
		/// <param name="whichModule">Which module to try to get the entity from. If it's not found there, the official modules will be searched also.</param>
		/// <returns>The file path of the data file that the specified Entity was read from. If no Entity of that description was found, an empty string is returned.</returns>
		std::string GetEntityDataLocation(const std::string& typeName, const std::string& presetName, int whichModule) const;
#pragma endregion

#pragma region Preset-by-Type Getters
		/// <summary>
		/// Adds to a list all previously read in (defined) Entities, by type.
		/// </summary>
		/// <param name="entityList">Reference to a list which will get all matching Entities added to it. Ownership of the list or the Entities placed in it are NOT transferred!</param>
		/// <param name="typeName">The type name of the Entities to get.</param>
		/// <param name="whichModule">Whether to only get those of one specific DataModule (0-n), or all (-1).</param>
		/// <param name="moduleSpace">Whether to get all instances in official modules, in addition to the specified module.</param>
		/// <returns>Whether any Entities were found and added to the list.</returns>
		bool GetAllOfType(std::list<Entity*>& entityList, const std::string& typeName, int whichModule = -1, bool moduleSpace = false) const;

		/// <summary>
		/// Adds to a list all previously read in (defined) Entities which are of a specific type, and only exist in a specific module space.
		/// </summary>
		/// <param name="entityList">Reference to a list which will get all matching Entities added to it. Ownership of the list or the Entities placed in it are NOT transferred!</param>
		/// <param name="typeName">The type name of the Entities to get.</param>
		/// <param name="whichModuleSpace">Which module to get the instances for, in addition to all groups in official modules. -1 means get ALL groups ever registered.</param>
		/// <returns>Whether any Entities were found and added to the list.</returns>
		bool GetAllOfTypeInModuleSpace(std::list<Entity*>& entityList, const std::string& typeName, int whichModuleSpace) const { return GetAllOfType(entityList, typeName, whichModuleSpace, true); }
#pragma endregion

#pragma region Preset-by-Group Getters
		/// <summary>
		/// Adds to a list all previously read in (defined) Entities which are associated with a specific group.
		/// </summary>
		/// <param name="entityList">Reference to a list which will get all matching Entities added to it. Ownership of the list or the Entities placed in it are NOT transferred!</param>
		/// <param name="groupName">The group to look for. "All" will look in all.</param>
		/// <param name="typeName">The name of the least common denominator type of the Entities you want. "All" will look at all types.</param>
		/// <param name="whichModule">Whether to only get those of one specific DataModule (0-n), or all (-1).</param>
		/// <returns>Whether any Entities were found and added to the list.</returns>
		bool GetAllOfGroup(std::list<Entity*>& entityList, const std::string& groupName, const std::string& typeName = "All", int whichModule = -1) const { return GetAllOfGroups(entityList, {groupName}, typeName, whichModule); }

		/// <summary>
		/// Adds to a list all previously read in (defined) Entities which are associated with a specific group, and only exist in a specific module space.
		/// </summary>
		/// <param name="entityList">Reference to a list which will get all matching Entities added to it. Ownership of the list or the Entities placed in it are NOT transferred!</param>
		/// <param name="groupName">The group to look for. "All" will look in all.</param>
		/// <param name="typeName">The name of the least common denominator type of the Entities to get. "All" will look at all types.</param>
		/// <param name="whichModuleSpace">Which module to get the instances for, in addition to all groups in official modules. -1 means get ALL groups ever registered.</param>
		/// <returns>Whether any Entities were found and added to the list.</returns>
		bool GetAllOfGroupInModuleSpace(std::list<Entity*>& entityList, const std::string& groupName, const std::string& typeName, int whichModuleSpace) const { return GetAllOfGroups(entityList, {groupName}, typeName, whichModuleSpace, true); }

		/// <summary>
		/// Adds to a list all previously read in (defined) Entities which are associated with several specific groups.
		/// </summary>
		/// <param name="entityList">Reference to a list which will get all matching Entities added to it. Ownership of the list or the Entities placed in it are NOT transferred!</param>
		/// <param name="groupNames">The groups to look for. "All" will look in all.</param>
		/// <param name="typeName">The name of the least common denominator type of the Entities you want. "All" will look at all types.</param>
		/// <param name="whichModule">Whether to only get those of one specific DataModule (0-n), or all (-1).</param>
		/// <param name="moduleSpace">Whether to get all instances in official modules, in addition to the specified module.</param>
		/// <returns>Whether any Entities were found and added to the list.</returns>
		bool GetAllOfGroups(std::list<Entity*>& entityList, const std::vector<std::string>& groupNames, const std::string& typeName = "All", int whichModule = -1, bool moduleSpace = false) const;

		/// <summary>
		/// Adds to a list all previously read in (defined) Entities which are not associated with a specific group.
		/// </summary>
		/// <param name="entityList">Reference to a list which will get all matching Entities added to it. Ownership of the list or the Entities placed in it are NOT transferred!</param>
		/// <param name="groupName">The group to exclude.</param>
		/// <param name="typeName">The name of the least common denominator type of the Entities you want. "All" will look at all types.</param>
		/// <param name="whichModule">Whether to only get those of one specific DataModule (0-n), or all (-1).</param>
		/// <returns>Whether any Entities were found and added to the list.</returns>
		bool GetAllNotOfGroup(std::list<Entity*>& entityList, const std::string& groupName, const std::string& typeName = "All", int whichModule = -1) const { return GetAllNotOfGroups(entityList, {groupName}, typeName, whichModule); }

		/// <summary>
		/// Adds to a list all previously read in (defined) Entities which are not associated with several specific groups.
		/// </summary>
		/// <param name="entityList">Reference to a list which will get all matching Entities added to it. Ownership of the list or the Entities placed in it are NOT transferred!</param>
		/// <param name="groupNames">The groups to exclude.</param>
		/// <param name="typeName">The name of the least common denominator type of the Entities you want. "All" will look at all types.</param>
		/// <param name="whichModule">Whether to only get those of one specific DataModule (0-n), or all (-1).</param>
		/// <returns>Whether any Entities were found and added to the list.</returns>
		bool GetAllNotOfGroups(std::list<Entity*>& entityList, const std::vector<std::string>& groupNames, const std::string& typeName = "All", int whichModule = -1) const;
#pragma endregion

#pragma region Random Preset Getters
		/// <summary>
		/// Returns a previously read in (defined) Entity which is randomly selected from a specific group.
		/// </summary>
		/// <param name="groupName">The group to randomly select an Entity from. "All" will look in all.</param>
		/// <param name="typeName">The name of the least common denominator type of the Entity to get. "All" will look at all types.</param>
		/// <param name="whichModule">Whether to only get those of one specific DataModule (0-n), or all (-1).</param>
		/// <param name="moduleSpace">Whether to get all instances in official modules, in addition to the group in the specified module.</param>
		/// <returns>The Entity preset that was randomly selected. Ownership is NOT transferred!</returns>
		Entity* GetRandomOfGroup(const std::string& groupName, const std::string& typeName = "All", int whichModule = -1, bool moduleSpace = false);

		/// <summary>
		/// Returns a previously read in (defined) Entity which is associated with a specific group, randomly selected and only exist in a specific module space.
		/// </summary>
		/// <param name="groupName">The group to randomly select an Entity from. "All" will look in all.</param>
		/// <param name="typeName">The name of the least common denominator type of the Entity to get. "All" will look at all types.</param>
		/// <param name="whichModuleSpace">Which module to get the instances for, in addition to all groups in official modules. -1 means get ALL groups ever registered.</param>
		/// <returns>The Entity preset that was randomly selected. Ownership is NOT transferred!</returns>
		Entity* GetRandomOfGroupInModuleSpace(const std::string& groupName, const std::string& typeName, int whichModuleSpace) { return GetRandomOfGroup(groupName, typeName, whichModuleSpace, true); }

		/// <summary>
		/// Returns a previously read in (defined) Entity which is randomly selected from a specific group only if it belongs to some tech.
		/// </summary>
		/// <param name="groupName">The group to randomly select an Entity from. "All" will look in all.</param>
		/// <param name="typeName">The name of the least common denominator type of the Entity to get. "All" will look at all types.</param>
		/// <param name="whichModule">Whether to only get those of one specific DataModule (0-n), or all (-1).</param>
		/// <returns>The Entity preset that was randomly selected. Ownership is NOT transferred!</returns>
		Entity* GetRandomBuyableOfGroupFromTech(const std::string& groupName, const std::string& typeName = "All", int whichModule = -1);
#pragma endregion

#pragma region Loadout Getters
		/// <summary>
		/// Creates and returns an Actor defined in the specified Loadout.
		/// </summary>
		/// <param name="loadoutName">Loadout preset name.</param>
		/// <param name="whichModule">The module ID in which the Loadout is defined.</param>
		/// <param name="spawnDeliveryCraft">Whether or not spawn delivery craft defined for that Loadout.</param>
		/// <returns>Created Actor if matching loadout was found or nullptr. OWNERSHIP IS TRANSFERED!</returns>
		Actor* GetLoadout(const std::string& loadoutName, int whichModule, bool spawnDeliveryCraft);

		/// <summary>
		/// Creates and returns an Actor defined in the specified Loadout.
		/// </summary>
		/// <param name="loadoutName">Loadout preset name.</param>
		/// <param name="whichModule">The module name in which the Loadout is defined.</param>
		/// <param name="spawnDeliveryCraft">Whether or not spawn delivery craft defined for that Loadout.</param>
		/// <returns>Created Actor if matching loadout was found or nullptr. OWNERSHIP IS TRANSFERED!</returns>
		Actor* GetLoadout(const std::string& loadoutName, const std::string& moduleName, bool spawnDeliveryCraft);
#pragma endregion

#pragma region Preset Reload Handling
		/// <summary>
		/// Reloads all scripted Entity Presets with the latest version of their respective script files.
		/// </summary>
		void ReloadAllScripts() const;

		/// <summary>
		/// Reloads an Entity preset and all related presets with the latest version of their respective files.
		/// Stores the passed in Entity preset info for later re-use in PresetMan::QuickReloadEntityPreset.
		/// </summary>
		/// <param name="presetName">The name of the preset to reload.</param>
		/// <param name="className">The type of the preset to reload, to avoid any ambiguity.</param>
		/// <param name="dataModule">The DataModule the preset to reload is defined in.</param>
		/// <param name="storeReloadedPresetDataForQuickReloading">Whether or not to store the reloaded entity preset data for quick reloading.</param>
		/// <returns>Whether reloading the preset was successful.</returns>
		bool ReloadEntityPreset(const std::string& presetName, const std::string& className, const std::string& dataModule, bool storeReloadedPresetDataForQuickReloading = true);

		/// <summary>
		/// Reloads the previously reloaded Entity preset and all related presets with the latest version of their respective files.
		/// </summary>
		/// <returns>Whether reloading the preset was successful.</returns>
		bool QuickReloadEntityPreset();

		/// <summary>
		/// Gets whether or not ReloadEntityPreset was called this update.
		/// </summary>
		/// <returns>Whether or not ReloadEntityPreset was called this update.</returns>
		bool GetReloadEntityPresetCalledThisUpdate() const { return m_ReloadEntityPresetCalledThisUpdate; }

		/// <summary>
		/// Resets whether or not ReloadEntityPreset was called this update.
		/// </summary>
		void ClearReloadEntityPresetCalledThisUpdate() { m_ReloadEntityPresetCalledThisUpdate = false; }
#pragma endregion

#pragma region Concrete Methods
		/// <summary>
		/// Adds a Material mapping local to a DataModule.
		/// This is used for when multiple DataModules are loading conflicting Materials, and need to resolve the conflicts by mapping their materials to ID's different than those specified in the data files.
		/// </summary>
		/// <param name="fromID">The material ID to map from.</param>
		/// <param name="toID">The material ID to map to.</param>
		/// <param name="whichModule">The ID of the  DataModule we are making the mapping for. This should be a non-official module as mapping shouldn't be needed in the official modules.</param>
		/// <returns>Whether this created a new mapping which didn't override a previous material mapping.</returns>
		bool AddMaterialMapping(uint8_t fromID, uint8_t toID, int whichModule) const;

		/// <summary>
		/// Adds an Entity instance's pointer and name associations to the internal list of already read in Entities. Ownership is NOT transferred!
		/// If there already is an instance defined, nothing happens. If there is not, a clone is made of the passed-in Entity and added to the library.
		/// </summary>
		/// <param name="entityToAdd">A pointer to the Entity derived instance to add. It should be created from a Reader. Ownership is NOT transferred!</param>
		/// <param name="whichModule">Which module to add the entity to.</param>
		/// <param name="overwriteSame">Whether to overwrite if an instance of the EXACT same TYPE and name was found. If one of the same name but not the exact type, false is returned regardless and nothing will have been added.</param>
		/// <param name="readFromFile">The file this instance was read from, or where it should be written. If "Same" is passed as the file path read from, an overwritten instance will keep the old one's file location entry.</param>
		/// <returns>Whether or not a copy of the passed-in instance was successfully inserted into the module. False will be returned if there already was an instance of that class and instance name inserted previously, unless overwritten.</returns>
		bool AddEntityPreset(Entity* entityToAdd, int whichModule = 0, bool overwriteSame = false, const std::string& readFromFile = "Same") const;

		/// <summary>
		/// Reads a preset of an Entity and tries to add it to the list of read-in instances. Regardless of whether there is a name collision, the read-in preset will be returned, ownership TRANSFERRED!
		/// </summary>
		/// <param name="reader">The Reader which is about to read in a preset.</param>
		/// <returns>A pointer to the Entity preset read in. 0 if there was an error, or the instance name was 'None'. Ownership IS transferred!</returns>
		Entity* ReadReflectedPreset(Reader& reader);
#pragma endregion

	private:
		bool m_ReloadEntityPresetCalledThisUpdate; //!< A flag for whether or not ReloadEntityPreset was called this update.
		std::array<std::string, 3> m_LastReloadedEntityPresetInfo; //!< Array storing the last reloaded Entity preset info (ClassName, PresetName and DataModule). Used for quick reloading via key combination.

		/// <summary>
		/// Clears all the member variables of this PresetMan, effectively resetting the members of this abstraction level only.
		/// </summary>
		void Clear();

		// Disallow the use of some implicit methods.
		PresetMan(const PresetMan& reference) = delete;
		PresetMan& operator=(const PresetMan& rhs) = delete;
	};
} // namespace RTE
#endif
