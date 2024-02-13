#pragma once

#include "Singleton.h"

#include <deque>
#include <unordered_set>
#include <string>

#define g_ConsoleMan ConsoleMan::Instance()

namespace RTE {

	class GUIScreen;
	class GUIInput;
	class GUIControlManager;
	class GUICollectionBox;
	class GUITextBox;
	class GUILabel;

	/// The singleton manager of the lua console.
	class ConsoleMan : public Singleton<ConsoleMan> {
		friend class SettingsMan;

	public:
#pragma region Creation
		/// Constructor method used to instantiate a ConsoleMan object in system memory. Create() should be called before using the object.
		ConsoleMan();

		/// Makes the ConsoleMan object ready for use.
		/// @return An error return value signaling success or any particular failure. Anything below 0 is an error signal.
		int Initialize();
#pragma endregion

#pragma region Destruction
		/// Destructor method used to clean up a ConsoleMan object before deletion from system memory.
		~ConsoleMan();

		/// Destroys and resets (through Clear()) the ConsoleMan object.
		void Destroy();
#pragma endregion

#pragma region Getters and Setters
		/// Reports whether the console is enabled or not.
		/// @return Whether the console is enabled or not.
		bool IsEnabled() const { return m_ConsoleState == ConsoleState::Enabled || m_ConsoleState == ConsoleState::Enabling; }

		/// Enables or disables the console.
		/// @param enable Whether to enable or disable the console.
		void SetEnabled(bool enable = true);

		/// Reports whether the console is in read-only mode or not.
		/// @return Whether the console is in read-only mode or not.
		bool IsReadOnly() const { return m_ReadOnly; }

		/// Gets how much of the screen that the console is covering when opened.
		/// @return The ratio of the screen that is covered.
		float GetConsoleScreenSize() const { return m_ConsoleScreenRatio; }

		/// Sets how much of the screen that the console should cover when opened.
		/// @param screenRatio The ratio of the screen to cover. 0 - 1.0.
		void SetConsoleScreenSize(float screenRatio = 0.3F);

		/// Gets whether the console text is using the monospace font or the regular proportional one.
		/// @return Whether the console text is using the monospace font or the regular proportional one.
		bool GetConsoleUseMonospaceFont() const { return m_ConsoleUseMonospaceFont; }

		/// Sets whether the console text is using the monospace font and changes to the appropriate skin to apply the setting.
		/// @param useFont Whether to use the monospace font or not.
		void SetConsoleUseMonospaceFont(bool useFont);
#pragma endregion

#pragma region Logging
		/// Gets whether the loading warning log has any warnings logged or not.
		/// @return Whether the log has logged warnings.
		bool LoadWarningsExist() const { return !m_LoadWarningLog.empty(); }

		/// Adds a new file extension mismatch entry to the loading warning log.
		/// @param pathToLog The path that produced the warning.
		/// @param readerPosition The file and line currently being loaded.
		/// @param altFileExtension The alternative file extension to the path that produced the warning (e.g. if file is ".bmp", alternative extension is ".png").
		void AddLoadWarningLogExtensionMismatchEntry(const std::string& pathToLog, const std::string& readerPosition = "", const std::string& altFileExtension = "");

		/// Writes the entire loading warning log to a file.
		/// @param filePath The filename of the file to write to.
		void SaveLoadWarningLog(const std::string& filePath);

		/// Writes all the input strings to a log in the order they were entered.
		/// @param filePath The filename of the file to write to.
		void SaveInputLog(const std::string& filePath);

		/// Writes the entire console buffer to a file.
		/// @param filePath The filename of the file to write to.
		/// @return Whether writing to the file was successful.
		bool SaveAllText(const std::string& filePath);

		/// Clears all previous input.
		void ClearLog();
#pragma endregion

#pragma region Concrete Methods
		/// Prints a string into the console.
		/// @param stringToPrint The string to print.
		void PrintString(const std::string& stringToPrint);

		/// Opens the console and prints the shortcut help text.
		void ShowShortcuts();

		/// Updates the state of this ConsoleMan. Supposed to be done every frame before drawing.
		void Update();

		/// Draws this ConsoleMan's current graphical representation to a BITMAP of choice.
		/// @param targetBitmap A pointer to a BITMAP to draw on.
		void Draw(BITMAP* targetBitmap) const;
#pragma endregion

	protected:
		/// Enumeration for console states when enabling/disabling the console. NOTE: This can't be lower down because m_ConsoleState relies on this definition.
		enum ConsoleState {
			Enabling = 0,
			Enabled,
			Disabling,
			Disabled
		};

		ConsoleState m_ConsoleState; //!< Current state of the console.
		bool m_ReadOnly; //!< Read-only mode where console text input is disabled and controller input should be preserved.
		float m_ConsoleScreenRatio; //!< The ratio of the screen that the console should take up, from 0.1 to 1.0 (whole screen).

		GUIScreen* m_GUIScreen; //!< GUI Screen for use by the in-game GUI.
		GUIInput* m_GUIInput; //!< GUI Input controller.
		GUIControlManager* m_GUIControlManager; //!< Manager of the console GUI elements.
		GUICollectionBox* m_ParentBox; //!< Collection box of the console GUI.
		GUILabel* m_ConsoleText; //!< The label which presents the console output.
		GUITextBox* m_InputTextBox; //!< The TextBox which the user types in the edited line.

		int m_ConsoleTextMaxNumLines; //!< Maximum number of lines to display in the console text label.

		std::deque<std::string> m_OutputLog; //!< Log of all strings outputted by the console.
		std::deque<std::string> m_InputLog; //!< Log of previously entered input strings.
		std::deque<std::string>::iterator m_InputLogPosition; //!< Iterator to the current position in the log.
		std::unordered_set<std::string> m_LoadWarningLog; //!< Log for non-fatal errors produced during loading (e.g. used .bmp file extension to load a .png file).

		std::string m_LastInputString; //!< Place to save the last worked on input string before deactivating the console.
		short m_LastLogMove; //!< The last direction the log marker was moved. Needed so that changing directions won't need double tapping.

	private:
		bool m_ConsoleUseMonospaceFont; //!< Whether the console text is using the monospace font.

		/// Sets the console to read-only mode and enables it.
		void SetReadOnly();

#pragma region Update Breakdown
		/// Console open/close animation handling and GUI element enabling/disabling. This is called from Update().
		void ConsoleOpenClose();

		/// Executes the string currently in the console textbox or multiple strings if a newline character is found.
		/// The input string is saved to the input log if it's different from the previous string. This is called from Update().
		/// @param feedEmptyString Whether to just pass in an empty string to make a new line.
		void FeedString(bool feedEmptyString = false);

		/// Loads a previously entered console string from the input log when pressing up or down. This is called from Update().
		/// @param nextEntry Whether to load the next entry in the log (true) or the previous (false).
		void LoadLoggedInput(bool nextEntry);

		/// Removes any grave accents (`) that are pasted or typed into the textbox by opening/closing it. This is called from Update().
		void RemoveGraveAccents() const;
#pragma endregion

		/// Clears all the member variables of this ConsoleMan, effectively resetting the members of this abstraction level only.
		void Clear();

		// Disallow the use of some implicit methods.
		ConsoleMan(const ConsoleMan& reference) = delete;
		ConsoleMan& operator=(const ConsoleMan& rhs) = delete;
	};
} // namespace RTE
