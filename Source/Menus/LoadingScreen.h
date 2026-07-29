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
	class StaticSceneLayer;

	/// Handling for the loading screen composition and loading progress box when starting the game.
	class LoadingScreen : public Singleton<LoadingScreen> {

	public:
#pragma region Creation
		/// Constructor method used to instantiate a LoadingScreen object in system memory.
		LoadingScreen();

		/// Makes the LoadingScreen object ready for use.
		/// @param guiScreen Pointer to a GUIScreen interface that will be used by this LoadingScreen's GUIControlManager. Ownership is NOT transferred!
		/// @param guiInput Pointer to a GUIInput interface that will be used by this LoadingScreen's GUIControlManager. Ownership is NOT transferred!
		/// @param progressReportDisabled Whether the loading screen progress report is disabled meaning GUI elements and adjustments relevant to it can be skipped.
		void Create(AllegroScreen* guiScreen, GUIInputWrapper* guiInput, bool progressReportDisabled);

		/// gtodo Creates the loading splash screen and draws the composed frame to the LoadingSplashBitmap.
		/// @param xOffset Horizontal offset of the loading splash screen.
		void CenterLoadingSplash();

#pragma endregion

#pragma region Destruction
		/// Destroys and resets (through Clear()) the LoadingScreen object.
		void Destroy();
#pragma endregion

#pragma region Concrete Methods
		/// gtodo redesc Updates the loading progress report and draws it to the screen if not disabled through the settings file.
		/// @param reportString The string to print in the report and log.
		/// @param newItem Whether to start a new line in the log writer and to scroll the bitmap.
		void UpdateWithProgressReport();

		/// Draws the loading splash to the screen.
		void DrawLoadingSplash();
#pragma endregion

	private:

		void UpdateWithProgressReport_AcquireEntries();

		void UpdateWithProgressReport_DrawOntoFramebuffer();

		BITMAP* m_ProgressBitmap; //!< gtodo 
		std::unique_ptr<StaticSceneLayer> m_LoadingBackground; //!< Loading Screen Background image.

		struct ProgressEntry {
			std::string str;
			bool done = false;
			float opacity = 0.1f;
		};

		std::vector<ProgressEntry> m_ProgressEntries;

		/// Clears all the member variables of this LoadingScreen, effectively resetting the members of this abstraction level only.
		void Clear();

		// Disallow the use of some implicit methods.
		LoadingScreen(const LoadingScreen& reference) = delete;
		LoadingScreen& operator=(const LoadingScreen& rhs) = delete;
	};
} // namespace RTE
