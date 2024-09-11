#pragma once

#include "GUIInput.h"
#include "SDL_scancode.h"

#include <array>
#include <memory>

namespace RTE {

	class Timer;

	/// Wrapper class to translate input handling of whatever library is currently used to valid GUI library input.
	class GUIInputWrapper : public GUIInput {

	public:
#pragma region Creation
		/// Constructor method used to instantiate a GUIInputWrapper object in system memory.
		/// @param whichPlayer Which player this GUIInputWrapper will handle input for. -1 means no specific player and will default to player 1.
		/// @param keyJoyMouseCursor Whether the keyboard and joysticks also can control the mouse cursor.
		GUIInputWrapper(int whichPlayer, bool keyJoyMouseCursor = false);
#pragma endregion

#pragma region Destruction
		/// Destructor method used to clean up a GUIInputWrapper object before deletion from system memory.
		~GUIInputWrapper() override = default;
#pragma endregion

#pragma region Virtual Override Methods
		/// Updates the input.
		void Update() override;
#pragma endregion

	private:
		const float m_KeyRepeatDelay = 0.10F; //!< The delay a key needs to be held to be considered a repeating input. TODO: Make this use proper OS repeating instead of this shit...
		std::array<float, GUIInput::Constants::KEYBOARD_BUFFER_SIZE> m_KeyHoldDuration; //!< How long each key has been held in order to set repeating inputs.

		std::unique_ptr<Timer> m_KeyTimer; //!< Timer for checking key hold duration.
		std::unique_ptr<Timer> m_CursorAccelTimer; //!< Timer to calculate the mouse cursor acceleration when it is controller with the keyboard or joysticks.

		/// Converts from SDL's key push to that used by this GUI lib, with timings for repeats taken into consideration.
		/// @param sdlKey The key scancode.
		/// @param guilibKey The corresponding GUIlib scancode
		/// @param elapsedS The elapsed time since the last update, in seconds.
		void ConvertKeyEvent(SDL_Scancode sdlKey, int guilibKey, float elapsedS);

#pragma region Update Breakdown
		/// Updates the keyboard input.
		void UpdateKeyboardInput(float keyElapsedTime);

		/// Updates the mouse input.
		void UpdateMouseInput();

		/// Updates the mouse input using the joystick or keyboard.
		void UpdateKeyJoyMouseInput(float keyElapsedTime);
#pragma endregion

		// Disallow the use of some implicit methods.
		GUIInputWrapper(const GUIInputWrapper& reference) = delete;
		GUIInputWrapper& operator=(const GUIInputWrapper& rhs) = delete;
	};
} // namespace RTE
