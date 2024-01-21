#pragma once

namespace RTE {

	class GUIControlManager;
	class GUICollectionBox;
	class GUICheckbox;
	class GUITextBox;
	class GUISlider;
	class GUILabel;
	class GUIEvent;

	/// Handling for gameplay settings through the game settings user interface.
	class SettingsGameplayGUI {

	public:
#pragma region Creation
		/// Constructor method used to instantiate a SettingsGameplayGUI object in system memory and make it ready for use.
		/// @param parentControlManager Pointer to the parent GUIControlManager which owns all the GUIControls of this SettingsGameplayGUI. Ownership is NOT transferred!
		explicit SettingsGameplayGUI(GUIControlManager* parentControlManager);
#pragma endregion

#pragma region Concrete Methods
		/// Enables or disables the SettingsGameplayGUI.
		/// @param enable Show and enable or hide and disable the SettingsGameplayGUI.
		void SetEnabled(bool enable = true);

		/// Handles the player interaction with the SettingsInputGUI GUI elements.
		/// @param guiEvent The GUIEvent containing information about the player interaction with an element.
		void HandleInputEvents(GUIEvent& guiEvent);
#pragma endregion

	private:
		GUIControlManager* m_GUIControlManager; //!< The GUIControlManager which holds all the GUIControls of this menu. Not owned by this.

		/// GUI elements that compose the gameplay settings menu screen.
		GUICollectionBox* m_GameplaySettingsBox;
		GUICheckbox* m_FlashOnBrainDamageCheckbox;
		GUICheckbox* m_BlipOnRevealUnseenCheckbox;
		GUICheckbox* m_ShowForeignItemsCheckbox;
		GUICheckbox* m_EnableCrabBombsCheckbox;
		GUICheckbox* m_EndlessMetaGameCheckbox;
		GUICheckbox* m_ShowEnemyHUDCheckbox;
		GUICheckbox* m_EnableSmartBuyMenuNavigationCheckbox;
		GUITextBox* m_MaxUnheldItemsTextbox;
		GUITextBox* m_CrabBombThresholdTextbox;

		GUISlider* m_UnheldItemsHUDDisplayRangeSlider;
		GUILabel* m_UnheldItemsHUDDisplayRangeLabel;

		GUICheckbox* m_AlwaysDisplayUnheldItemsInStrategicModeCheckbox;

		GUISlider* m_ScreenShakeStrengthSlider;
		GUILabel* m_ScreenShakeStrengthLabel;

#pragma region Gameplay Settings Handling
		/// Updates the MaxUnheldItems textbox to override any invalid input, applies the setting value and removes its focus.
		void UpdateMaxUnheldItemsTextbox();

		/// Updates the CrabBombThreshold textbox to override any invalid input, applies the setting value and removes its focus.
		void UpdateCrabBombThresholdTextbox();

		/// Updates the UnheldItemsHUDDisplayRange setting and label according to the slider value.
		void UpdateUnheldItemsHUDDisplayRange();

		/// Updates the Screen Shake strength setting and label according to the slider value.
		void UpdateScreenShakeStrength();
#pragma endregion

		// Disallow the use of some implicit methods.
		SettingsGameplayGUI(const SettingsGameplayGUI& reference) = delete;
		SettingsGameplayGUI& operator=(const SettingsGameplayGUI& rhs) = delete;
	};
} // namespace RTE
