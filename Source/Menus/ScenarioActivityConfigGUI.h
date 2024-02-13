#pragma once

#include "Activity.h"

#include <array>

namespace RTE {

	class GameActivity;
	class AllegroBitmap;
	class GUIControlManager;
	class GUICollectionBox;
	class GUIComboBox;
	class GUICheckbox;
	class GUIButton;
	class GUILabel;
	class GUISlider;
	class GUIEvent;

	/// Handling for the scenario Activity configuration screen composition and interaction.
	class ScenarioActivityConfigGUI {

	public:
#pragma region Creation
		/// Constructor method used to instantiate a ScenarioActivityConfigGUI object in system memory and make it ready for use.
		/// @param parentControlManager Pointer to the parent GUIControlManager which owns all the GUIControls of this ScenarioActivityConfigGUI. Ownership is NOT transferred!
		explicit ScenarioActivityConfigGUI(GUIControlManager* parentControlManager);
#pragma endregion

#pragma region Getters and Setters
		/// Gets whether this ScenarioActivityConfigGUI is currently visible and enabled.
		/// @return Whether this ScenarioActivityConfigGUI is currently visible and enabled.
		bool IsEnabled() const;

		/// Enables or disables the ScenarioActivityConfigGUI.
		/// @param enable Show and enable or hide and disable the ScenarioActivityConfigGUI.
		/// @param selectedActivity Pointer to the Activity this ScenarioActivityConfigGUI will be configuring for.
		/// @param selectedScene Pointer to the Scene the passed in Activity will be using.
		void SetEnabled(bool enable, const Activity* selectedActivity = nullptr, Scene* selectedScene = nullptr);
#pragma endregion

#pragma region Concrete Methods
		/// Updates the ScenarioActivityConfigGUI state.
		/// @param mouseX Mouse X position.
		/// @param mouseY Mouse Y position.
		/// @return Whether the player started a new game through the ScenarioActivityConfigGUI.
		bool Update(int mouseX, int mouseY);

		/// Draws the ScenarioActivityConfigGUI to the screen.
		void Draw();
#pragma endregion

	private:
		/// Enumeration for all the player columns in the player setup box. "Extends" the Players enumeration by adding an entry for the CPU player.
		enum PlayerColumns {
			PlayerCPU = Players::MaxPlayerCount,
			PlayerColumnCount
		};

		/// Enumeration for all the team rows in the player setup box. "Extends" the Teams enumeration by adding an entry for unused (disabled) Team.
		enum TeamRows {
			DisabledTeam = Activity::Teams::MaxTeamCount,
			TeamRowCount
		};

		GUIControlManager* m_GUIControlManager; //!< The GUIControlManager which holds all the GUIControls of this menu. Not owned by this.

		const GameActivity* m_SelectedActivity; //!< The Activity this ScenarioActivityConfigGUI is configuring.
		const GameActivity* m_PreviouslySelectedActivity; //!< The Activity this ScenarioActivityConfigGUI was configuring last, before it got was disabled.
		Scene* m_SelectedScene; //!< The Scene the selected Activity will be using.
		int m_LockedCPUTeam = Activity::Teams::NoTeam; //!< Which team the CPU is locked to, if any.

		bool m_StartingGoldAdjustedManually; //!< Whether the player adjusted the starting gold, meaning it should stop automatically adjusting to the difficulty setting default starting gold where applicable.

		Timer m_StartGameButtonBlinkTimer; //!< Timer for blinking the start game button.

		bool m_TechListFetched; //!< Whether the tech list was fetched and each team's ComboBox was populated with it, even if no valid tech modules were added.

		/// GUI elements that compose the Activity setup box.
		GUICollectionBox* m_ActivityConfigBox;
		GUILabel* m_StartErrorLabel;
		GUIButton* m_StartGameButton;
		GUIButton* m_CancelConfigButton;
		GUILabel* m_ActivityDifficultyLabel;
		GUISlider* m_ActivityDifficultySlider;
		GUILabel* m_StartingGoldLabel;
		GUISlider* m_StartingGoldSlider;
		GUICheckbox* m_RequireClearPathToOrbitCheckbox;
		GUICheckbox* m_FogOfWarCheckbox;
		GUICheckbox* m_DeployUnitsCheckbox;
		GUILabel* m_CPULockLabel;
		GUICollectionBox* m_PlayersAndTeamsConfigBox;
		std::array<GUICollectionBox*, TeamRows::TeamRowCount> m_TeamIconBoxes;
		std::array<GUILabel*, TeamRows::TeamRowCount> m_TeamNameLabels;
		std::array<std::array<GUICollectionBox*, TeamRows::TeamRowCount>, PlayerColumns::PlayerColumnCount> m_PlayerBoxes;
		std::array<GUIComboBox*, Activity::Teams::MaxTeamCount> m_TeamTechComboBoxes;
		std::array<GUILabel*, Activity::Teams::MaxTeamCount> m_TeamAISkillLabels;
		std::array<GUISlider*, Activity::Teams::MaxTeamCount> m_TeamAISkillSliders;

#pragma region Activity Configuration Screen Handling
		/// Fills each team's Tech ComboBox with all valid Tech DataModules.
		void PopulateTechComboBoxes();

		/// Resets the configuration screen to the selected Activity's default settings and enables/disables attribute settings accordingly, making the configuration screen ready for interaction.
		void ResetActivityConfigBox();

		/// Sets up and starts the currently selected Activity with the configured settings.
		void StartGame();

		/// Updates the starting gold slider to the Activity difficulty setting (when applicable) and updates the value in the label according to the value in the slider.
		/// @return
		void UpdateStartingGoldSliderAndLabel();

		/// Updates the currently hovered cell in the players and teams config box to apply the hovered visual and removes the hovered visual from any other cells. Also handles clicking on cells.
		/// @param mouseX Mouse X position.
		/// @param mouseY Mouse Y position.
		void UpdatePlayerTeamSetupCell(int mouseX, int mouseY);

		/// Handles the player interaction with a cell in the players and teams config box.
		/// @param clickedPlayer The player box that was clicked.
		/// @param clickedTeam The team box that was clicked.
		void HandleClickOnPlayerTeamSetupCell(int clickedPlayer, int clickedTeam);

		/// Handles the player interaction with the ScenarioActivityConfigGUI GUI elements.
		/// @return Whether the player started a new game through the ScenarioActivityConfigGUI.
		bool HandleInputEvents();
#pragma endregion

		// Disallow the use of some implicit methods.
		ScenarioActivityConfigGUI(const ScenarioActivityConfigGUI& reference) = delete;
		ScenarioActivityConfigGUI& operator=(const ScenarioActivityConfigGUI& rhs) = delete;
	};
} // namespace RTE
