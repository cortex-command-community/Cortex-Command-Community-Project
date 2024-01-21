#pragma once

#include "Singleton.h"
#include "Activity.h"

#include "BS_thread_pool.hpp"

#define g_ActivityMan ActivityMan::Instance()

namespace RTE {

	/// The singleton manager of the Activities and rules of Cortex Command.
	class ActivityMan : public Singleton<ActivityMan> {
		friend class SettingsMan;

	public:
#pragma region Creation
		/// Constructor method used to instantiate an ActivityMan object in system memory. Create() should be called before using the object.
		ActivityMan();

		/// Makes the ActivityMan object ready for use.
		/// @return An error return value signaling success or any particular failure. Anything below 0 is an error signal.
		bool Initialize();
#pragma endregion

#pragma region Destruction
		/// Destructor method used to clean up an ActivityMan object before deletion from system memory.
		~ActivityMan();

		/// Destroys and resets (through Clear()) the ActivityMan object.
		void Destroy() { Clear(); }
#pragma endregion

#pragma region Getters and Setters
		/// Gets the currently active Activity. Won't be what has been set by SetStartActivity unless RestartActivity has been called since.
		/// @return The currently active Activity. Will be nullptr if no Activity is going.
		Activity* GetActivity() const { return m_Activity.get(); }

		/// Gets the async save game task.
		/// @return The savegame task.
		BS::multi_future<void>& GetSaveGameTask() { return m_SaveGameTask; }

		/// Indicates whether the game is currently running or not (not editing, over or paused).
		/// @return Whether the game is running or not.
		bool ActivityRunning() const { return m_Activity && m_Activity->IsRunning(); }

		/// Indicates whether the game is currently paused or not.
		/// @return Whether the game is paused or not.
		bool ActivityPaused() const { return !m_Activity || m_Activity->IsPaused(); }

		/// Gets whether we are currently in game (as in, not in the main menu or any other out-of-game menus), regardless of its state.
		/// @return Whether we are currently in game, regardless of it's state.
		bool IsInActivity() const { return m_InActivity; }

		/// Sets whether we are currently in game (as in, not in the main menu or any other out-of-game menus) or not.
		/// @param isInActivity In game or not.
		void SetInActivity(bool isInActivity) { m_InActivity = isInActivity; }

		/// Gets whether the current Activity needs to be restarted.
		/// @return Whether the current Activity needs to be restarted.
		bool ActivitySetToRestart() const { return m_ActivityNeedsRestart; }

		/// Sets whether the current Activity needs to be restarted.
		/// @param restartActivity Restart the Activity or not.
		void SetRestartActivity(bool restartActivity = true) { m_ActivityNeedsRestart = restartActivity; }

		/// Gets whether the game simulation needs to be started back up after the current Activity was unpaused.
		/// @return Whether the game simulation needs to be started back up after the current Activity was unpaused.
		bool ActivitySetToResume() const { return m_ActivityNeedsResume; }

		/// Sets the game simulation to be started back up after the current Activity was unpaused.
		/// @param resumingFromPauseMenu Whether the game simulation is being resumed from the pause menu.
		void SetResumeActivity(bool resumingFromPauseMenu = false) {
			m_ActivityNeedsResume = true;
			m_ResumingActivityFromPauseMenu = resumingFromPauseMenu;
		}

		/// Gets whether the pause menu should be skipped when the game simulation is paused.
		/// @return Whether the pause menu should be skipped when the game simulation is paused.
		bool SkipPauseMenuWhenPausingActivity() const { return m_SkipPauseMenuWhenPausingActivity; }
#pragma endregion

#pragma region Default Activity Handling
		/// Gets the type name of the default Activity to be loaded if nothing else is available.
		/// @return The default Activity type name.
		std::string GetDefaultActivityType() const { return m_DefaultActivityType; }

		/// Sets the type name of the default Activity to be loaded if nothing else is available.
		/// @param defaultActivityType The default Activity type name.
		void SetDefaultActivityType(const std::string_view& defaultActivityType) { m_DefaultActivityType = defaultActivityType; }

		/// Gets the name of the default Activity to be loaded if nothing else is available.
		/// @return The default Activity preset name.
		std::string GetDefaultActivityName() const { return m_DefaultActivityName; }

		/// Sets the preset name of the default Activity to be loaded if nothing else is available.
		/// @param defaultActivityName The default Activity preset name.
		void SetDefaultActivityName(const std::string_view& defaultActivityName) { m_DefaultActivityName = defaultActivityName; }

		/// Gets whether the intro and main menu should be skipped on game start and launch directly into the set default Activity instead.
		/// @return Whether the game is set to launch directly into the set default Activity or not.
		bool IsSetToLaunchIntoActivity() const { return m_LaunchIntoActivity; }

		/// Gets whether the intro and main menu should be skipped on game start and launch directly into the set editor Activity instead.
		/// @return Whether the game is set to launch directly into the set editor Activity or not.
		bool IsSetToLaunchIntoEditor() const { return m_LaunchIntoEditor; }

		/// Sets the name of the editor to launch directly into.
		/// @param editorName
		void SetEditorToLaunch(const std::string_view& editorName) {
			if (!editorName.empty()) {
				m_EditorToLaunch = editorName;
				m_LaunchIntoEditor = true;
			}
		}
#pragma endregion

#pragma region Saving and Loading
		/// A utility function we can call in the debugger quickwatch window to force an abort save to occur (great for force-saving the game when it crashes)
		/// @return Whether the game was successfully saved.
		bool ForceAbortSave();

		/// Saves the currently running Scene and Activity to a savegame file. Note this only works for GAScripted activities.
		/// @param fileName Path to the file.
		/// @return Whether the game was successfully saved.
		bool SaveCurrentGame(const std::string& fileName);

		/// Loads a saved game, and launches its Scene and Activity.
		/// @param fileName Path to the file.
		/// @return Whether or not the saved game was successfully loaded.
		bool LoadAndLaunchGame(const std::string& fileName);
#pragma endregion

#pragma region Activity Start Handling
		// TODO: Fix crappy naming. None of these actually start anything. Maybe "...ActivityToStart" instead of "...StartActivity".

		/// Gets the Activity that will be used in the next restart. Ownership is NOT transferred!
		/// @return The Activity to put into effect next time ResetActivity is called.
		Activity* GetStartActivity() const { return m_StartActivity.get(); }

		/// Sets a new Activity to copy for next restart. You have to use RestartActivity to get it going. Ownership IS transferred!
		/// @param newActivity The new Activity to put into effect next time ResetActivity is called.
		void SetStartActivity(Activity* newActivity);

		/// Loads the "Tutorial Mission" Scene and starts the Tutorial Activity.
		void SetStartTutorialActivity();

		/// Loads "Editor Scene" and starts the given editor Activity.
		/// @param editorToLaunch The editor name to put into effect next time ResetActivity is called.
		void SetStartEditorActivity(const std::string_view& editorToLaunch);

		/// Launch editor Activity specified in command-line argument.
		/// @return Whether a valid editor name was passed in and set to be launched next time ResetActivity is called.
		bool SetStartEditorActivitySetToLaunchInto();

		/// Loads "Multiplayer Scene" and starts the MultiplayerGame Activity.
		/// @return Whether the MultiplayerGame Activity was successfully created and set to be launched next time ResetActivity is called.
		bool SetStartMultiplayerActivity();

		/// Launch multiplayer server overview Activity.
		/// @return Whether the server overview Activity was successfully created and set to be launched next time ResetActivity is called.
		bool SetStartMultiplayerServerOverview();
#pragma endregion

#pragma region Concrete Methods
		/// Officially starts the Activity passed in. Ownership IS transferred!
		/// @param activity The new activity to start.
		/// @return An error return value signaling success or any particular failure. Anything below 0 is an error signal.@return
		int StartActivity(Activity* activity);

		/// Officially gets and starts the Activity described.
		/// @param className The class name of the Activity to start.
		/// @param presetName The PresetName of the Activity to start.
		/// @return An error return value signaling success or any particular failure. Anything below 0 is an error signal.
		int StartActivity(const std::string& className, const std::string& presetName);

		/// Pauses/unpauses the game and saving/resuming in-game music if possible, or queuing default music if not.
		/// @param pause Whether to pause the game or not.
		/// @param skipPauseMenu Whether the pause menu should be skipped when the game simulation is paused.
		void PauseActivity(bool pause = true, bool skipPauseMenu = false);

		/// Start the game simulation back up after the current Activity was unpaused.
		void ResumeActivity();

		/// Completely restarts whatever Activity was last started.
		/// @return An error return value signaling success or any particular failure. Anything below 0 is an error signal.
		bool RestartActivity();

		/// Forces the current game's end.
		void EndActivity() const;

		/// Only updates Global Scripts of the current activity with LateUpdate flag enabled.
		void LateUpdateGlobalScripts() const;

		/// Updates the state of this and the current Activity. Supposed to be done every frame before drawing.
		void Update();
#pragma endregion

	private:
		std::string m_DefaultActivityType; //!< The type name of the default Activity to be loaded if nothing else is available.
		std::string m_DefaultActivityName; //!< The preset name of the default Activity to be loaded if nothing else is available.

		std::unique_ptr<Activity> m_Activity; //!< The currently active Activity.
		std::unique_ptr<Activity> m_StartActivity; //!< The starting condition of the next Activity to be (re)started.

		BS::multi_future<void> m_SaveGameTask; //!< The current save game task.

		bool m_InActivity; //!< Whether we are currently in game (as in, not in the main menu or any other out-of-game menus), regardless of its state.
		bool m_ActivityNeedsRestart; //!< Whether the current Activity needs to be restarted.
		bool m_ActivityNeedsResume; //!< Whether the game simulation needs to be started back up after the current Activity was unpaused.
		bool m_ResumingActivityFromPauseMenu; //!< Whether the game simulation is being resumed from the pause menu.
		bool m_SkipPauseMenuWhenPausingActivity; //!< Whether the pause menu should be skipped when the game simulation is paused.

		std::string m_LastMusicPath; //!< Path to the last music stream being played.
		float m_LastMusicPos; //!< What the last position of the in-game music track was before pause, in seconds.

		bool m_LaunchIntoActivity; //!< Whether to skip the intro and main menu and launch directly into the set default Activity instead.
		bool m_LaunchIntoEditor; //!< Whether to skip the intro and main menu and launch directly into the set editor Activity instead.
		std::string_view m_EditorToLaunch; //!< The name of the editor Activity to launch directly into.

		/// Clears all the member variables of this ActivityMan, effectively resetting the members of this abstraction level only.
		void Clear();

		// Disallow the use of some implicit methods.
		ActivityMan(const ActivityMan& reference) = delete;
		ActivityMan& operator=(const ActivityMan& rhs) = delete;
	};
} // namespace RTE
