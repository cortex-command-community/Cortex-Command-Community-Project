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

		/// <summary>
		/// Makes the PresetMan object ready for use.
		/// </summary>
		/// <returns>An error return value signaling success or any particular failure. Anything below 0 is an error signal.</returns>
		int Initialize() { return 0; }
#pragma endregion

#pragma region Destruction
		/// <summary>
		/// Destructor method used to clean up a PresetMan object before deletion from system memory.
		/// </summary>
		~PresetMan() { Destroy(); }

		/// <summary>
		/// Resets the entire PresetMan, including its inherited members, to their default settings or values.
		/// </summary>
		void Reset() { Clear(); }

		/// <summary>
		/// Destroys and resets (through Clear()) the PresetMan object.
		/// </summary>
		void Destroy() { Clear(); }
#pragma endregion


		//////////////////////////////////////////////////////////////////////////////////////////
		// Method:          AddEntityPreset
		//////////////////////////////////////////////////////////////////////////////////////////
		// Description:     Adds an Entity instance's pointer and name associations to the
		//                  internal list of already read in Entity:s. Ownership is NOT transferred!
		//                  If there already is an instance defined, nothing happens. If there
		//                  is not, a clone is made of the passed-in Entity and added to the library.
		// Arguments:       A pointer to the Entity derived instance to add. It should be created
		//                  from a Reader. Ownership is NOT transferred!
		//                  Which module to add the entity to.
		//                  Whether to overwrite if an instance of the EXACT same TYPE and name
		//                  was found. If one of the same name but not the exact type, false
		//                  is returned regardless and nothing will have been added.
		//                  The file this instance was read from, or where it should be written.
		//                  If "Same" is passed as the file path read from, an overwritten instance
		//                  will keep the old one's file location entry.
		// Return value:    Whether or not a copy of the passed-in instance was successfully inserted
		//                  into the module. False will be returned if there already was an instance
		//                  of that class and instance name inserted previously, unless overwritten.
		bool AddEntityPreset(Entity *pEntToAdd, int whichModule = 0, bool overwriteSame = false, std::string readFromFile = "Same");


		//////////////////////////////////////////////////////////////////////////////////////////
		// Method:          GetEntityPreset
		//////////////////////////////////////////////////////////////////////////////////////////
		// Description:     Gets a previously read in (defined) Entity, by type and instance name.
		// Arguments:       The type name of the derived Entity. Ownership is NOT transferred!
		//                  The instance name of the derived Entity instance.
		//                  Which module to try to get the entity from. If it's not found there,
		//                  the official modules will be searched also. -1 means search ALL modules!
		// Return value:    A pointer to the requested Entity instance. 0 if no Entity with that
		//                  derived type or instance name was found. Ownership is NOT transferred!
		const Entity * GetEntityPreset(std::string type, std::string preset, int whichModule = -1);
		// Helper for passing in string module name instead of ID
		const Entity * GetEntityPreset(std::string type, std::string preset, std::string module);


		//////////////////////////////////////////////////////////////////////////////////////////
		// Method:          GetEntityPreset
		//////////////////////////////////////////////////////////////////////////////////////////
		// Description:     Reads an instance of an Entity that will be used as preset
		//                  to later on be used to generate more instances of the same state.
		//                  Will check if there was already one of the same class and instance
		//                  name read in previously, and will return that one if found, or
		//                  add the newly read in one to this PresetMan's list if not found to
		//                  exist there previously. Ownership is NOT transferred!
		// Arguments:       The Reader which is about to read in an instance reference. It'll make
		//                  this look in the same module as it's reading from.
		// Return value:    A const pointer to the Entity instance read in. 0 if there was an
		//                  error, or the instance name was 'None'. Ownership is NOT transferred!
		const Entity * GetEntityPreset(Reader &reader);


		//////////////////////////////////////////////////////////////////////////////////////////
		// Method:          ReadReflectedPreset
		//////////////////////////////////////////////////////////////////////////////////////////
		// Description:     Reads a preset of an Entity and tries to add it to the list of
		//                  read-in instances. Regardless of whether there is a name collision,
		//                  the read-in preset will be returned, ownership TRANSFERRED!
		// Arguments:       The Reader which is about to read in a preset.
		// Return value:    A pointer to the Entity preset read in. 0 if there was an
		//                  error, or the instance name was 'None'. Ownership IS transferred!
		Entity * ReadReflectedPreset(Reader &reader);


		//////////////////////////////////////////////////////////////////////////////////////////
		// Method:          GetAllOfType
		//////////////////////////////////////////////////////////////////////////////////////////
		// Description:     Adds to a list all previously read in (defined) Entitys, by type.
		// Arguments:       Reference to a list which will get all matching Entity:s added to it.
		//                  Ownership of the list or the Entitys placed in it are NOT transferred!
		//                  The type name of the Entitys you want.
		//                  Whether to only get those of one specific DataModule (0-n), or all (-1).
		// Return value:    Whether any Entity:s were found and added to the list.
		bool GetAllOfType(std::list<Entity *> &entityList, std::string type, int whichModule = -1);


		//////////////////////////////////////////////////////////////////////////////////////////
		// Method:          GetAllOfTypeInModuleSpace
		//////////////////////////////////////////////////////////////////////////////////////////
		// Description:     Adds to a list all previously read in (defined) Entitys which are
		//                  of a specific type, and only exist in a specific module space.
		// Arguments:       Reference to a list which will get all matching Entity:s added to it.
		//                  Ownership of the list or the Entitys placed in it are NOT transferred!
		//                  The type name of the Entitys you want.
		//                  Which module to get the instances for, in addition to all groups in
		//                  official modules loaded earlier than the one specified here. -1 means
		//                  get ALL groups ever reg'd.
		// Return value:    Whether any Entity:s were found and added to the list.
		bool GetAllOfTypeInModuleSpace(std::list<Entity *> &entityList, std::string type, int whichModuleSpace);


		/// <summary>
		/// Adds to a list all previously read in (defined) Entities which are associated with a specific group.
		/// </summary>
		/// <param name="entityList">Reference to a list which will get all matching Entities added to it. Ownership of the list or the Entities placed in it are NOT transferred!</param>
		/// <param name="group">The group to look for. "All" will look in all.</param>
		/// <param name="type">The name of the least common denominator type of the Entities you want. "All" will look at all types.</param>
		/// <param name="whichModule">Whether to only get those of one specific DataModule (0-n), or all (-1).</param>
		/// <returns>Whether any Entities were found and added to the list.</returns>
		bool GetAllOfGroup(std::list<Entity *> &entityList, const std::string &group, const std::string &type = "All", int whichModule = -1) { return GetAllOfGroups(entityList, { group }, type, whichModule); }

		/// <summary>
		/// Adds to a list all previously read in (defined) Entities which are associated with several specific groups.
		/// </summary>
		/// <param name="entityList">Reference to a list which will get all matching Entities added to it. Ownership of the list or the Entities placed in it are NOT transferred!</param>
		/// <param name="groups">The groups to look for. "All" will look in all.</param>
		/// <param name="type">The name of the least common denominator type of the Entities you want. "All" will look at all types.</param>
		/// <param name="whichModule">Whether to only get those of one specific DataModule (0-n), or all (-1).</param>
		/// <returns>Whether any Entities were found and added to the list.</returns>
		bool GetAllOfGroups(std::list<Entity *> &entityList, const std::vector<std::string> &groups, const std::string &type = "All", int whichModule = -1);

		/// <summary>
		/// Adds to a list all previously read in (defined) Entities which are not associated with a specific group.
		/// </summary>
		/// <param name="entityList">Reference to a list which will get all matching Entities added to it. Ownership of the list or the Entities placed in it are NOT transferred!</param>
		/// <param name="group">The group to exclude.</param>
		/// <param name="type">The name of the least common denominator type of the Entities you want. "All" will look at all types.</param>
		/// <param name="whichModule">Whether to only get those of one specific DataModule (0-n), or all (-1).</param>
		/// <returns>Whether any Entities were found and added to the list.</returns>
		bool GetAllNotOfGroup(std::list<Entity *> &entityList, const std::string &group, const std::string &type = "All", int whichModule = -1) { return GetAllNotOfGroups(entityList, { group }, type, whichModule); }

		/// <summary>
		/// Adds to a list all previously read in (defined) Entities which are not associated with several specific groups.
		/// </summary>
		/// <param name="entityList">Reference to a list which will get all matching Entities added to it. Ownership of the list or the Entities placed in it are NOT transferred!</param>
		/// <param name="groups">The groups to exclude.</param>
		/// <param name="type">The name of the least common denominator type of the Entities you want. "All" will look at all types.</param>
		/// <param name="whichModule">Whether to only get those of one specific DataModule (0-n), or all (-1).</param>
		/// <returns>Whether any Entities were found and added to the list.</returns>
		bool GetAllNotOfGroups(std::list<Entity *> &entityList, const std::vector<std::string> &groups, const std::string &type = "All", int whichModule = -1);


		//////////////////////////////////////////////////////////////////////////////////////////
		// Method:          GetRandomOfGroup
		//////////////////////////////////////////////////////////////////////////////////////////
		// Description:     Returns a previously read in (defined) Entity which is randomly
		//                  selected from a specific group.
		// Arguments:       The group to randomly select an Entity from. "All" will look in all.
		//                  The name of the least common denominator type of the Entitys you want.
		//                  "All" will look at all types.
		//                  Whether to only get those of one specific DataModule (0-n), or all (-1).
		// Return value:    The Entity preset that was randomly selected. Ownership is NOT transferred!
		Entity * GetRandomOfGroup(std::string group, std::string type = "All", int whichModule = -1);


		//////////////////////////////////////////////////////////////////////////////////////////
		// Method:          GetRandomOfGroupFromTech
		//////////////////////////////////////////////////////////////////////////////////////////
		// Description:     Returns a previously read in (defined) Entity which is randomly
		//                  selected from a specific group only if it belongs to some tech.
		// Arguments:       The group to randomly select an Entity from. "All" will look in all.
		//                  The name of the least common denominator type of the Entitys you want.
		//                  "All" will look at all types.
		//                  Whether to only get those of one specific DataModule (0-n), or all (-1)
		//					or all modules uncluding non-tech ones.
		// Return value:    The Entity preset that was randomly selected. Ownership is NOT transferred!
		Entity * GetRandomBuyableOfGroupFromTech(std::string group, std::string type = "All", int whichModule = -1);


		//////////////////////////////////////////////////////////////////////////////////////////
		// Method:          GetAllOfGroupInModuleSpace
		//////////////////////////////////////////////////////////////////////////////////////////
		// Description:     Adds to a list all previously read in (defined) Entitys which are
		//                  associated with a specific group, and only exist in a specific module
		//                  space.
		// Arguments:       Reference to a list which will get all matching Entity:s added to it.
		//                  Ownership of the list or the Entitys placed in it are NOT transferred!
		//                  The group to look for. "All" will look in all.
		//                  The name of the least common denominator type of the Entitys you want.
		//                  "All" will look at all types.
		//                  Which module to get the instances for, in addition to all groups in
		//                  official modules loaded earlier than the one specified here. -1 means
		//                  get ALL groups ever reg'd.
		// Return value:    Whether any Entity:s were found and added to the list.
		bool GetAllOfGroupInModuleSpace(std::list<Entity *> &entityList, std::string group, std::string type, int whichModuleSpace);


		//////////////////////////////////////////////////////////////////////////////////////////
		// Method:          GetRandomOfGroupInModuleSpace
		//////////////////////////////////////////////////////////////////////////////////////////
		// Description:     Returns a previously read in (defined) Entity which is associated with
		//                  a specific group, randomly selected and only exist in a specific module
		//                  space.
		// Arguments:       Ownership of the list or the Entitys placed in it are NOT transferred!
		//                  The group to randomly select from. "All" will look in all.
		//                  The name of the least common denominator type of the Entity:s you want.
		//                  "All" will look at all types.
		//                  Which module to get the instances for, in addition to all groups in
		//                  official modules loaded earlier than the one specified here. -1 means
		//                  get ALL groups ever reg'd.
		// Return value:    The randomly select preset, if any was found with thse search params.
		//                  Ownership is NOT transferred!
		Entity * GetRandomOfGroupInModuleSpace(std::string group, std::string type, int whichModuleSpace);


		//////////////////////////////////////////////////////////////////////////////////////////
		// Method:          GetEntityDataLocation
		//////////////////////////////////////////////////////////////////////////////////////////
		// Description:     Gets the data file path of a previously read in (defined) Entity.
		// Arguments:       The type name of the derived Entity. Ownership is NOT transferred!
		//                  The preset name of the derived Entity preset.
		//                  Which module to try to get the entity from. If it's not found there,
		//                  the official modules will be searched also.
		// Return value:    The file path of the data file that the specified Entity was read from.
		//                  If no Entity of that description was found, "" is returned.
		std::string GetEntityDataLocation(std::string type, std::string preset, int whichModule);


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
		bool ReloadEntityPreset(const std::string &presetName, const std::string &className, const std::string &dataModule, bool storeReloadedPresetDataForQuickReloading = true);

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


		//////////////////////////////////////////////////////////////////////////////////////////
		// Method:          AddMaterialMapping
		//////////////////////////////////////////////////////////////////////////////////////////
		// Description:     Adds a Material mapping local to a DataModule. This is used for when
		//                  multiple DataModule:s are loading conflicting Material:s, and need to
		//                  resolve the conflicts by mapping their materials to ID's different than
		//                  those specified in the data files.
		// Arguments:       The material ID to map from.
		//                  The material ID to map to.
		//                  The ID of the  DataModule we are making the mapping for. This should be
		//                  a non-official module as mapping shouldn't be needed in the official
		//                  modules.
		// Return value:    Whether this created a new mapping which didn't override a previous
		//                  material mapping.
		bool AddMaterialMapping(int fromID, int toID, int whichModule);


		//////////////////////////////////////////////////////////////////////////////////////////
		// Method:          RegisterGroup
		//////////////////////////////////////////////////////////////////////////////////////////
		// Description:     Registers the existence of an Entity group, and in which module.
		// Arguments:       The group to register.
		//                  The ID of the module in which at least one entity of this group can be
		//                  found.
		//                  global register.
		// Return value:    None.
		void RegisterGroup(std::string newGroup, int whichModule);


		//////////////////////////////////////////////////////////////////////////////////////////
		// Method:          GetGroups
		//////////////////////////////////////////////////////////////////////////////////////////
		// Description:     Fills out a list with all groups registered in a specific module.
		// Arguments:       The list that all found groups will be ADDED to. OWNERSHIP IS NOT TRANSFERRED!
		//                  Which module to get the groups for. -1 means get ALL groups ever reg'd.
		//                  Pass a type name here and only groups with entitys of that type will be
		//                  be included. "All" means don't consider what types are in the groups.
		// Return value:    Whether any groups were found and thus added to the list.
		bool GetGroups(std::list<std::string> &groupList, int whichModule = -1, std::string withType = "All") const;


		//////////////////////////////////////////////////////////////////////////////////////////
		// Method:          GetModuleSpaceGroups
		//////////////////////////////////////////////////////////////////////////////////////////
		// Description:     Fills out a list with all groups registered in all official modules,
		//                  PLUS a specific non-official module as well.
		// Arguments:       The list that all found groups will be ADDED to. OWNERSHIP IS NOT TRANSFERRED!
		//                  Which module to get the groups for, in addition to all groups in
		//                  official modules loaded earlier than the one specified here. -1 means
		//                  get ALL groups ever reg'd.
		//                  Pass a type name here and only groups with entitys of that type will be
		//                  be included. "All" means don't consider what types are in the groups.
		// Return value:    Whether any groups were found and thus added to the list.
		bool GetModuleSpaceGroups(std::list<std::string> &groupList, int whichModule, std::string withType = "All") const;


		//////////////////////////////////////////////////////////////////////////////////////////
		// Method:          GetLoadout
		//////////////////////////////////////////////////////////////////////////////////////////
		// Description:     Creates and returns actor defined in the specified loadout.
		// Arguments:       Loadout preset name, module name, whether or not spawn delivery craft defined for that loadout
		// Return value:    Created actor if matching loadout was found or 0. OWNERSHIP IS TRANSFERED.
		Actor * GetLoadout(std::string loadoutName, std::string module, bool spawnDropShip);


		//////////////////////////////////////////////////////////////////////////////////////////
		// Method:          GetLoadout
		//////////////////////////////////////////////////////////////////////////////////////////
		// Description:     Creates and returns actor defined in the specified loadout.
		// Arguments:       Loadout preset name, module id, whether or not spawn delivery craft defined for that loadout
		// Return value:    Created actor if matching loadout was found or 0. OWNERSHIP IS TRANSFERED.
		Actor * GetLoadout(std::string loadoutName, int moduleNumber, bool spawnDropShip);

	private:

		std::list<std::string> m_TotalGroupRegister; //!< List of all Entity groups ever registered, all uniques. This is just a handy total of all the groups registered in all the individual DataModules.

		bool m_ReloadEntityPresetCalledThisUpdate; //!< A flag for whether or not ReloadEntityPreset was called this update.
		std::array<std::string, 3> m_LastReloadedEntityPresetInfo; //!< Array storing the last reloaded Entity preset info (ClassName, PresetName and DataModule). Used for quick reloading via key combination.

		/// <summary>
		/// Clears all the member variables of this PresetMan, effectively resetting the members of this abstraction level only.
		/// </summary>
		void Clear();

		// Disallow the use of some implicit methods.
		PresetMan(const PresetMan &reference) = delete;
		PresetMan & operator=(const PresetMan &rhs) = delete;
	};
}
#endif