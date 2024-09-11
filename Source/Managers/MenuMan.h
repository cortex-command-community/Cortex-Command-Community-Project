#pragma once

#include "Singleton.h"

#include <memory>

#define g_MenuMan MenuMan::Instance()

namespace RTE {

	class AllegroScreen;
	class GUIInputWrapper;
	class Controller;
	class TitleScreen;
	class MainMenuGUI;
	class ScenarioGUI;
	class PauseMenuGUI;

	/// The singleton manager responsible for handling all the out-of-game menu screens (main menu, scenario menu, etc.).
	class MenuMan : public Singleton<MenuMan> {

	public:
#pragma region Creation
		/// Constructor method used to instantiate a MenuMan object in system memory. Initialize() should be called before using the object.
		MenuMan() = default;

		/// Makes the MenuMan object ready for use.
		/// @param firstTimeInit Whether this is initializing for the first time, meaning the game is booting up, so the loading screen needs to be shown and all module loading should happen.
		void Initialize(bool firstTimeInit = true);

		/// Reinitializes all the Main Menu GUIs after a resolution change. Must be done otherwise the GUIs retain the original resolution settings and become all screwy.
		void Reinitialize();
#pragma endregion

#pragma region Concrete Methods
		/// Sets the appropriate TitleScreen transition before entering the menu loop.
		void HandleTransitionIntoMenuLoop();

		/// Updates the MenuMan state.
		/// @return Whether the MenuMan update has reached a state where the menu loop should be exited so the simulation loop can proceed.
		bool Update();

		/// Draws the MenuMan to the screen.
		void Draw() const;
#pragma endregion

	private:
		/// Enumeration for the different menu screens that are active based on transition states.
		enum ActiveMenu {
			MenusDisabled,
			MainMenuActive,
			ScenarioMenuActive,
			MetaGameMenuActive,
			PauseMenuActive,
		};

		ActiveMenu m_ActiveMenu; //!< The currently active menu screen that is being updated and drawn. See ActiveMenu enumeration.

		std::unique_ptr<GUIInputWrapper> m_GUIInput; //!< The GUIInput interface of this MenuMan.
		std::unique_ptr<AllegroScreen> m_GUIScreen; //!< The GUIScreen interface of this MenuMan.
		std::unique_ptr<Controller> m_MenuController; //!< A Controller to handle player input in menu screens that require it.

		std::unique_ptr<TitleScreen> m_TitleScreen; //!< The title screen.
		std::unique_ptr<MainMenuGUI> m_MainMenu; //!< The main menu screen.
		std::unique_ptr<ScenarioGUI> m_ScenarioMenu; //!< The scenario menu screen.
		std::unique_ptr<PauseMenuGUI> m_PauseMenu; //!< The game pause menu screen.

#pragma region Updates
		/// Sets the active menu screen to be enabled, updated and drawn to the screen, besides the title screen which is always active.
		void SetActiveMenu();

		/// Updates the main menu screen and handles the update results.
		/// @return Whether the program was set to be terminated by the user through the main menu screen.
		bool UpdateMainMenu() const;

		/// Updates the scenario menu screen and handles the update results.
		void UpdateScenarioMenu() const;

		/// Updates the MetaGame menu screen and handles the update results.
		/// @return Whether the program was set to be terminated by the user through the MetaGame menu screen.
		bool UpdateMetaGameMenu() const;

		/// Updates the pause menu screen and handles the update results.
		void UpdatePauseMenu() const;
#pragma endregion

		// Disallow the use of some implicit methods.
		MenuMan(const MenuMan& reference) = delete;
		MenuMan& operator=(const MenuMan& rhs) = delete;
	};
} // namespace RTE
