#pragma once

/// Header file for the PresetMan class.
/// @author Daniel Tabar
/// data@datarealms.com
/// http://www.datarealms.com
/// Inclusions of header files
#include "Singleton.h"
#include "Entity.h"

#include <array>
#include <list>
#include <map>
#include <string>
#include <vector>

#define g_PresetMan PresetMan::Instance()

namespace RTE {

	class Actor;
	class DataModule;

	/// The singleton manager of all presets of Entities in the RTE.
	/// The presets serve as a repository of Entity instances with specific, unique and initial runtime data.
	class PresetMan : public Singleton<PresetMan> {
		friend struct ManagerLuaBindings;

	public:
#pragma region Creation
		/// Constructor method used to instantiate a PresetMan entity in system
		/// memory. Create() should be called before using the entity.
		PresetMan();

		/// Makes the PresetMan entity ready for use.
		/// @return An error return value signaling sucess or any particular failure.
		/// Anything below 0 is an error signal.
		int Initialize() { return 0; }

		/// Resets the entire PresetMan, including its inherited members, to
		/// their default settings or values.
		void Reset() { Clear(); }
#pragma endregion

#pragma region Group Register Handling
		/// Registers the existence of an Entity group, and in which module.
		/// @param groupName The group to register.
		/// @param whichModule The ID of the module in which at least one Entity of this group can be found.
		void RegisterGroup(const std::string& groupName, int whichModule) const;

		/// Fills out a list with all groups registered in a specific module.
		/// @param groupList The list that all found groups will be added to. OWNERSHIP IS NOT TRANSFERRED!
		/// @param whichModule Which module to get the groups for. -1 means get ALL groups ever registered.
		/// @param withType Pass a type name here and only groups with Entities of that type will be included. "All" means don't consider what types are in the groups.
		/// @param moduleSpace Whether to get all groups in official modules, in addition to the specified module.
		/// @return Whether any groups were found and added to the list.
		bool GetGroups(std::list<std::string>& groupList, int whichModule = -1, const std::string& withType = "All", bool moduleSpace = false) const;

		/// Fills out a list with all groups registered in all official modules, PLUS a specific non-official module as well.
		/// @param groupList The list that all found groups will be added to. OWNERSHIP IS NOT TRANSFERRED!
		/// @param whichModule Which module to get the groups for, in addition to all groups in official modules. -1 means get ALL groups ever registered.
		/// @param withType Pass a type name here and only groups with Entities of that type will be included. "All" means don't consider what types are in the groups.
		/// @return Whether any groups were found and added to the list.
		bool GetModuleSpaceGroups(std::list<std::string>& groupList, int whichModuleSpace, const std::string& withType = "All") const { return GetGroups(groupList, whichModuleSpace, withType, true); }
#pragma endregion

#pragma region Preset Getters
		/// Reads an instance of an Entity that will be used as preset to later on be used to generate more instances of the same state. Ownership is NOT transferred!
		/// Will check if there was already one of the same class and instance name read in previously, and will return that one if found, or add the newly read in one to this PresetMan's list if not found to exist there previously.
		/// @param reader The Reader which is about to read in an instance reference. It'll make this look in the same module as it's reading from.
		/// @return A pointer to const of the Entity instance read in. nullptr if there was an error, or the instance name was 'None'. Ownership is NOT transferred!
		const Entity* GetEntityPreset(Reader& reader);

		/// Gets a previously read in (defined) Entity, by type and instance name. Ownership is NOT transferred!
		/// @param typeName The type name of the derived Entity.
		/// @param presetName The instance name of the derived Entity instance.
		/// @param whichModule Which module to try to get the entity from. If it's not found there, the official modules will be searched also. -1 means search ALL modules!
		/// @return A pointer to const of the requested Entity instance. nullptr if no Entity with that derived type or instance name was found. Ownership is NOT transferred!
		const Entity* GetEntityPreset(const std::string& typeName, std::string presetName, int whichModule = -1) const;

		/// Gets a previously read in (defined) Entity, by type and instance name. Ownership is NOT transferred!
		/// @param typeName The type name of the derived Entity.
		/// @param presetName The instance name of the derived Entity instance.
		/// @param whichModule Which module to try to get the entity from. If it's not found there, the official modules will be searched also.
		/// @return A pointer to const of the requested Entity instance. nullptr if no Entity with that derived type or instance name was found. Ownership is NOT transferred!
		const Entity* GetEntityPreset(const std::string& typeName, const std::string& presetName, const std::string& whichModule) const;

		/// Gets the data file path of a previously read in (defined) Entity. Ownership is NOT transferred!
		/// @param typeName The type name of the derived Entity.
		/// @param presetName The preset name of the derived Entity preset.
		/// @param whichModule Which module to try to get the entity from. If it's not found there, the official modules will be searched also.
		/// @return The file path of the data file that the specified Entity was read from. If no Entity of that description was found, an empty string is returned.
		std::string GetEntityDataLocation(const std::string& typeName, const std::string& presetName, int whichModule) const;
#pragma endregion

#pragma region Preset-by-Type Getters
		/// Adds to a list all previously read in (defined) Entities, by type.
		/// @param entityList Reference to a list which will get all matching Entities added to it. Ownership of the list or the Entities placed in it are NOT transferred!
		/// @param typeName The type name of the Entities to get.
		/// @param whichModule Whether to only get those of one specific DataModule (0-n), or all (-1).
		/// @param moduleSpace Whether to get all instances in official modules, in addition to the specified module.
		/// @return Whether any Entities were found and added to the list.
		bool GetAllOfType(std::list<Entity*>& entityList, const std::string& typeName, int whichModule = -1, bool moduleSpace = false) const;

		/// Adds to a list all previously read in (defined) Entities which are of a specific type, and only exist in a specific module space.
		/// @param entityList Reference to a list which will get all matching Entities added to it. Ownership of the list or the Entities placed in it are NOT transferred!
		/// @param typeName The type name of the Entities to get.
		/// @param whichModuleSpace Which module to get the instances for, in addition to all groups in official modules. -1 means get ALL groups ever registered.
		/// @return Whether any Entities were found and added to the list.
		bool GetAllOfTypeInModuleSpace(std::list<Entity*>& entityList, const std::string& typeName, int whichModuleSpace) const { return GetAllOfType(entityList, typeName, whichModuleSpace, true); }
#pragma endregion

#pragma region Preset-by-Group Getters
		/// Adds to a list all previously read in (defined) Entities which are associated with a specific group.
		/// @param entityList Reference to a list which will get all matching Entities added to it. Ownership of the list or the Entities placed in it are NOT transferred!
		/// @param groupName The group to look for. "All" will look in all.
		/// @param typeName The name of the least common denominator type of the Entities you want. "All" will look at all types.
		/// @param whichModule Whether to only get those of one specific DataModule (0-n), or all (-1).
		/// @return Whether any Entities were found and added to the list.
		bool GetAllOfGroup(std::list<Entity*>& entityList, const std::string& groupName, const std::string& typeName = "All", int whichModule = -1) const { return GetAllOfGroups(entityList, {groupName}, typeName, whichModule); }

		/// Adds to a list all previously read in (defined) Entities which are associated with a specific group, and only exist in a specific module space.
		/// @param entityList Reference to a list which will get all matching Entities added to it. Ownership of the list or the Entities placed in it are NOT transferred!
		/// @param groupName The group to look for. "All" will look in all.
		/// @param typeName The name of the least common denominator type of the Entities to get. "All" will look at all types.
		/// @param whichModuleSpace Which module to get the instances for, in addition to all groups in official modules. -1 means get ALL groups ever registered.
		/// @return Whether any Entities were found and added to the list.
		bool GetAllOfGroupInModuleSpace(std::list<Entity*>& entityList, const std::string& groupName, const std::string& typeName, int whichModuleSpace) const { return GetAllOfGroups(entityList, {groupName}, typeName, whichModuleSpace, true); }

		/// Adds to a list all previously read in (defined) Entities which are associated with several specific groups.
		/// @param entityList Reference to a list which will get all matching Entities added to it. Ownership of the list or the Entities placed in it are NOT transferred!
		/// @param groupNames The groups to look for. "All" will look in all.
		/// @param typeName The name of the least common denominator type of the Entities you want. "All" will look at all types.
		/// @param whichModule Whether to only get those of one specific DataModule (0-n), or all (-1).
		/// @param moduleSpace Whether to get all instances in official modules, in addition to the specified module.
		/// @return Whether any Entities were found and added to the list.
		bool GetAllOfGroups(std::list<Entity*>& entityList, const std::vector<std::string>& groupNames, const std::string& typeName = "All", int whichModule = -1, bool moduleSpace = false) const;

		/// Adds to a list all previously read in (defined) Entities which are not associated with a specific group.
		/// @param entityList Reference to a list which will get all matching Entities added to it. Ownership of the list or the Entities placed in it are NOT transferred!
		/// @param groupName The group to exclude.
		/// @param typeName The name of the least common denominator type of the Entities you want. "All" will look at all types.
		/// @param whichModule Whether to only get those of one specific DataModule (0-n), or all (-1).
		/// @return Whether any Entities were found and added to the list.
		bool GetAllNotOfGroup(std::list<Entity*>& entityList, const std::string& groupName, const std::string& typeName = "All", int whichModule = -1) const { return GetAllNotOfGroups(entityList, {groupName}, typeName, whichModule); }

		/// Adds to a list all previously read in (defined) Entities which are not associated with several specific groups.
		/// @param entityList Reference to a list which will get all matching Entities added to it. Ownership of the list or the Entities placed in it are NOT transferred!
		/// @param groupNames The groups to exclude.
		/// @param typeName The name of the least common denominator type of the Entities you want. "All" will look at all types.
		/// @param whichModule Whether to only get those of one specific DataModule (0-n), or all (-1).
		/// @return Whether any Entities were found and added to the list.
		bool GetAllNotOfGroups(std::list<Entity*>& entityList, const std::vector<std::string>& groupNames, const std::string& typeName = "All", int whichModule = -1) const;
#pragma endregion

#pragma region Random Preset Getters
		/// Returns a previously read in (defined) Entity which is randomly selected from a specific group.
		/// @param groupName The group to randomly select an Entity from. "All" will look in all.
		/// @param typeName The name of the least common denominator type of the Entity to get. "All" will look at all types.
		/// @param whichModule Whether to only get those of one specific DataModule (0-n), or all (-1).
		/// @param moduleSpace Whether to get all instances in official modules, in addition to the group in the specified module.
		/// @return The Entity preset that was randomly selected. Ownership is NOT transferred!
		Entity* GetRandomOfGroup(const std::string& groupName, const std::string& typeName = "All", int whichModule = -1, bool moduleSpace = false);

		/// Returns a previously read in (defined) Entity which is associated with a specific group, randomly selected and only exist in a specific module space.
		/// @param groupName The group to randomly select an Entity from. "All" will look in all.
		/// @param typeName The name of the least common denominator type of the Entity to get. "All" will look at all types.
		/// @param whichModuleSpace Which module to get the instances for, in addition to all groups in official modules. -1 means get ALL groups ever registered.
		/// @return The Entity preset that was randomly selected. Ownership is NOT transferred!
		Entity* GetRandomOfGroupInModuleSpace(const std::string& groupName, const std::string& typeName, int whichModuleSpace) { return GetRandomOfGroup(groupName, typeName, whichModuleSpace, true); }

		/// Returns a previously read in (defined) Entity which is randomly selected from a specific group only if it belongs to some tech.
		/// @param groupName The group to randomly select an Entity from. "All" will look in all.
		/// @param typeName The name of the least common denominator type of the Entity to get. "All" will look at all types.
		/// @param whichModule Whether to only get those of one specific DataModule (0-n), or all (-1).
		/// @return The Entity preset that was randomly selected. Ownership is NOT transferred!
		Entity* GetRandomBuyableOfGroupFromTech(const std::string& groupName, const std::string& typeName = "All", int whichModule = -1);
#pragma endregion

#pragma region Loadout Getters
		/// Creates and returns an Actor defined in the specified Loadout.
		/// @param loadoutName Loadout preset name.
		/// @param whichModule The module ID in which the Loadout is defined.
		/// @param spawnDeliveryCraft Whether or not spawn delivery craft defined for that Loadout.
		/// @return Created Actor if matching loadout was found or nullptr. OWNERSHIP IS TRANSFERED!
		Actor* GetLoadout(const std::string& loadoutName, int whichModule, bool spawnDeliveryCraft);

		/// Creates and returns an Actor defined in the specified Loadout.
		/// @param loadoutName Loadout preset name.
		/// @param whichModule The module name in which the Loadout is defined.
		/// @param spawnDeliveryCraft Whether or not spawn delivery craft defined for that Loadout.
		/// @return Created Actor if matching loadout was found or nullptr. OWNERSHIP IS TRANSFERED!
		Actor* GetLoadout(const std::string& loadoutName, const std::string& moduleName, bool spawnDeliveryCraft);
#pragma endregion

#pragma region Preset Reload Handling
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
#pragma endregion

#pragma region Concrete Methods
		/// Adds a Material mapping local to a DataModule.
		/// This is used for when multiple DataModules are loading conflicting Materials, and need to resolve the conflicts by mapping their materials to ID's different than those specified in the data files.
		/// @param fromID The material ID to map from.
		/// @param toID The material ID to map to.
		/// @param whichModule The ID of the  DataModule we are making the mapping for. This should be a non-official module as mapping shouldn't be needed in the official modules.
		/// @return Whether this created a new mapping which didn't override a previous material mapping.
		bool AddMaterialMapping(uint8_t fromID, uint8_t toID, int whichModule) const;

		/// Adds an Entity instance's pointer and name associations to the internal list of already read in Entities. Ownership is NOT transferred!
		/// If there already is an instance defined, nothing happens. If there is not, a clone is made of the passed-in Entity and added to the library.
		/// @param entityToAdd A pointer to the Entity derived instance to add. It should be created from a Reader. Ownership is NOT transferred!
		/// @param whichModule Which module to add the entity to.
		/// @param overwriteSame Whether to overwrite if an instance of the EXACT same TYPE and name was found. If one of the same name but not the exact type, false is returned regardless and nothing will have been added.
		/// @param readFromFile The file this instance was read from, or where it should be written. If "Same" is passed as the file path read from, an overwritten instance will keep the old one's file location entry.
		/// @return Whether or not a copy of the passed-in instance was successfully inserted into the module. False will be returned if there already was an instance of that class and instance name inserted previously, unless overwritten.
		bool AddEntityPreset(Entity* entityToAdd, int whichModule = 0, bool overwriteSame = false, const std::string& readFromFile = "Same") const;

		/// Reads a preset of an Entity and tries to add it to the list of read-in instances. Regardless of whether there is a name collision, the read-in preset will be returned, ownership TRANSFERRED!
		/// @param reader The Reader which is about to read in a preset.
		/// @return A pointer to the Entity preset read in. 0 if there was an error, or the instance name was 'None'. Ownership IS transferred!
		Entity* ReadReflectedPreset(Reader& reader);
#pragma endregion

	private:
		bool m_ReloadEntityPresetCalledThisUpdate; //!< A flag for whether or not ReloadEntityPreset was called this update.
		std::array<std::string, 3> m_LastReloadedEntityPresetInfo; //!< Array storing the last reloaded Entity preset info (ClassName, PresetName and DataModule). Used for quick reloading via key combination.

		/// Clears all the member variables of this PresetMan, effectively resetting the members of this abstraction level only.
		void Clear();

		// Disallow the use of some implicit methods.
		PresetMan(const PresetMan& reference) = delete;
		PresetMan& operator=(const PresetMan& rhs) = delete;
	};
} // namespace RTE
