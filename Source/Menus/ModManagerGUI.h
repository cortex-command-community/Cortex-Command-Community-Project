#pragma once

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

	/// Integrated mod and script manager user interface composition and handling.
	class ModManagerGUI {

	public:
#pragma region Creation
		/// Constructor method used to instantiate a ModManagerGUI object in system memory and make it ready for use.
		/// @param guiScreen Pointer to a GUIScreen interface that will be used by this ModManagerGUI's GUIControlManager. Ownership is NOT transferred!
		/// @param guiInput Pointer to a GUIInput interface that will be used by this ModManagerGUI's GUIControlManager. Ownership is NOT transferred!
		/// @param createForPauseMenu Whether this SettingsGUI is part of ModManagerGUI and should have a slightly different layout.
		ModManagerGUI(AllegroScreen* guiScreen, GUIInputWrapper* guiInput, bool createForPauseMenu = false);
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
			std::string ModulePath; //!< Mod DataModule path.
			std::string ModuleName; //!< Mod ModuleName.
			std::string Description; //!< Mod description.
			bool Disabled; //!< Whether the mod is disabled through the settings file or not.

			/// Makes GUI displayable string with mod info.
			/// @return String with mod info.
			std::string GetDisplayString() const { return (Disabled ? "- " : "+ ") + ModulePath + " - " + ModuleName; }

			/// Comparison operator for sorting the KnownMods list alphabetically by path with std::sort.
			/// @param rhs ModRecord to compare with.
			/// @return Bool with result of the alphabetical comparison.
			bool operator<(const ModRecord& rhs) const { return ModulePath < rhs.ModulePath; }
		};

		/// Struct containing information about a valid GlobalScript.
		struct ScriptRecord {
			std::string PresetName; //!< Script PresetName.
			std::string Description; //!< Script description.
			bool Enabled; //!< Whether the script is enabled through the settings file or not.

			/// Makes GUI displayable string with script info.
			/// @return String with script info.
			std::string GetDisplayString() const { return (!Enabled ? "- " : "+ ") + PresetName; }

			/// Comparison operator for sorting the KnownScripts list alphabetically by PresetName with std::sort.
			/// @param rhs ScriptRecord to compare with.
			/// @return Bool with result of the alphabetical comparison.
			bool operator<(const ScriptRecord& rhs) const { return PresetName < rhs.PresetName; }
		};

		std::unique_ptr<GUIControlManager> m_GUIControlManager; //!< The GUIControlManager which holds all the GUIControls of the ModManagerGUI.

		std::vector<ModRecord> m_KnownMods; //!< Contains ModRecords for all valid mod DataModules.
		std::vector<ScriptRecord> m_KnownScripts; //!< Contains ScriptRecords for all valid GlobalScripts.

		bool m_ModsListFetched; //!< Whether the known mods list was fetched, even if no valid mod DataModules were added to it.
		bool m_ScriptsListFetched; //!< Whether the known scripts list was fetched, even if no valid GlobalScripts were added to it.

		/// GUI elements that compose the Mod Manager menu screen.
		GUIButton* m_BackToMainButton;
		GUIButton* m_ToggleModButton;
		GUIButton* m_ToggleScriptButton;
		GUIListBox* m_ModsListBox;
		GUIListBox* m_ScriptsListBox;
		GUILabel* m_ModOrScriptDescriptionLabel;

#pragma region Mod and Script Handling
		/// Gets whether both lists were fetched, even if nothing valid was added to them.
		/// @return Whether both lists were fetched, even if nothing valid was added to them.
		bool ListsFetched() const { return m_ModsListFetched && m_ScriptsListFetched; }

		/// Fills the KnownMods list with all valid mod DataModules, then fills the ModsListBox using it.
		void PopulateKnownModsList();

		/// Fills the KnownScripts list with all valid GlobalScripts, then fills the ScriptsListBox using it.
		void PopulateKnownScriptsList();

		/// Turns currently selected mod on and off and changes GUI elements accordingly.
		void ToggleMod();

		/// Turns currently selected script on and off and changes GUI elements accordingly.
		void ToggleScript();
#pragma endregion

		// Disallow the use of some implicit methods.
		ModManagerGUI(const ModManagerGUI& reference) = delete;
		ModManagerGUI& operator=(const ModManagerGUI& rhs) = delete;
	};
} // namespace RTE
