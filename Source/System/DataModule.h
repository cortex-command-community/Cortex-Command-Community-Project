#pragma once

#include "ContentFile.h"
#include "Constants.h"

#include <array>
#include <list>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

// struct DATAFILE; // DataFile loading not implemented.
struct BITMAP;

namespace version {
	class Semver200_version;
}

namespace RTE {

	class Entity;

	/// A representation of a DataModule containing zero or many Material, Effect, Ammo, Device, Actor, or Scene definitions.
	class DataModule : public Serializable {
		friend struct SystemLuaBindings;

	public:
		SerializableClassNameGetter;
		SerializableOverrideMethods;

		/// Struct that holds data about the custom BuyMenu/ObjectPicker theme of this DataModule.
		/// Themes are used when a DataModule is considered a faction and is selected to be played as in an Activity.
		struct BuyMenuTheme {
			std::string SkinFilePath = ""; //!< Path to the custom BuyMenu skin file.
			std::string BannerImagePath = ""; //!< Path to the custom BuyMenu banner image.
			std::string LogoImagePath = ""; //< Path to the custom BuyMenu logo.
			int BackgroundColorIndex = -1; //!< The custom BuyMenu background color.
		};

#pragma region Creation
		/// Constructor method used to instantiate a DataModule object in system memory. Create() should be called before using the object.
		DataModule();

		/// Constructor method used to instantiate a DataModule object in system memory, and also do a Create() in the same line.
		/// Create() should therefore not be called after using this constructor.
		/// @param moduleName A string defining the path to where the content file itself is located, either within the package file, or directly on the disk.
		/// @param progressCallback A function pointer to a function that will be called and sent a string with information about the progress of this DataModule's creation.
		DataModule(const std::string& moduleName, const ProgressCallback& progressCallback = nullptr);

		/// Makes the DataModule object ready for use. This needs to be called after PresetMan is created.
		/// This looks for an "index.ini" within the specified .rte directory and loads all the defined objects in that index file.
		/// @param moduleName A string defining the name of this DataModule, e.g. "MyModule.rte".
		/// @param progressCallback A function pointer to a function that will be called and sent a string with information about the progress of this DataModule's creation.
		/// @return An error return value signaling success or any particular failure. Anything below 0 is an error signal.
		int Create(const std::string& moduleName, const ProgressCallback& progressCallback = nullptr);

		/// Creates a new DataModule directory with "Index.ini" on disk to be used for userdata. Does NOT instantiate the newly created DataModule.
		/// @param moduleName File/folder name of the data module, e.g. "MyMod.rte".
		/// @param friendlyName Friendly name of the data module, e.g. "My Weapons Mod".
		/// @param scanFolderContents Whether module loader should scan for any .ini's inside module folder instead of loading files defined in IncludeFile only.
		/// @param ignoreMissingItems Whether module loader should ignore missing items in this module.
		/// @return Whether the DataModule was successfully created on disk.
		static bool CreateOnDiskAsUserdata(const std::string& moduleName, const std::string_view& friendlyName, bool scanFolderContents = false, bool ignoreMissingItems = false);
#pragma endregion

#pragma region Destruction
		/// Destructor method used to clean up a DataModule object before deletion from system memory.
		~DataModule() override;

		/// Destroys and resets (through Clear()) the DataModule object.
		void Destroy();

		/// Resets the entire DataModule, including its inherited members, to their default settings or values.
		void Reset() override { Clear(); }
#pragma endregion

#pragma region INI Handling
		/// Read module specific properties from index.ini without processing IncludeFiles and loading the whole module.
		/// @param moduleName A string defining the name of this DataModule, e.g. "MyModule.rte".
		/// @param progressCallback A function pointer to a function that will be called and sent a string with information about the progress of this DataModule's creation.
		/// @return An error return value signaling success or any particular failure. Anything below 0 is an error signal.
		int ReadModuleProperties(const std::string& moduleName, const ProgressCallback& progressCallback = nullptr);

		/// Returns true if loader should ignore missing items in this module.
		/// @return True if loader should ignore missing items in this module.
		bool GetIgnoreMissingItems() const { return m_IgnoreMissingItems; }
#pragma endregion

#pragma region Module Information Getters
		/// Gets whether this DataModule is a userdata module.
		/// @return Whether this DataModule is used for userdata written by the game.
		bool IsUserdata() const { return m_IsUserdata; }

		/// Sets this DataModule as a userdata module.
		void SetAsUserdata() { m_IsUserdata = true; }

		/// Gets the file name of this DataModule, e.g. "MyMod.rte".
		/// @return A string with the data module file name.
		const std::string& GetFileName() const { return m_FileName; }

		/// Gets the friendly name of this DataModule, e.g. "My Great Mod".
		/// @return A string with the data module's friendly name.
		const std::string& GetFriendlyName() const { return m_FriendlyName; }

		/// Gets the author name of this DataModule, e.g. "Data Realms, LLC".
		/// @return A string with the author's name.
		const std::string& GetAuthor() const { return m_Author; }

		/// Gets the description of this DataModule's contents.
		/// @return A string with the description.
		const std::string& GetDescription() const { return m_Description; }

		/// Gets whether this DataModule is considered a faction.
		/// @return Whether this DataModule is considered a faction or not.
		bool IsFaction() const { return m_IsFaction; }

		/// Gets whether this DataModule is considered a merchant.
		/// @return Whether this DataModule is considered a merchant or not.
		bool IsMerchant() const { return m_IsMerchant; }

		/// Gets the version number of this DataModule.
		/// @return An int with the version number, starting at 1.
		int GetVersionNumber() const { return m_Version; }

		/// Gets the BITMAP that visually represents this DataModule, for use in menus.
		/// @return BITMAP pointer that might have the icon. 0 is very possible.
		BITMAP* GetIcon() const { return m_Icon; }

		/// Returns crab-to-human spawn ration for this tech.
		/// @return Crab-to-human spawn ration value.
		float GetCrabToHumanSpawnRatio() const { return m_CrabToHumanSpawnRatio; }

		/// Gets the faction BuyMenu theme data of this DataModule.
		/// @return The faction BuyMenu theme information of this DataModule
		const BuyMenuTheme& GetFactionBuyMenuTheme() const { return m_BuyMenuTheme; }
#pragma endregion

#pragma region Entity Mapping
		/// Gets the data file path of a previously read in (defined) Entity.
		/// @param exactType The type name of the derived Entity. Ownership is NOT transferred!
		/// @param instance The instance name of the derived Entity instance.
		/// @return The file path of the data file that the specified Entity was read from. If no Entity of that description was found, "" is returned.
		std::string GetEntityDataLocation(const std::string& exactType, const std::string& instance);

		/// Gets a previously read in (defined) Entity, by exact type and instance name. Ownership is NOT transferred!
		/// @param exactType The exact type name of the derived Entity instance to get.
		/// @param instance The instance name of the derived Entity instance.
		/// @return A pointer to the requested Entity instance. 0 if no Entity with that derived type or instance name was found. Ownership is NOT transferred!
		const Entity* GetEntityPreset(const std::string& exactType, const std::string& instance);

		/// Adds an Entity instance's pointer and name associations to the internal list of already read in Entities. Ownership is NOT transferred!
		/// If there already is an instance defined, nothing happens. If there is not, a clone is made of the passed-in Entity and added to the library.
		/// @param entityToAdd A pointer to the Entity derived instance to add. It should be created from a Reader. Ownership is NOT transferred!
		/// @param overwriteSame
		/// Whether to overwrite if an instance of the EXACT same TYPE and name was found.
		/// If one of the same name but not the exact type, false is returned regardless and nothing will have been added.
		/// @param readFromFile
		/// The file the instance was read from, or where it should be written.
		/// If "Same" is passed as the file path read from, an overwritten instance will keep the old one's file location entry.
		/// @return
		/// Whether or not a copy of the passed-in instance was successfully inserted into the module.
		/// False will be returned if there already was an instance of that class and instance name inserted previously, unless overwritten.
		bool AddEntityPreset(Entity* entityToAdd, bool overwriteSame = false, const std::string& readFromFile = "Same");

		/// Gets the list of all registered Entity groups of this.
		/// @return The list of all groups. Ownership is not transferred.
		const std::list<std::string>* GetGroupRegister() const { return &m_GroupRegister; }

		/// Registers the existence of an Entity group in this module.
		/// @param newGroup The group to register.
		void RegisterGroup(const std::string& newGroup) {
			m_GroupRegister.push_back(newGroup);
			m_GroupRegister.sort();
			m_GroupRegister.unique();
		}

		/// Fills out a list with all groups registered with this that contain any objects of a specific type and it derivatives.
		/// @param groupList The list that all found groups will be ADDED to. OWNERSHIP IS NOT TRANSFERRED!
		/// @param withType The name of the type to only get groups of.
		/// @return Whether any groups with the specified type were found.
		bool GetGroupsWithType(std::list<std::string>& groupList, const std::string& withType);

		/// Adds to a list all previously read in (defined) Entities which are associated with several specific groups.
		/// @param entityList Reference to a list which will get all matching Entities added to it. Ownership of the list or the Entities placed in it are NOT transferred!
		/// @param groups A list of groups to look for.
		/// @param type The name of the least common denominator type of the Entities you want. "All" will look at all types.
		/// @return Whether any Entities were found and added to the list.
		bool GetAllOfGroups(std::list<Entity*>& entityList, const std::vector<std::string>& groups, const std::string& type) { return GetAllOfOrNotOfGroups(entityList, type, groups, false); }

		/// Adds to a list all previously read in (defined) Entities which are not associated with several specific groups.
		/// @param entityList Reference to a list which will get all matching Entities added to it. Ownership of the list or the Entities placed in it are NOT transferred!
		/// @param group A list of groups to exclude.
		/// @param type The name of the least common denominator type of the Entities you want. "All" will look at all types.
		/// @return Whether any Entities were found and added to the list.
		bool GetAllNotOfGroups(std::list<Entity*>& entityList, const std::vector<std::string>& groups, const std::string& type) { return GetAllOfOrNotOfGroups(entityList, type, groups, true); }

		/// Adds to a list all previously read in (defined) Entities, by inexact type.
		/// @param objectList Reference to a list which will get all matching Entities added to it. Ownership of the list or the Entities placed in it are NOT transferred!
		/// @param type The name of the least common denominator type of the Entities you want. "All" will look at all types.
		/// @return Whether any Entities were found and added to the list.
		bool GetAllOfType(std::list<Entity*>& objectList, const std::string& type);
#pragma endregion

#pragma region Material Mapping
		/// Gets a Material mapping local to this DataModule.
		/// This is used for when multiple DataModules are loading conflicting Materials, and need to resolve the conflicts by mapping their materials to ID's different than those specified in the data files.
		/// @param materialID The material ID to get the mapping for.
		/// @return The material ID that the passed in ID is mapped to, if any. 0 if no mapping present.
		unsigned char GetMaterialMapping(unsigned char materialID) const { return m_MaterialMappings.at(materialID); }

		/// Gets the entire Material mapping array local to this DataModule.
		/// @return A const reference to the entire local mapping array, 256 unsigned chars. Ownership is NOT transferred!
		const std::array<unsigned char, c_PaletteEntriesNumber>& GetAllMaterialMappings() const { return m_MaterialMappings; }

		/// Adds a Material mapping local to a DataModule.
		/// This is used for when multiple DataModules are loading conflicting Materials, and need to resolve the conflicts by mapping their materials to ID's different than those specified in the data files.
		/// @param fromID The material ID to map from.
		/// @param toID The material ID to map to.
		/// @return Whether this created a new mapping which didn't override a previous material mapping.
		bool AddMaterialMapping(unsigned char fromID, unsigned char toID);
#pragma endregion

#pragma region Lua Script Handling
		/// Loads the preset scripts of this object, from a specified path.
		/// @return An error return value signaling success or any particular failure. Anything below 0 is an error signal.
		int LoadScripts() const;

		/// Reloads all scripted Entity Presets with the latest version of their respective script files.
		void ReloadAllScripts() const;
#pragma endregion

	protected:
		/// Holds and owns the actual object instance pointer, and the location of the data file it was read from, as well as where in that file.
		struct PresetEntry {
			/// Constructor method used to instantiate a PresetEntry object in system memory.
			PresetEntry(Entity* preset, const std::string& file) :
			    m_EntityPreset(preset), m_FileReadFrom(file) {}

			Entity* m_EntityPreset; //!< Owned by this.
			std::string m_FileReadFrom; //!< Where the instance was read from.
		};

		bool m_IsUserdata; //!< Whether this DataModule contains userdata written by the game (e.g saved games or editor scenes), meaning it is not an official nor a 3rd party module and is ignored anywhere where that is relevant.
		bool m_ScanFolderContents; //!< Indicates whether module loader should scan for any .ini's inside module folder instead of loading files defined in IncludeFile only.
		bool m_IgnoreMissingItems; //!< Indicates whether module loader should ignore missing items in this module.

		std::string m_FileName; //!< File/folder name of the data module, eg "MyMod.rte".
		std::string m_FriendlyName; //!< Friendly name of the data module, eg "My Weapons Mod".
		std::string m_Author; //!< Name of the author of this module.
		std::string m_Description; //!< Brief description of what this module is and contains.
		std::string m_ScriptPath; //!< Path to script to execute when this module is loaded.
		bool m_IsFaction; //!< Whether this data module is considered a faction.
		bool m_IsMerchant; //!< Whether this data module is considered a merchant.
		version::Semver200_version* m_SupportedGameVersion; //!< Game version this DataModule supports. Needs to satisfy Caret Version Range for this DataModule to be allowed. Base DataModules don't need this.
		int m_Version; //!< Version number, starting with 1.
		int m_ModuleID; //!< ID number assigned to this upon loading, for internal use only, don't reflect in ini's.

		ContentFile m_IconFile; //!< File to the icon/symbol bitmap.
		BITMAP* m_Icon; //!< Bitmap with the icon loaded from above file.

		BuyMenuTheme m_BuyMenuTheme; //!< Faction BuyMenu theme data.

		float m_CrabToHumanSpawnRatio; //!< Crab-to-human Spawn ratio to replace value from Constants.lua.

		std::list<const Entity*> m_EntityList; //!< A list of loaded entities solely for the purpose of enumeration presets from Lua.
		std::list<std::string> m_GroupRegister; //!< List of all Entity groups ever registered in this, all uniques.
		std::array<unsigned char, c_PaletteEntriesNumber> m_MaterialMappings; //!< Material mappings local to this DataModule.

		/// Ordered list of all owned Entity instances, ordered by the sequence of their reading - really now with overwriting?.
		/// This is used to be able to write back all of them in proper order into their respective files in the DataModule when writing this.
		/// The Entity instances ARE owned by this list.
		std::list<PresetEntry> m_PresetList;

		/// Map of class names and map of instance template names and actual Entity instances that were read for this DataModule.
		/// An Entity instance of a derived type will be placed in EACH of EVERY of its parent class' maps here.
		/// There can be multiple entries of the same instance name in any of the type sub-maps, but only ONE whose exact class is that of the type-list!
		/// The Entity instances are NOT owned by this map.
		std::unordered_map<std::string, std::list<std::pair<std::string, Entity*>>> m_TypeMap;

	private:
		static const std::string c_ClassName; //!< A string with the friendly-formatted type name of this object.

#pragma region INI Handling
		/// Checks the module's supported game version against the current game version to ensure compatibility.
		void CheckSupportedGameVersion() const;

		/// If ScanFolderContents is enabled in this DataModule's Index.ini, looks for any ini files in the top-level directory of the module and reads all of them in alphabetical order.
		/// @param progressCallback A function pointer to a function that will be called and sent a string with information about the progress of this DataModule's creation.
		/// @return An error return value signaling success or any particular failure. Anything below 0 is an error signal.
		int FindAndRead(const ProgressCallback& progressCallback = nullptr);
#pragma endregion

#pragma region Entity Mapping
		/// Shared method for filling a list with all previously read in (defined) Entities which are or are not associated with a specific group or groups.
		/// @param entityList Reference to a list which will get all matching Entities added to it. Ownership of the list or the Entities placed in it are NOT transferred!
		/// @param type The name of the least common denominator type of the Entities you want. "All" will look at all types.
		/// @param groups The groups to look for.
		/// @param excludeGroups Whether Entities belonging to the specified group or groups should be excluded.
		/// @return Whether any Entities were found and added to the list.
		bool GetAllOfOrNotOfGroups(std::list<Entity*>& entityList, const std::string& type, const std::vector<std::string>& groups, bool excludeGroups);

		/// Checks if the type map has an instance added of a specific name and exact type.
		/// Does not check if any parent types with that name has been added. If found, that instance is returned, otherwise 0.
		/// @param exactType The exact type name to look for.
		/// @param presetName The exact PresetName to look for.
		/// @return The found Entity Preset of the exact type and name, if found.
		Entity* GetEntityIfExactType(const std::string& exactType, const std::string& presetName);

		/// Adds a newly added preset instance to the type map, where it will end up in every type-list of every class it derived from as well.
		/// I.e the "Entity" map will contain every single instance added to this.
		/// This will NOT check if duplicates are added to any type-list, so please use GetEntityIfExactType to check this beforehand.
		/// Dupes are allowed if there are no more than one of the exact class and name.
		/// @param entityToAdd The new object instance to add. OWNERSHIP IS NOT TRANSFERRED!
		/// @return Whether the Entity was added successfully or not.
		bool AddToTypeMap(Entity* entityToAdd);
#pragma endregion

		/// Clears all the member variables of this DataModule, effectively resetting the members of this abstraction level only.
		void Clear();

		// Disallow the use of some implicit methods.
		DataModule(const DataModule& reference) = delete;
		DataModule& operator=(const DataModule& rhs) = delete;
	};
} // namespace RTE
