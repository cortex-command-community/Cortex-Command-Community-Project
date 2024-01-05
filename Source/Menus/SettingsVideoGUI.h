#ifndef _RTESETTINGSVIDEOGUI_
#define _RTESETTINGSVIDEOGUI_

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

	/// <summary>
	/// Handling for video settings through the game settings user interface.
	/// </summary>
	class SettingsVideoGUI {

	public:

#pragma region Creation
		/// <summary>
		/// Constructor method used to instantiate a SettingsVideoGUI object in system memory and make it ready for use.
		/// </summary>
		/// <param name="parentControlManager">Pointer to the parent GUIControlManager which owns all the GUIControls of this SettingsVideoGUI. Ownership is NOT transferred!</param>
		explicit SettingsVideoGUI(GUIControlManager *parentControlManager);
#pragma endregion

#pragma region Getters and Setters
		/// <summary>
		/// Enables or disables the SettingsVideoGUI.
		/// </summary>
		/// <param name="enable">Show and enable or hide and disable the SettingsVideoGUI.</param>
		void SetEnabled(bool enable = true) const;

		/// <summary>
		/// Gets the currently active GUICollectionBox that acts as a dialog box and requires disabling navigation and drawing an overlay.
		/// </summary>
		/// <returns>Pointer to the GUICollectionBox that is the currently active dialog box, if any. Ownership is NOT transferred!</returns>
		GUICollectionBox * GetActiveDialogBox() const;

		/// <summary>
		/// Closes the currently active GUICollectionBox that acts as a dialog box by hiding it. If the active dialog box is a sub-menu, disables it.
		/// </summary>
		void CloseActiveDialogBox() const;
#pragma endregion

#pragma region Concrete Methods
		/// <summary>
		/// Handles the player interaction with the SettingsVideoGUI GUI elements.
		/// </summary>
		/// <param name="guiEvent">The GUIEvent containing information about the player interaction with an element.</param>
		void HandleInputEvents(GUIEvent &guiEvent);
#pragma endregion

	private:

		/// <summary>
		/// Enumeration for the different types of quick resolution change options.
		/// </summary>
		enum ResolutionQuickChangeType {
			Windowed,
			Fullscreen,
			UpscaledFullscreen,
			QuickChangeTypeCount
		};

		/// <summary>
		/// Struct containing information about a supported resolution preset.
		/// </summary>
		struct PresetResolutionRecord {
			int Width; //!< Resolution width.
			int Height; //!< Resolution height.
			float Scale; //!< Whether resolution is upscaled.

			/// <summary>
			/// Constructor method to instantiate a PresetResolutionRecord object in system memory and make it ready for use.
			/// </summary>
			/// <param name="width">Resolution width.</param>
			/// <param name="height">Resolution height.</param>
			/// <param name="upscaled">Whether resolution is upscaled.</param>
			PresetResolutionRecord(int width, int height, float scale) : Width(width), Height(height), Scale(scale) {}

			/// <summary>
			/// Makes UI displayable string with resolution info.
			/// </summary>
			/// <returns>String with resolution info.</returns>
			std::string GetDisplayString() const;

			/// <summary>
			/// Comparison operator for eliminating duplicates and sorting in the temporary PresetResolutionRecord std::sets during PopulateResolutionsComboBox.
			/// </summary>
			/// <param name="rhs">The PresetResolutionRecord to compare with.</param>
			/// <returns>Bool with the result of the comparison.</returns>
			bool operator<(const PresetResolutionRecord &rhs) const {
				if (Width == rhs.Width && Height == rhs.Height) {
					return Scale > rhs.Scale;
				} else if (Width == rhs.Width) {
					return Height < rhs.Height;
				}
				return Width < rhs.Width;
			}
		};

		GUIControlManager *m_GUIControlManager; //!< The GUIControlManager which holds all the GUIControls of this menu. Not owned by this.

		std::vector<PresetResolutionRecord> m_PresetResolutions; //!< Contains PresetResolutionRecords for all the supported preset resolutions.

		int m_NewResX; //!< The new resolution width to use when changing resolution.
		int m_NewResY; //!< The new resolution height to use when changing resolution.
		float m_NewResMultiplier; //!< How much the new resolution should be upscaled when changing resolution.
		bool m_NewFullscreen;  //!< Whether the game will be windowed or fullscreen.

		/// <summary>
		/// GUI elements that compose the video settings menu screen.
		/// </summary>
		GUICollectionBox *m_VideoSettingsBox;
		GUIRadioButton *m_TwoPlayerSplitscreenHSplitRadioButton;
		GUIRadioButton *m_TwoPlayerSplitscreenVSplitRadioButton;
		GUICheckbox *m_EnableVSyncCheckbox;
		GUICheckbox *m_FullscreenCheckbox;
		GUICheckbox *m_UseMultiDisplaysCheckbox;
		GUIRadioButton *m_PresetResolutionRadioButton;
		GUIRadioButton *m_CustomResolutionRadioButton;
		GUICollectionBox *m_PresetResolutionBox;
		GUIComboBox *m_PresetResolutionComboBox;
		GUIButton *m_PresetResolutionApplyButton;
		GUILabel *m_PresetResolutionMessageLabel;
		GUICollectionBox *m_CustomResolutionBox;
		GUITextBox *m_CustomResolutionWidthTextBox;
		GUITextBox *m_CustomResolutionHeightTextBox;
		GUIComboBox *m_CustomResolutionMultiplierComboBox;
		GUILabel *m_CustomResolutionMessageLabel;
		GUIButton *m_CustomResolutionApplyButton;
		GUICollectionBox *m_ResolutionChangeDialogBox;
		GUIButton *m_ResolutionChangeConfirmButton;
		GUIButton *m_ResolutionChangeCancelButton;
		std::array<GUIButton *, ResolutionQuickChangeType::QuickChangeTypeCount> m_ResolutionQuickToggleButtons;

#pragma region Create Breakdown
		/// <summary>
		/// Creates all the elements that compose the preset resolution selection box.
		/// </summary>
		void CreatePresetResolutionBox();

		/// <summary>
		/// Creates all the elements that compose the custom resolution controls box.
		/// </summary>
		void CreateCustomResolutionBox();
#pragma endregion

#pragma region Video Settings Handling
		/// <summary>
		/// Checks whether the passed in width and height values can be used as a valid resolution setting.
		/// </summary>
		/// <param name="width">Resolution width.</param>
		/// <param name="height">Resolution height.</param>
		/// <returns>Whether the resolution is supported or not.</returns>
		bool IsSupportedResolution(int width, int height) const;

		/// <summary>
		/// Fills the preset resolutions set and combo box with scaled resolutions down to c_MinRes. Defaults the combobox to the closest resolution to c_DefaultRes.
		/// </summary>
		void PopulateResolutionsComboBox();

		/// <summary>
		/// Creates Resolution multipliers down to c_DefaultRes.
		/// </summary>
		void PopulateResMultplierComboBox();

		/// <summary>
		/// Remaps the displays to get the new maximum resolution values to update the numeric limits on the custom resolution textboxes.
		/// </summary>
		void UpdateCustomResolutionLimits();

		/// <summary>
		/// Attempts to change the resolution using the new values set by this SettingsVideoGUI, or if an Activity is running, first prompts to end it.
		/// </summary>
		/// <param name="displaysWereMapped">Whether displays were mapped during interaction with this SettingsVideoGUI.</param>
		void ApplyNewResolution(bool displaysWereMapped = false);

		/// <summary>
		/// Attempts to change the resolution using the new values set by the appropriate quick change type.
		/// </summary>
		/// <param name="resolutionChangeType">The type of quick resolution change to apply. See ResolutionQuickChangeType enumeration.</param>
		void ApplyQuickChangeResolution(ResolutionQuickChangeType resolutionChangeType);

		/// <summary>
		/// Attempts to change the resolution using the new values set from the selected preset resolution.
		/// </summary>
		void ApplyPresetResolution();

		/// <summary>
		/// Attempts to change the resolution using the new values set from the custom resolution controls.
		/// </summary>
		void ApplyCustomResolution();
#pragma endregion

		// Disallow the use of some implicit methods.
		SettingsVideoGUI(const SettingsVideoGUI &reference) = delete;
		SettingsVideoGUI & operator=(const SettingsVideoGUI &rhs) = delete;
	};
}
#endif
