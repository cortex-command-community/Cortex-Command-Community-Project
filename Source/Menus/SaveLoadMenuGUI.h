#pragma once

#include <filesystem>
#include <vector>
#include <string>
#include <memory>

namespace RTE {

	class PauseMenuGUI;

	class AllegroScreen;
	class GUIInputWrapper;
	class GUIControlManager;
	class GUILabel;
	class GUIButton;
	class GUIListBox;
	class GUITextBox;
	class GUIComboBox;
	class GUICollectionBox;

	/// Integrated savegame user interface composition and handling.
	class SaveLoadMenuGUI {

	public:
#pragma region Creation
		/// Constructor method used to instantiate a SaveLoadMenuGUI object in system memory and make it ready for use.
		/// @param guiScreen Pointer to a GUIScreen interface that will be used by this SaveLoadMenuGUI's GUIControlManager. Ownership is NOT transferred!
		/// @param guiInput Pointer to a GUIInput interface that will be used by this SaveLoadMenuGUI's GUIControlManager. Ownership is NOT transferred!
		/// @param createForPauseMenu Whether this SettingsGUI is part of SaveLoadMenuGUI and should have a slightly different layout.
		SaveLoadMenuGUI(AllegroScreen* guiScreen, GUIInputWrapper* guiInput, bool createForPauseMenu = false);
#pragma endregion

#pragma region Concrete Methods
		/// Handles the player interaction with the SaveLoadMenuGUI GUI elements.
		/// @param pauseMenu Pointer to the pause menu, if we're being called from the pause menu. Ownership is NOT transferred!
		/// @return Whether the player requested to return to the main menu.
		bool HandleInputEvents(PauseMenuGUI* pauseMenu = nullptr);

		/// Causes a refresh of the save files.
		void Refresh();

		/// Draws the SaveLoadMenuGUI to the screen.
		void Draw() const;
#pragma endregion

	private:
		enum class ConfirmDialogMode {
			None,
			ConfirmOverwrite,
			ConfirmDelete
		};

		/// Struct containing information about a valid Savegame.
		struct SaveRecord {
			std::filesystem::path SavePath; //!< Savegame filepath.
			std::filesystem::file_time_type SaveDate; //!< Last modified date.
			std::string Activity; //!< The activity name.
			std::string Scene; //!< The scene name.
		};

		std::unique_ptr<GUIControlManager> m_GUIControlManager; //!< The GUIControlManager which holds all the GUIControls of the SaveLoadMenuGUI.

		std::vector<SaveRecord> m_SaveGames; //!< Contains all SaveGames.

		bool m_SaveGamesFetched; //!< Whether the savegames list has been fetched.

		/// GUI elements that compose the Mod Manager menu screen.
		GUICollectionBox* m_SaveGameMenuBox;
		GUIButton* m_BackToMainButton;
		GUITextBox* m_SaveGameName;
		GUIButton* m_LoadButton;
		GUIButton* m_CreateButton;
		GUIButton* m_OverwriteButton;
		GUIButton* m_DeleteButton;
		GUIListBox* m_SaveGamesListBox;
		GUILabel* m_ActivityCannotBeSavedLabel;
		GUIComboBox* m_OrderByComboBox;

		// The confirmation box and its controls
		ConfirmDialogMode m_ConfirmDialogMode;
		GUICollectionBox* m_ConfirmationBox;
		GUILabel* m_ConfirmationLabel;
		GUIButton* m_ConfirmationButton;
		GUIButton* m_CancelButton;

#pragma region Savegame Handling
		/// Gets whether both lists were fetched, even if nothing valid was added to them.
		/// @return Whether save games were fetched, even if nothing valid was added to them.
		bool ListsFetched() const { return m_SaveGamesFetched; }

		/// Fills the SaveGames list with all valid savegames.
		void PopulateSaveGamesList();

		/// Updates the SaveGamesListBox GUI.
		void UpdateSaveGamesGUIList();

		/// Loads the currently selected savefile.
		/// @return Whether a same was succesfully loaded.
		bool LoadSave();

		/// Creates a new savefile (or overwrites the existing one) with the name from the textbox.
		void CreateSave();

		/// Deletes the savefile with the name from the textbox.
		void DeleteSave();
#pragma endregion

		/// Updates buttons and sets whether or not they should be enabled.
		void UpdateButtonEnabledStates();

		/// Shows confirmation box for overwrite or delete.
		void SwitchToConfirmDialogMode(ConfirmDialogMode mode);

		// Disallow the use of some implicit methods.
		SaveLoadMenuGUI(const SaveLoadMenuGUI& reference) = delete;
		SaveLoadMenuGUI& operator=(const SaveLoadMenuGUI& rhs) = delete;
	};
} // namespace RTE
