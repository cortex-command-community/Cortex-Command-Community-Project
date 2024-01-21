#pragma once

#include "Timer.h"
#include "InputScheme.h"

struct BITMAP;

namespace RTE {

	class InputScheme;
	class GUIControlManager;
	class GUICollectionBox;
	class GUILabel;
	class GUIButton;
	class GUIEvent;

	/// Handling for the user input mapping wizard through the game settings user interface.
	class SettingsInputMappingWizardGUI {

	public:
#pragma region Creation
		/// Constructor method used to instantiate a SettingsInputMappingWizardGUI object in system memory and make it ready for use.
		/// @param parentControlManager Pointer to the parent GUIControlManager which owns all the GUIControls of this SettingsInputMappingWizardGUI. Ownership is NOT transferred!
		explicit SettingsInputMappingWizardGUI(GUIControlManager* parentControlManager);
#pragma endregion

#pragma region Getters and Setters
		/// Gets whether this SettingsInputMappingWizardGUI is currently visible and enabled.
		/// @return Whether this SettingsInputMappingWizardGUI is currently visible and enabled.
		bool IsEnabled() const;

		/// Enables or disables the SettingsInputMappingWizardGUI.
		/// @param enable Show and enable or hide and disable the SettingsInputMappingWizardGUI.
		/// @param player The player this SettingsInputMappingWizardGUI is mapping inputs for.
		void SetEnabled(bool enable = true, int player = 0, InputScheme* playerScheme = nullptr);

		/// Gets the currently active GUICollectionBox that acts as a dialog box and requires disabling navigation and drawing an overlay.
		/// @return Pointer to the GUICollectionBox that is the currently active dialog box, if any. Ownership is NOT transferred!
		GUICollectionBox* GetActiveDialogBox() const;

		/// Gets whether this SettingsInputMappingWizardGUI needs to capture input for manual configuration.
		/// @return Whether this SettingsInputMappingWizardGUI needs to capture input for manual configuration.
		bool IsConfiguringManually() const;
#pragma endregion

#pragma region Concrete Methods
		/// Handles the player interaction with the SettingsInputMappingWizardGUI GUI elements.
		/// @param guiEvent The GUIEvent containing information about the player interaction with an element.
		/// @return Whether this SettingsInputMappingGUI changed the input scheme of the configuring player.
		bool HandleInputEvents(GUIEvent& guiEvent);

		/// Handles updating and progressing the manual input configuration sequence.
		void HandleManualConfigSequence();
#pragma endregion

	private:
		/// Enumeration for the different types of gamepads that can be configured.
		enum GamepadType {
			DPad,
			AnalogDualShock,
			AnalogXbox
		};

		/// Struct containing GUI elements that compose the input mapping wizard manual configuration menu screen.
		struct WizardManualConfigScreen {
			GUICollectionBox* ManualConfigBox;
			GUILabel* ConfigDeviceTypeLabel;
			GUILabel* ConfigStepDescriptionLabel;
			GUILabel* ConfigStepRecommendedKeyLabel;
			GUICollectionBox* GamepadConfigRecommendedBox;
			GUILabel* GamepadConfigStepRecommendedInputLabel;
			GUICollectionBox* GamepadConfigRecommendedDiagramBox;
			GUILabel* ConfigStepLabel;
			GUIButton* PrevConfigStepButton;
			GUIButton* NextConfigStepButton;
			GUIButton* ResetConfigButton;
			GUIButton* DiscardOrApplyConfigButton;
		};

		/// Struct containing GUI elements that compose the input mapping wizard preset selection menu screen.
		struct WizardPresetSelectScreen {
			GUICollectionBox* PresetSelectBox;
			GUIButton* CloseWizardButton;
			GUIButton* PresetSelectSNESButton;
			GUIButton* PresetSelectDS4Button;
			GUIButton* PresetSelectXB360Button;
			GUIButton* StartConfigDPadTypeButton;
			GUIButton* StartConfigAnalogDSTypeButton;
			GUIButton* StartConfigAnalogXBTypeButton;
		};

		static constexpr int m_KeyboardConfigSteps = 16; //!< The step count for keyboard only manual configuration.
		static constexpr int m_MouseAndKeyboardConfigSteps = 11; //!< The step count for mouse + keyboard manual configuration.
		static constexpr int m_DPadConfigSteps = 12; //!< The step count for DPad type gamepad manual configuration.
		static constexpr int m_DualAnalogConfigSteps = 20; //!< The step count for DualAnalog type gamepad manual configuration.

		GUIControlManager* m_GUIControlManager; //!< The GUIControlManager which holds all the GUIControls of this menu. Not owned by this.

		Players m_ConfiguringPlayer; //!< The player this SettingsInputMappingWizardGUI is configuring input mapping for.
		InputScheme* m_ConfiguringPlayerScheme; //!< The InputScheme of the configuring player.

		InputDevice m_ConfiguringDevice; //!< Which type of device we are currently configuring.
		bool m_ConfiguringDeviceIsGamepad; //!< Whether the device being configured is a gamepad of any type.
		GamepadType m_ConfiguringGamepadType; //!< Which type of gamepad we are currently configuring.
		int m_ConfiguringGamepadIndex; //!< The index number of the configuring gamepad. See UInputMan::GetJoystickIndex for info.

		bool m_ConfiguringManually; //!< Indicates that the SettingsInputMappingWizardGUI needs to capture input because the player is configuring manually.
		bool m_ConfigFinished; //!< Whether the last input was mapped and the manual configuration mode is ready to apply the new InputScheme.
		int m_ConfigStep; //!< The current step in the manual configuration sequence.
		bool m_ConfigStepChange; //!< Whether a configuration step was completed and the sequence needs updating to start handling the next step.

		InputScheme m_NewInputScheme; //!< The new InputScheme that was configured in manual configuration mode.
		bool m_NewInputSchemeApplied; //!< Whether the new InputScheme was applied as the configuring player's active InputScheme.

		Timer m_BlinkTimer; //!< Timer for blinking the "Apply Changes" button and animating the recommended input diagram when configuring gamepads.

		std::vector<BITMAP*> m_DPadDiagramBitmaps; //!< Vector containing all the D-Pad type gamepad recommended input diagram bitmaps.
		std::vector<BITMAP*> m_DualAnalogDSDiagramBitmaps; //!< Vector containing all the DualShock type gamepad recommended input diagram bitmaps.
		std::vector<BITMAP*> m_DualAnalogXBDiagramBitmaps; //!< Vector containing all the Xbox type gamepad recommended input diagram bitmaps.

		WizardManualConfigScreen m_WizardManualConfigScreen; //!< The manual input configuration menu screen.
		WizardPresetSelectScreen m_WizardPresetSelectScreen; //!< The preset selection menu screen.

		/// GUI elements that compose the input mapping wizard menu screen.
		GUICollectionBox* m_InputWizardScreenBox;
		GUILabel* m_InputWizardTitleLabel;

#pragma region Create Breakdown
		/// Creates all the elements that compose the manual input configuration box.
		void CreateManualConfigScreen();

		/// Creates all the elements that compose the gamepad input preset selection box.
		void CreatePresetSelectionScreen();
#pragma endregion

#pragma region Input Mapping Wizard Handling
		/// Makes the manual input configuration menu screen visible to be interacted with by the player.
		void ShowManualConfigScreen();

		/// Makes the gamepad input preset selection menu screen visible to be interacted with by the player.
		void ShowPresetSelectionScreen();

		/// Clears the InputScheme that was configured during manual configuration and resets the sequence to the first step.
		void ResetManualConfigSequence();

		/// Applies the manually configured InputScheme as the active InputScheme of the configuring player.
		void ApplyManuallyConfiguredScheme();

		/// Applies a gamepad InputScheme preset as the active InputScheme of the configuring player.
		void ApplyGamepadInputPreset(GamepadType gamepadType);
#pragma endregion

#pragma region Input Event Handling Breakdown
		/// Handles the player interaction with the SettingsInputMappingWizardGUI's WizardManualConfigScreen GUI elements.
		/// @param guiEvent The GUIEvent containing information about the player interaction with an element.
		void HandleManualConfigScreenInputEvents(GUIEvent& guiEvent);

		/// Handles the player interaction with the SettingsInputMappingWizardGUI's WizardPresetSelectScreen GUI elements.
		/// @param guiEvent The GUIEvent containing information about the player interaction with an element.
		void HandlePresetSelectScreenInputEvents(GUIEvent& guiEvent);
#pragma endregion

#pragma region Input Configuration Sequence Handling Breakdown
		/// Handles step changes in the manual input configuration sequence.
		void HandleManualConfigStepChange();

		/// Handles capturing input and progressing the keyboard only configuration sequence.
		/// @return Whether input was captured and the sequence needs to progress.
		bool UpdateKeyboardConfigSequence();

		/// Handles capturing input and progressing the mouse and keyboard configuration sequence.
		/// @return Whether input was captured and the sequence needs to progress.
		bool UpdateMouseAndKeyboardConfigSequence();

		/// Handles capturing input and progressing the D-Pad type gamepad configuration sequence.
		/// @return Whether input was captured and the sequence needs to progress.
		bool UpdateGamepadDPadConfigSequence();

		/// Handles capturing input and progressing the dual-analog type gamepad (DualShock/Xbox) configuration sequence.
		/// @return Whether input was captured and the sequence needs to progress.
		bool UpdateGamepadAnalogConfigSequence();
#pragma endregion

		/// Clears all the member variables of this SettingsInputMappingWizardGUI, effectively resetting the members of this object.
		void Clear();

		// Disallow the use of some implicit methods.
		SettingsInputMappingWizardGUI(const SettingsInputMappingWizardGUI& reference) = delete;
		SettingsInputMappingWizardGUI& operator=(const SettingsInputMappingWizardGUI& rhs) = delete;
	};
} // namespace RTE
