#pragma once

#include "Singleton.h"

#include <memory>
#include <string>

#define g_LoadingScreen LoadingScreen::Instance()

namespace RTE {

	class AllegroScreen;
	class GUIInputWrapper;
	class GUIControlManager;
	class Writer;

	/// Handling for the loading screen composition and loading progress box when starting the game.
	class LoadingScreen : public Singleton<LoadingScreen> {

	public:
#pragma region Creation
		/// Constructor method used to instantiate a LoadingScreen object in system memory.
		LoadingScreen() { Clear(); }

		/// Makes the LoadingScreen object ready for use.
		/// @param guiScreen Pointer to a GUIScreen interface that will be used by this LoadingScreen's GUIControlManager. Ownership is NOT transferred!
		/// @param guiInput Pointer to a GUIInput interface that will be used by this LoadingScreen's GUIControlManager. Ownership is NOT transferred!
		/// @param progressReportDisabled Whether the loading screen progress report is disabled meaning GUI elements and adjustments relevant to it can be skipped.
		void Create(AllegroScreen* guiScreen, GUIInputWrapper* guiInput, bool progressReportDisabled);

		/// Creates the loading splash screen and draws the composed frame to the LoadingSplashBitmap.
		/// @param xOffset Horizontal offset of the loading splash screen.
		void CreateLoadingSplash(int xOffset = 0);

		/// Creates the GUIListBox that the progress report will be drawn to, if not disabled through the settings file to speed up loading times.
		/// As it turned out, a massive amount of time is spent updating the GUI control and flipping the frame buffers.
		/// @param parentControlManager Pointer to the parent GUIControlManager which owns all the GUIControls of this LoadingScreen. Ownership is NOT transferred!
		void CreateProgressReportListbox(GUIControlManager* parentControlManager);
#pragma endregion

#pragma region Destruction
		/// Destroys and resets (through Clear()) the LoadingScreen object.
		void Destroy();
#pragma endregion

#pragma region Concrete Methods
		/// Updates the loading progress report and draws it to the screen if not disabled through the settings file.
		/// @param reportString The string to print in the report and log.
		/// @param newItem Whether to start a new line in the log writer and to scroll the bitmap.
		static void LoadingSplashProgressReport(const std::string& reportString, bool newItem = false);

		/// Draws the loading splash to the screen.
		void DrawLoadingSplash();
#pragma endregion

	private:
		std::unique_ptr<Writer> m_LoadingLogWriter; //!< The Writer that generates the loading log.

		BITMAP* m_LoadingSplashBitmap; //!< BITMAP that is used for drawing the splash screen.
		BITMAP* m_ProgressListboxBitmap; //!< BITMAP that the progress report will be drawn into.
		int m_ProgressListboxPosX; //!< Position of the progress report box on X axis.
		int m_ProgressListboxPosY; //!< Position of the progress report box on Y axis.

		/// Clears all the member variables of this LoadingScreen, effectively resetting the members of this abstraction level only.
		void Clear();

		// Disallow the use of some implicit methods.
		LoadingScreen(const LoadingScreen& reference) = delete;
		LoadingScreen& operator=(const LoadingScreen& rhs) = delete;
	};
} // namespace RTE
