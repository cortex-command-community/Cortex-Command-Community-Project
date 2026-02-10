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
			const DataModule* dataModule;
			bool Enabled;
			int InternalModuleIndex;

			//!< Contructor for a mod DataModule that WAS loaded at launch.
			ModRecord(const DataModule* dataModule, int internalModuleIndex) : 
				dataModule(dataModule) 
			{
				Loaded = true;
				Enabled = true;
				InternalModuleIndex = internalModuleIndex;
			}
			//!< Contructor for a mod DataModule that was NOT loaded at launch.
			ModRecord(const std::string& dotRTEName) :
			    dataModule(nullptr), DotRTENameIfWasntLoaded(dotRTEName) 
			{
				Loaded = false;
				Enabled = false;
				InternalModuleIndex = -1;
			}
			//!< Mod DataModule path.
			const std::string& GetDotRTEName() const {
				return Loaded ? dataModule->GetFileName() : DotRTENameIfWasntLoaded;
			}
			//!< Mod ModuleName.
			const std::string& GetFriendlyName() const {
				return Loaded ? dataModule->GetFriendlyName() : "";
			}
			//!< Mod description.
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
			//!< Whether the mod DataModule was loaded at game launch.
			bool Loaded;
			//!< Mod DataModule's name string for if it was NOT loaded at game launch.
			std::string DotRTENameIfWasntLoaded;
		};

		/// Struct containing information about a valid GlobalScript.
		struct ScriptRecord {
			std::string DisplayName;
			std::string ModuleAndPresetName; //!< The .rte name.
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
			int InternalModuleIndex;

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

		//todo
		const std::string& GetDisclaimerText() {
			static const std::string disclaimerText = 
				"DISCLAIMER: mods and any global scripts may cause performance degradation, unbalancing, game exploitation, crashes and extreme fun. Use wisely and be prepared to face the consequences.";
			return disclaimerText;
		}

		enum ModManagerTab {
			Mods,
			Scripts
		};
		ModManagerTab m_CurrentTab;

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

		void OpenModsFolder();

		/// Turns currently selected mod on and off and changes GUI elements accordingly.
		void ToggleMod();

		//todo
		ScriptRecord* ScriptListExtraIndexToScriptRecord(int);

		void UpdateModuleHeader(const int moduleIndex);

		/// todo Turns currently selected script on and off and changes GUI elements accordingly.
		void ToggleInScriptList();
#pragma endregion

		// Disallow the use of some implicit methods.
		ModManagerGUI(const ModManagerGUI& reference) = delete;
		ModManagerGUI& operator=(const ModManagerGUI& rhs) = delete;
	};
} // namespace RTE
