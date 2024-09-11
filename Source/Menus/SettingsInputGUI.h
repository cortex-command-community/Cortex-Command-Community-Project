#pragma once

#include "SettingsInputMappingGUI.h"

struct BITMAP;

#include <array>
#include <memory>

namespace RTE {

	class Controller;
	class GUIControlManager;
	class GUICollectionBox;
	class GUILabel;
	class GUIButton;
	class GUISlider;
	class GUIRadioButton;
	class GUIEvent;

	/// Handling for player input settings through the game settings user interface.
	class SettingsInputGUI {

	public:
#pragma region Creation
		/// Constructor method used to instantiate a SettingsInputGUI object in system memory and make it ready for use.
		/// @param parentControlManager Pointer to the parent GUIControlManager which owns all the GUIControls of this SettingsInputGUI. Ownership is NOT transferred!
		explicit SettingsInputGUI(GUIControlManager* parentControlManager);
#pragma endregion

#pragma region Getters and Setters
		/// Enables or disables the SettingsInputGUI.
		/// @param enable Show and enable or hide and disable the SettingsInputGUI.
		void SetEnabled(bool enable = true) const;

		/// Gets the currently active GUICollectionBox that acts as a dialog box and requires disabling navigation and drawing an overlay.
		/// @return Pointer to the GUICollectionBox that is the currently active dialog box, if any. Ownership is NOT transferred!
		GUICollectionBox* GetActiveDialogBox() const { return m_InputMappingConfigMenu->GetActiveDialogBox(); }

		/// Closes the currently active GUICollectionBox that acts as a dialog box by hiding it. If the active dialog box is a sub-menu, disables it.
		void CloseActiveDialogBox() const { m_InputMappingConfigMenu->CloseActiveDialogBox(); }
#pragma endregion

#pragma region Input Config Wizard Handling
		/// Gets whether the player is currently manually configuring an InputMapping through the input mapping menu screen.
		/// @return Whether the player is currently manually configuring an InputMapping through the input mapping menu screen.
		bool InputMappingConfigIsConfiguringManually() const { return m_InputMappingConfigMenu->IsConfiguringManually(); }

		/// Handles input capture logic of the input mapping menu screen manual configuration sequence.
		void HandleMappingConfigManualConfiguration() const { m_InputMappingConfigMenu->HandleManualConfigSequence(); }

		/// Gets whether the player is currently manually configuring the InputScheme through the input mapping wizard.
		/// @return Whether the player is currently manually configuring the InputScheme through the input mapping wizard.
		bool InputConfigWizardIsConfiguringManually() const { return m_InputMappingConfigMenu->GetInputConfigWizardMenu()->IsConfiguringManually(); }

		/// Handles input capture logic of the input mapping wizard manual configuration sequence.
		void HandleConfigWizardManualConfiguration() const { m_InputMappingConfigMenu->GetInputConfigWizardMenu()->HandleManualConfigSequence(); }
#pragma endregion

#pragma region Concrete Methods
		/// Handles the player interaction with the SettingsInputGUI GUI elements.
		/// @param guiEvent The GUIEvent containing information about the player interaction with an element.
		void HandleInputEvents(GUIEvent& guiEvent);
#pragma endregion

	private:
		/// Struct containing GUI elements that compose the input settings box of a player.
		struct PlayerInputSettingsBox {
			GUILabel* SelectedDeviceLabel;
			GUIButton* NextDeviceButton;
			GUIButton* PrevDeviceButton;
			GUIButton* ConfigureControlsButton;
			GUIButton* ResetControlsButton;
			GUILabel* SensitivityLabel;
			GUISlider* SensitivitySlider;
			GUICollectionBox* DeadZoneControlsBox;
			GUIRadioButton* CircleDeadZoneRadioButton;
			GUIRadioButton* SquareDeadZoneRadioButton;
		};

		GUIControlManager* m_GUIControlManager; //!< The GUIControlManager which holds all the GUIControls of this menu. Not owned by this.

		std::unique_ptr<SettingsInputMappingGUI> m_InputMappingConfigMenu; //!< The input mapping configuration sub-menu.

		/// GUI elements that compose the input settings menu screen.
		GUICollectionBox* m_InputSettingsBox;
		std::array<PlayerInputSettingsBox, Players::MaxPlayerCount> m_PlayerInputSettingsBoxes;

#pragma region Input Settings Handling
		/// Resets the player input settings to the defaults.
		/// @param player The player to reset input settings for.
		void ResetPlayerInputSettings(int player);

		/// Changes the player's input device in the InputScheme and proceeds to update the device labels accordingly.
		/// @param player The player to change input device for.
		/// @param nextDevice Whether to change to the next or previous input device.
		void SetPlayerNextOrPrevInputDevice(int player, bool nextDevice);

		/// Updates the currently selected input device label of a player according to the InputScheme.
		/// @param player The player to update selected input device label for.
		void UpdatePlayerSelectedDeviceLabel(int player);

		/// Enables the input sensitivity controls for a player if applicable to the selected input device.
		/// @param player The player to enable input sensitivity controls for.
		void ShowOrHidePlayerInputDeviceSensitivityControls(int player);

		/// Updates the input sensitivity controls of a player according to the InputScheme.
		/// @param player The player to update input sensitivity control values for.
		void UpdatePlayerInputSensitivityControlValues(int player);
#pragma endregion

		// Disallow the use of some implicit methods.
		SettingsInputGUI(const SettingsInputGUI& reference) = delete;
		SettingsInputGUI& operator=(const SettingsInputGUI& rhs) = delete;
	};
} // namespace RTE
