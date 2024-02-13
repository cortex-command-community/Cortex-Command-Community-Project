#pragma once

#include "SettingsInputMappingWizardGUI.h"

#include <array>
#include <memory>

namespace RTE {

	class GUIControlManager;
	class GUICollectionBox;
	class GUIComboBox;
	class GUILabel;
	class GUIButton;
	class GUIScrollbar;
	class GUIEvent;

	/// Handling for player input mapping settings through the game settings user interface.
	class SettingsInputMappingGUI {

	public:
#pragma region Creation
		/// Constructor method used to instantiate a SettingsInputMappingGUI object in system memory and make it ready for use.
		/// @param parentControlManager Pointer to the parent GUIControlManager which owns all the GUIControls of this SettingsInputMappingGUI. Ownership is NOT transferred!
		explicit SettingsInputMappingGUI(GUIControlManager* parentControlManager);
#pragma endregion

#pragma region Getters and Setters
		/// Gets whether this SettingsInputMappingGUI is currently visible and enabled.
		/// @return Whether this SettingsInputMappingGUI is currently visible and enabled.
		bool IsEnabled() const;

		/// Enables or disables the SettingsInputMappingGUI.
		/// @param enable Show and enable or hide and disable the SettingsInputMappingGUI.
		/// @param player The player this SettingsInputMappingGUI is configuring input mapping for.
		void SetEnabled(bool enable = true, int player = 0);

		/// Gets the currently active GUICollectionBox that acts as a dialog box and requires disabling navigation and drawing an overlay.
		/// @return Pointer to the GUICollectionBox that is the currently active dialog box, if any. Ownership is NOT transferred!
		GUICollectionBox* GetActiveDialogBox() const;

		/// Closes the currently active GUICollectionBox that acts as a dialog box by hiding it. If the active dialog box is a sub-menu, disables it.
		void CloseActiveDialogBox();

		/// Gets whether this SettingsInputMappingGUI needs to capture input for manual configuration.
		/// @return Whether this SettingsInputMappingGUI needs to capture input for manual configuration.
		bool IsConfiguringManually() const;

		/// Gets the SettingsInputMappingWizardGUI of this SettingsInputMappingGUI.
		/// @return Pointer to the SettingsInputMappingWizardGUI of this SettingsInputMappingGUI. Ownership is NOT transferred!
		SettingsInputMappingWizardGUI* GetInputConfigWizardMenu() { return m_InputConfigWizardMenu.get(); }
#pragma endregion

#pragma region Concrete Methods
		/// Handles the player interaction with the SettingsInputMappingGUI GUI elements.
		/// @param guiEvent The GUIEvent containing information about the player interaction with an element.
		void HandleInputEvents(GUIEvent& guiEvent);

		/// Handles capturing input and updating the manual input configuration sequence.
		void HandleManualConfigSequence();
#pragma endregion

	private:
		static std::array<InputElements, 7> m_InputElementsUsedByMouse; //!< Array containing InputElements that are hard mapped to mouse controls when using mouse + keyboard.

		GUIControlManager* m_GUIControlManager; //!< The GUIControlManager which holds all the GUIControls of this menu. Not owned by this.

		Players m_ConfiguringPlayer; //!< The player this SettingsInputMappingGUI is configuring input mapping for.
		InputScheme* m_ConfiguringPlayerInputScheme; //!< The InputScheme of the configuring player.

		bool m_ConfiguringManually; //!< Indicates that the SettingsInputMappingGUI needs to capture input because the player is configuring manually.
		InputElements m_InputElementCapturingInput; //!< The InputElement in the configuring player's InputScheme that is currently being configured and is capturing input.

		int m_LastInputMapScrollingBoxScrollbarValue; //!< The previous value of the input mappings scrolling box scrollbar. Used to calculate the scroll position.

		std::unique_ptr<SettingsInputMappingWizardGUI> m_InputConfigWizardMenu; //!< The input mapping config wizard.

		/// GUI elements that compose the input mapping settings menu screen.
		GUICollectionBox* m_InputMappingSettingsBox;
		GUILabel* m_InputMappingSettingsLabel;
		GUIButton* m_CloseMappingBoxButton;
		GUIButton* m_RunConfigWizardButton;
		GUICollectionBox* m_InputMapScrollingBox;
		GUIScrollbar* m_InputMapScrollingBoxScrollbar;
		GUICollectionBox* m_InputMappingCaptureBox;
		GUIButton* m_InputElementCapturingInputNameLabel;
		std::array<GUILabel*, InputElements::INPUT_COUNT> m_InputMapLabel;
		std::array<GUIButton*, InputElements::INPUT_COUNT> m_InputMapButton;

#pragma region Input Mapping Settings Handling
		/// Shows and enables the input mapping capture box, starting the input capture sequence.
		/// @param inputElement
		void ShowInputMappingCaptureBox(InputElements inputElement);

		/// Hides and disables the input mapping capture box, ending the input capture sequence.
		void HideInputMappingCaptureBox();

		/// Updates the mapping button key labels with the configuring player's InputScheme mappings.
		void UpdateMappingButtonLabels();

		/// Updates the input mapping scrolling box scroll position.
		void UpdateScrollingInputBoxScrollPosition();
#pragma endregion

		// Disallow the use of some implicit methods.
		SettingsInputMappingGUI(const SettingsInputMappingGUI& reference) = delete;
		SettingsInputMappingGUI& operator=(const SettingsInputMappingGUI& rhs) = delete;
	};
} // namespace RTE
