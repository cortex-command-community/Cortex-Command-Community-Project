#pragma once

#include "DataModule.h"

#include <string>
#include <vector>
#include <memory>

namespace RTE {

	class AllegroScreen;
	class GUIInputWrapper;
	class GUIControlManager;
	class GUILabel;
	class GUIButton;
	class GUIListBox;
	class GUITab;
	class GUICollectionBox;
	class GUIBitmap;

	/// Integrated mod and script manager user interface composition and handling.
	class ModManagerGUI {

	public:
#pragma region Creation
		/// Constructor method used to instantiate a ModManagerGUI object in system memory and make it ready for use.
		/// @param guiScreen Pointer to a GUIScreen interface that will be used by this ModManagerGUI's GUIControlManager. Ownership is NOT transferred!
		/// @param guiInput Pointer to a GUIInput interface that will be used by this ModManagerGUI's GUIControlManager. Ownership is NOT transferred!
		ModManagerGUI(AllegroScreen* guiScreen, GUIInputWrapper* guiInput);
#pragma endregion

#pragma region Concrete Methods
		/// Handles the player interaction with the ModManagerGUI GUI elements.
		/// @return Whether the player requested to return to the main menu.
		bool HandleInputEvents();

		/// Draws the ModManagerGUI to the screen.
		void Draw() const;
#pragma endregion

	private:
		/// Struct containing information about a valid mod DataModule.
		struct ModRecord {
			const DataModule* dataModule; //!< The mod's DataModule, or nullptr if it wasn't loaded at launch. Not owned.
			bool Enabled; //!< Whether the mod is enabled.
			int InternalModuleIndex; //!< The mod's module ID, or -1 if it wasn't loaded at launch.

			/// Constructor for a mod DataModule that WAS loaded at launch.
			ModRecord(const DataModule* dataModule, int internalModuleIndex) : 
				dataModule(dataModule) 
			{
				Loaded = true;
				Enabled = true;
				InternalModuleIndex = internalModuleIndex;
			}
			/// Constructor for a mod DataModule that was NOT loaded at launch.
			ModRecord(const std::string& dotRTEName) :
			    dataModule(nullptr), DotRTENameIfWasntLoaded(dotRTEName) 
			{
				Loaded = false;
				Enabled = false;
				InternalModuleIndex = -1;
			}
			/// Gets the mod's .rte name, e.g. "MyMod.rte".
			const std::string& GetDotRTEName() const {
				return Loaded ? dataModule->GetFileName() : DotRTENameIfWasntLoaded;
			}
			/// Gets the mod's friendly name, empty if it wasn't loaded at launch.
			const std::string& GetFriendlyName() const {
				return Loaded ? dataModule->GetFriendlyName() : "";
			}
			/// Gets the mod's author and description, for the description box.
			const std::string GetDescription() const {
				if (!Loaded) {
					return "N/A, Module was not loaded";
				} else {
					std::string authorString =
					    dataModule->GetAuthor() == ""
					        ? ""
					        : "Author: " + dataModule->GetAuthor() + "\n\n";
					return std::move(authorString) + dataModule->GetDescription();
				}
			}

			/// Makes GUI displayable string with mod info.
			/// @return String with mod info.
			std::string GetDisplayString() const { 
				return (Enabled ? "+ " : "- ") + GetDotRTEName(); 
			}

			/// Comparison operator for sorting the KnownMods list alphabetically by path with std::sort.
			/// @param rhs ModRecord to compare with.
			/// @return Bool with result of the alphabetical comparison.
			bool operator<(const ModRecord& rhs) const { return GetDotRTEName() < rhs.GetDotRTEName(); }

		private:
			bool Loaded; //!< Whether the mod DataModule was loaded at game launch.
			std::string DotRTENameIfWasntLoaded; //!< Mod DataModule's name string for if it was NOT loaded at game launch.
		};

		/// Struct containing information about a valid GlobalScript.
		struct ScriptRecord {
			std::string DisplayName;
			std::string ModuleAndPresetName; //!< The script's module and preset name, which the settings file knows it by.
			std::string Description; //!< Script description.
			bool Enabled; //!< Whether the script is enabled through the settings file or not.

			/// Makes GUI displayable string with script info.
			/// @return String with script info.
			std::string GetDisplayString() const {
				return (!Enabled ? "    - " : "    + ") + DisplayName;
			}

			/// Comparison operator for sorting the KnownScripts list alphabetically by PresetName with std::sort.
			/// @param rhs ScriptRecord to compare with.
			/// @return Bool with result of the alphabetical comparison.
			bool operator<(const ScriptRecord& rhs) const { return DisplayName < rhs.DisplayName; }
		};

		struct ScriptRecordsInAModule {
			std::string ModuleName;
			std::string Description;
			std::vector<ScriptRecord> Records;
			bool Collapsed = false;
			int InternalModuleIndex; //!< The module ID of the module the scripts are in.

			ScriptRecordsInAModule(std::string moduleName, std::string description, int internalModuleIndex) {
				ModuleName = moduleName;
				Description = description;
				InternalModuleIndex = internalModuleIndex;
			}

			ScriptRecord& operator[](int i) {
				return Records[i];
			}

			std::string GetDisplayString() const {
				return (Collapsed ? "+ " : "- ") + ModuleName;
			}
		};

		std::unique_ptr<GUIControlManager> m_GUIControlManager; //!< The GUIControlManager which holds all the GUIControls of the ModManagerGUI.

		std::vector<ModRecord> m_KnownMods; //!< Contains ModRecords for all valid mod DataModules.
		std::vector<ScriptRecord> m_KnownScripts; //!< Contains ScriptRecords for all valid GlobalScripts.
		std::vector<ScriptRecordsInAModule> m_KnownScriptsPerModule;

		bool m_ModsListFetched; //!< Whether the known mods list was fetched, even if no valid mod DataModules were added to it.
		bool m_ScriptsListFetched; //!< Whether the known scripts list was fetched, even if no valid GlobalScripts were added to it.

		/// GUI elements that compose the Mod Manager menu screen.
		GUIButton* m_BackToMainButton;
		GUIButton* m_OpenModsFolderButton;
		GUIButton* m_ToggleListEntryButton;
		GUIListBox* m_ModsListBox;
		GUIListBox* m_ScriptsListBox;
		GUILabel* m_ModOrScriptDescriptionLabel;
		GUIButton* m_ModsTabButton;
		GUIButton* m_ScriptsTabButton;
		GUILabel* m_ModuleHeaderLabel;
		GUICollectionBox* m_ModuleIconCollectionBox;

		/// Gets the warning shown in the description box when nothing is selected.
		const std::string& GetDisclaimerText() {
			static const std::string disclaimerText = 
				"DISCLAIMER: mods and any global scripts may cause performance degradation, unbalancing, game exploitation, crashes and extreme fun. Use wisely and be prepared to face the consequences.";
			return disclaimerText;
		}

		/// The tabs of the Mod Manager menu.
		enum ModManagerTab {
			Mods,
			Scripts
		};
		ModManagerTab m_CurrentTab; //!< The tab currently shown.

#pragma region Mod and Script Handling
		/// Gets whether both lists were fetched, even if nothing valid was added to them.
		/// @return Whether both lists were fetched, even if nothing valid was added to them.
		bool ListsFetched() const { return m_ModsListFetched && m_ScriptsListFetched; }

		/// Fills the KnownMods list with all valid mod DataModules, then fills the ModsListBox using it.
		void PopulateKnownModsList();

		void InitializeKnownScripts();

		int ScriptListEntryEncodeExtraIndex(const int, const int);
		std::pair<int, int> ScriptListEntryDecodeExtraIndex(const int);

		const int EXTRA_INDEX_IS_A_MODULE_LABEL_MASK = 1 << 24;

		void PopulateKnownScriptsList(bool);

		void ResetSelectionsAndGoToTop();

		/// Opens the Mods folder in the system's file browser.
		void OpenModsFolder();

		/// Turns currently selected mod on and off and changes GUI elements accordingly.
		void ToggleMod();

		//todo
		ScriptRecord* ScriptListExtraIndexToScriptRecord(int);

		/// Shows a module's name and icon in the header above the description box.
		/// @param moduleIndex The module ID of the module to show, or -1 to show none.
		void UpdateModuleHeader(const int moduleIndex);

		/// todo Turns currently selected script on and off and changes GUI elements accordingly.
		void ToggleInScriptList();
#pragma endregion

		// Disallow the use of some implicit methods.
		ModManagerGUI(const ModManagerGUI& reference) = delete;
		ModManagerGUI& operator=(const ModManagerGUI& rhs) = delete;
	};
} // namespace RTE
