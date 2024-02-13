#pragma once

#include <string>
#include <vector>
#include <array>

namespace RTE {

	class GUIControlManager;
	class GUICollectionBox;
	class GUIComboBox;
	class GUILabel;
	class GUIButton;
	class GUIRadioButton;
	class GUICheckbox;
	class GUITextBox;
	class GUIEvent;

	/// Handling for video settings through the game settings user interface.
	class SettingsVideoGUI {

	public:
#pragma region Creation
		/// Constructor method used to instantiate a SettingsVideoGUI object in system memory and make it ready for use.
		/// @param parentControlManager Pointer to the parent GUIControlManager which owns all the GUIControls of this SettingsVideoGUI. Ownership is NOT transferred!
		explicit SettingsVideoGUI(GUIControlManager* parentControlManager);
#pragma endregion

#pragma region Getters and Setters
		/// Enables or disables the SettingsVideoGUI.
		/// @param enable Show and enable or hide and disable the SettingsVideoGUI.
		void SetEnabled(bool enable = true) const;

		/// Gets the currently active GUICollectionBox that acts as a dialog box and requires disabling navigation and drawing an overlay.
		/// @return Pointer to the GUICollectionBox that is the currently active dialog box, if any. Ownership is NOT transferred!
		GUICollectionBox* GetActiveDialogBox() const;

		/// Closes the currently active GUICollectionBox that acts as a dialog box by hiding it. If the active dialog box is a sub-menu, disables it.
		void CloseActiveDialogBox() const;
#pragma endregion

#pragma region Concrete Methods
		/// Handles the player interaction with the SettingsVideoGUI GUI elements.
		/// @param guiEvent The GUIEvent containing information about the player interaction with an element.
		void HandleInputEvents(GUIEvent& guiEvent);
#pragma endregion

	private:
		/// Enumeration for the different types of quick resolution change options.
		enum ResolutionQuickChangeType {
			Windowed,
			Fullscreen,
			UpscaledFullscreen,
			QuickChangeTypeCount
		};

		/// Struct containing information about a supported resolution preset.
		struct PresetResolutionRecord {
			int Width; //!< Resolution width.
			int Height; //!< Resolution height.
			float Scale; //!< Whether resolution is upscaled.

			/// Constructor method to instantiate a PresetResolutionRecord object in system memory and make it ready for use.
			/// @param width Resolution width.
			/// @param height Resolution height.
			/// @param upscaled Whether resolution is upscaled.
			PresetResolutionRecord(int width, int height, float scale) :
			    Width(width), Height(height), Scale(scale) {}

			/// Makes UI displayable string with resolution info.
			/// @return String with resolution info.
			std::string GetDisplayString() const;

			/// Comparison operator for eliminating duplicates and sorting in the temporary PresetResolutionRecord std::sets during PopulateResolutionsComboBox.
			/// @param rhs The PresetResolutionRecord to compare with.
			/// @return Bool with the result of the comparison.
			bool operator<(const PresetResolutionRecord& rhs) const {
				if (Width == rhs.Width && Height == rhs.Height) {
					return Scale > rhs.Scale;
				} else if (Width == rhs.Width) {
					return Height < rhs.Height;
				}
				return Width < rhs.Width;
			}
		};

		GUIControlManager* m_GUIControlManager; //!< The GUIControlManager which holds all the GUIControls of this menu. Not owned by this.

		std::vector<PresetResolutionRecord> m_PresetResolutions; //!< Contains PresetResolutionRecords for all the supported preset resolutions.

		int m_NewResX; //!< The new resolution width to use when changing resolution.
		int m_NewResY; //!< The new resolution height to use when changing resolution.
		float m_NewResMultiplier; //!< How much the new resolution should be upscaled when changing resolution.
		bool m_NewFullscreen; //!< Whether the game will be windowed or fullscreen.

		/// GUI elements that compose the video settings menu screen.
		GUICollectionBox* m_VideoSettingsBox;
		GUIRadioButton* m_TwoPlayerSplitscreenHSplitRadioButton;
		GUIRadioButton* m_TwoPlayerSplitscreenVSplitRadioButton;
		GUICheckbox* m_EnableVSyncCheckbox;
		GUICheckbox* m_FullscreenCheckbox;
		GUICheckbox* m_UseMultiDisplaysCheckbox;
		GUIRadioButton* m_PresetResolutionRadioButton;
		GUIRadioButton* m_CustomResolutionRadioButton;
		GUICollectionBox* m_PresetResolutionBox;
		GUIComboBox* m_PresetResolutionComboBox;
		GUIButton* m_PresetResolutionApplyButton;
		GUILabel* m_PresetResolutionMessageLabel;
		GUICollectionBox* m_CustomResolutionBox;
		GUITextBox* m_CustomResolutionWidthTextBox;
		GUITextBox* m_CustomResolutionHeightTextBox;
		GUIComboBox* m_CustomResolutionMultiplierComboBox;
		GUILabel* m_CustomResolutionMessageLabel;
		GUIButton* m_CustomResolutionApplyButton;
		GUICollectionBox* m_ResolutionChangeDialogBox;
		GUIButton* m_ResolutionChangeConfirmButton;
		GUIButton* m_ResolutionChangeCancelButton;
		std::array<GUIButton*, ResolutionQuickChangeType::QuickChangeTypeCount> m_ResolutionQuickToggleButtons;

#pragma region Create Breakdown
		/// Creates all the elements that compose the preset resolution selection box.
		void CreatePresetResolutionBox();

		/// Creates all the elements that compose the custom resolution controls box.
		void CreateCustomResolutionBox();
#pragma endregion

#pragma region Video Settings Handling
		/// Checks whether the passed in width and height values can be used as a valid resolution setting.
		/// @param width Resolution width.
		/// @param height Resolution height.
		/// @return Whether the resolution is supported or not.
		bool IsSupportedResolution(int width, int height) const;

		/// Fills the preset resolutions set and combo box with scaled resolutions down to c_MinRes. Defaults the combobox to the closest resolution to c_DefaultRes.
		void PopulateResolutionsComboBox();

		/// Creates Resolution multipliers down to c_DefaultRes.
		void PopulateResMultplierComboBox();

		/// Remaps the displays to get the new maximum resolution values to update the numeric limits on the custom resolution textboxes.
		void UpdateCustomResolutionLimits();

		/// Attempts to change the resolution using the new values set by this SettingsVideoGUI, or if an Activity is running, first prompts to end it.
		/// @param displaysWereMapped Whether displays were mapped during interaction with this SettingsVideoGUI.
		void ApplyNewResolution(bool displaysWereMapped = false);

		/// Attempts to change the resolution using the new values set by the appropriate quick change type.
		/// @param resolutionChangeType The type of quick resolution change to apply. See ResolutionQuickChangeType enumeration.
		void ApplyQuickChangeResolution(ResolutionQuickChangeType resolutionChangeType);

		/// Attempts to change the resolution using the new values set from the selected preset resolution.
		void ApplyPresetResolution();

		/// Attempts to change the resolution using the new values set from the custom resolution controls.
		void ApplyCustomResolution();
#pragma endregion

		// Disallow the use of some implicit methods.
		SettingsVideoGUI(const SettingsVideoGUI& reference) = delete;
		SettingsVideoGUI& operator=(const SettingsVideoGUI& rhs) = delete;
	};
} // namespace RTE
