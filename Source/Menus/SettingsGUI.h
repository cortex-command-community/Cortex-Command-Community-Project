#pragma once

#include "SettingsVideoGUI.h"
#include "SettingsAudioGUI.h"
#include "SettingsInputGUI.h"
#include "SettingsGameplayGUI.h"
#include "SettingsMiscGUI.h"

#include <array>
#include <memory>

namespace RTE {

	class AllegroScreen;
	class GUIInputWrapper;
	class GUIControlManager;
	class GUIButton;
	class GUITab;

	/// Handling for the settings menu screen composition and sub-menu interaction.
	class SettingsGUI {

	public:
#pragma region Creation
		/// Constructor method used to instantiate a SettingsGUI object in system memory and make it ready for use.
		/// @param guiScreen Pointer to a GUIScreen interface that will be used by this SettingsGUI's GUIControlManager. Ownership is NOT transferred!
		/// @param guiInput Pointer to a GUIInput interface that will be used by this SettingsGUI's GUIControlManager. Ownership is NOT transferred!
		/// @param createForPauseMenu Whether this SettingsGUI is part of PauseMenuGUI and should have a slightly different layout.
		SettingsGUI(AllegroScreen* guiScreen, GUIInputWrapper* guiInput, bool createForPauseMenu = false);
#pragma endregion

#pragma region Getters
		/// Gets the currently active GUICollectionBox of this SettingsGUI or any of its sub-menus that acts as a dialog box and requires disabling navigation and drawing an overlay.
		/// @return Pointer to the GUICollectionBox that is the currently active dialog box. Ownership is NOT transferred!
		GUICollectionBox* GetActiveDialogBox() const;
#pragma endregion

#pragma region Concrete Methods
		/// Closes the currently active GUICollectionBox that acts as a dialog box by hiding it. If the active dialog box is a sub-menu, disables it.
		void CloseActiveDialogBox() const;

		/// Sets the currently active settings menu screen again to refresh it. This is used in case textboxes were left empty or focused on settings screen exit via back button or esc.
		void RefreshActiveSettingsMenuScreen() { SetActiveSettingsMenuScreen(m_ActiveSettingsMenuScreen, false); }

		/// Handles the player interaction with the SettingsGUI GUI elements.
		/// @return Whether the player requested to return to the main menu.
		bool HandleInputEvents();

		/// Draws the SettingsGUI to the screen.
		void Draw() const;
#pragma endregion

	private:
		/// Enumeration for the different sub-menu screens of the settings menu.
		enum SettingsMenuScreen {
			VideoSettingsMenu,
			AudioSettingsMenu,
			InputSettingsMenu,
			GameplaySettingsMenu,
			MiscSettingsMenu,
			SettingsMenuCount
		};

		std::unique_ptr<GUIControlManager> m_GUIControlManager; //!< The GUIControlManager which owns all the GUIControls the SettingsGUI.

		SettingsMenuScreen m_ActiveSettingsMenuScreen; //!< The currently active settings menu that is being updated and drawn to the screen. See SettingsMenuScreen enumeration.

		std::unique_ptr<SettingsVideoGUI> m_VideoSettingsMenu; //!< The video settings sub-menu.
		std::unique_ptr<SettingsAudioGUI> m_AudioSettingsMenu; //!< The audio settings sub-menu.
		std::unique_ptr<SettingsInputGUI> m_InputSettingsMenu; //!< The input settings sub-menu.
		std::unique_ptr<SettingsGameplayGUI> m_GameplaySettingsMenu; //!< The gameplay settings sub-menu.
		std::unique_ptr<SettingsMiscGUI> m_MiscSettingsMenu; //!< The misc settings sub-menu.

		/// GUI elements that compose the settings menu screen.
		GUICollectionBox* m_SettingsTabberBox;
		GUIButton* m_BackToMainButton;
		std::array<GUITab*, SettingsMenuScreen::SettingsMenuCount> m_SettingsMenuTabs;

#pragma region Settings Menu Handling
		/// Disables the settings menu tabber and back buttons. This is used when a settings sub-menu dialog box is active.
		void DisableSettingsMenuNavigation(bool disable) const;

		/// Sets the SettingsGUI to display a settings menu screen.
		/// @param activeMenu Which settings menu screen to display. See the SettingsMenuScreen enumeration.
		/// @param playButtonPressSound Whether to play a sound if the menu screen change is triggered by a button/tab press.
		void SetActiveSettingsMenuScreen(SettingsMenuScreen activeMenu, bool playButtonPressSound = true);
#pragma endregion

		// Disallow the use of some implicit methods.
		SettingsGUI(const SettingsGUI& reference) = delete;
		SettingsGUI& operator=(const SettingsGUI& rhs) = delete;
	};
} // namespace RTE
