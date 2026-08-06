#pragma once

#include "Singleton.h"
#include "AllegroBitmap.h"

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
		void Create(AllegroScreen* guiScreen, GUIInputWrapper* guiInput);

		/// Sets the static background layer position to be centered, for use during init and on resolution change
		void CenterLoadingSplash();

#pragma endregion

#pragma region Destruction
		/// Destroys and resets (through Clear()) the LoadingScreen object.
		void Destroy();
#pragma endregion

#pragma region Concrete Methods
		/// Updates the loading progress report bitmap. To be called repeatedly during game loading.
		void UpdateWithProgressReport();

		/// Draws the loading splash to the screen.
		void DrawLoadingSplash();
#pragma endregion

	private:

		void UpdateWithProgressReport_AcquireEntries();

		void UpdateWithProgressReport_DrawOntoFramebuffer();

		BITMAP* m_ProgressBitmap; //!< Bitmap onto which progress text is drawn.
		AllegroBitmap* m_ProgressBitmapAllegro; //!< Progress text bitmap wrapper for the drawing functions.
		std::unique_ptr<StaticSceneLayer> m_LoadingBackground; //!< Loading Screen Background image.

		struct ProgressEntry {
			std::string str;
			bool done = false;
			float opacity = 0.1f;
		};

		int m_ProgressTextYOffset = 0;
		const int m_ProgressTextLineSpacing = 12;

		std::vector<ProgressEntry> m_ProgressEntries;

		/// Clears all the member variables of this LoadingScreen, effectively resetting the members of this abstraction level only.
		void Clear();

		// Disallow the use of some implicit methods.
		LoadingScreen(const LoadingScreen& reference) = delete;
		LoadingScreen& operator=(const LoadingScreen& rhs) = delete;
	};
} // namespace RTE
