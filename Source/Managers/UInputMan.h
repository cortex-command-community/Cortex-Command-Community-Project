#pragma once

#include "Constants.h"
#include "SDL3/SDL_joystick.h"
#include "SDL3/SDL_keycode.h"
#include "SDL3/SDL_scancode.h"
#include "Singleton.h"
#include "Vector.h"
#include "InputScheme.h"
#include "Gamepad.h"
#include "allegro/keyboard.h"
#include <SDL3/SDL_keyboard.h>
#include <SDL3/SDL_events.h>

#include <algorithm>
#include <array>
#include <string>
#include <vector>
#include <optional>
#include <functional>

#define g_UInputMan UInputMan::Instance()

extern "C" {
struct SDL_Rect;
}

namespace RTE {

	class Icon;

	/// The singleton manager responsible for handling user input.
	class UInputMan : public Singleton<UInputMan> {
		friend class SettingsMan;

	public:
		/// Enumeration for the mouse cursor actions in menus.
		enum MenuCursorButtons {
			MENU_PRIMARY,
			MENU_SECONDARY,
			MENU_EITHER
		};

#pragma region Creation
		/// Constructor method used to instantiate a UInputMan object in system memory. Create() should be called before using the object.
		UInputMan();

		/// Makes the UInputMan object ready for use.
		/// @return An error return value signaling success or any particular failure. Anything below 0 is an error signal.
		int Initialize();
#pragma endregion

#pragma region Destruction
		/// Destructor method used to clean up a UInputMan object before deletion from system memory.
		~UInputMan();

		/// Destroys and resets (through Clear()) the UInputMan object.
		void Destroy() { Clear(); }
#pragma endregion

#pragma region Concrete Methods
		/// Loads the input device icons from loaded presets. Can't do this during Create() because the presets don't exist so this will be called from MenuMan::Initialize() after modules are loaded.
		void LoadDeviceIcons();

		/// Adds an (input) SDL_Event to the Event queue for processing on Update.
		/// @param inputEvent The SDL input event to queue.
		void HandleInputEvent(const SDL_Event& inputEvent);

		/// Updates the state of this UInputMan. Supposed to be done every frame.
		/// @return An error return value signaling success or any particular failure. Anything below 0 is an error signal.
		int Update();

		/// Resets the changed states for keyboard and mouse events.
		void EndFrame();
#pragma endregion

#pragma region Control Scheme and Input Mapping Handling
		/// Sets whether to skip handling any special input (F1-F12, etc.) to avoid shenanigans during manual input mapping.
		/// @param skip Whether to skip handling special input or not.
		void SetSkipHandlingSpecialInput(bool skip) { m_SkipHandlingSpecialInput = skip; }

		/// Gets the currently used input device of the specified player.
		/// @param whichPlayer Which player to get input device for.
		/// @return A number value representing the currently used input device of this player. See InputDevice enumeration for values.
		int GetInputDevice(int whichPlayer) const { return m_ControlScheme.at(whichPlayer).GetDevice(); }

		/// Access a specific player's control scheme.
		/// @param whichPlayer Which player to get the scheme for.
		/// @return A pointer to the requested player's control scheme. Ownership is NOT transferred!
		InputScheme* GetControlScheme(int whichPlayer) { return &m_ControlScheme.at(whichPlayer); }

		/// Get the current device Icon of a specific player's scheme.
		/// @param whichPlayer Which player to get the scheme device icon of.
		/// @return A const pointer to the requested player's control scheme icon. Ownership is NOT transferred!
		const Icon* GetSchemeIcon(int whichPlayer) const { return (whichPlayer < Players::PlayerOne || whichPlayer >= Players::MaxPlayerCount) ? nullptr : m_DeviceIcons[m_ControlScheme.at(whichPlayer).GetDevice()]; }

		/// Get the current device Icon of a specific device.
		/// @param whichDevice Which device to get the icon of.
		/// @return A const pointer to the requested device's control scheme icon. Ownership is NOT transferred!
		const Icon* GetDeviceIcon(int whichDevice) const { return (whichDevice < InputDevice::DEVICE_KEYB_ONLY || whichDevice > InputDevice::DEVICE_GAMEPAD_4) ? nullptr : m_DeviceIcons[whichDevice]; }
#pragma endregion

#pragma region General Input Handling
		/// Gets the last device which affected GUI cursor position.
		/// @return The last device which affected GUI cursor position.
		InputDevice GetLastDeviceWhichControlledGUICursor() const { return m_LastDeviceWhichControlledGUICursor; }

		/// Gets the analog moving values of a specific player's control scheme.
		/// @param whichPlayer Which player to check for.
		/// @return The analog axis values ranging between -1.0 to 1.0, in both axes.
		Vector AnalogMoveValues(int whichPlayer = 0);

		/// Gets the analog aiming values of a specific player's control scheme.
		/// @param whichPlayer Which player to check for.
		/// @return The analog axis values ranging between -1.0 to 1.0, in both axes.
		Vector AnalogAimValues(int whichPlayer = 0);

		/// Gets whether a specific input element was held during the last update.
		/// @param whichPlayer Which player to check for.
		/// @param whichElement Which element to check for.
		/// @return Whether the element is held or not.
		bool ElementHeld(int whichPlayer, int whichElement) { return GetInputElementState(whichPlayer, whichElement, InputState::Held); }

		/// Gets whether a specific input element was depressed between the last update and the one previous to it.
		/// @param whichPlayer Which player to check for.
		/// @param whichElement Which element to check for.
		/// @return Whether the element is pressed or not.
		bool ElementPressed(int whichPlayer, int whichElement) { return GetInputElementState(whichPlayer, whichElement, InputState::Pressed); }

		/// Gets whether a specific input element was released between the last update and the one previous to it.
		/// @param whichPlayer Which player to check for.
		/// @param whichElement Which element to check for.
		/// @return Whether the element is released or not.
		bool ElementReleased(int whichPlayer, int whichElement) { return GetInputElementState(whichPlayer, whichElement, InputState::Released); }

		/// Gets the generic direction input from one or all players which can affect a shared menu cursor. Normalized to 1.0 max.
		/// @param whichPlayer The player for which menu direction is taken, -1 for combined.
		/// @return The vector with the directional input from any or all players.
		Vector GetMenuDirectional(int whichPlayer = -1);

		/// Gets whether any generic button with the menu cursor is held down.
		/// @param whichButton Which generic menu cursor button to check for.
		/// @return Whether the button is held or not.
		bool MenuButtonHeld(int whichButton) { return GetMenuButtonState(whichButton, InputState::Held); }

		/// Gets whether any generic button with the menu cursor was pressed between previous update and this.
		/// @param whichButton Which generic menu cursor button to check for.
		/// @return Whether the button is pressed or not.
		bool MenuButtonPressed(int whichButton) { return GetMenuButtonState(whichButton, InputState::Pressed); }

		/// Gets whether any generic button with the menu cursor was released between previous update and this.
		/// @param whichButton Which generic menu cursor button to check for.
		/// @return Whether the button is released or not.
		bool MenuButtonReleased(int whichButton) { return GetMenuButtonState(whichButton, InputState::Released); }

		/// Gets whether there is any input at all, keyboard or buttons or D-pad.
		/// @return Whether any buttons of pads are pressed at all.
		bool AnyKeyOrJoyInput() const;

		/// Gets whether there are any key, button, or D-pad presses at all. MUST call Update before calling this for it to work properly!
		/// @return Whether any buttons of pads have been pressed at all since last frame.
		bool AnyPress() const;

		/// Gets whether there are any start key/button presses at all. MUST call Update before calling this for it to work properly!
		/// @param includeSpacebar Whether to check for space bar presses or not.
		/// @return Whether any start buttons or keys have been pressed at all since last frame.
		bool AnyStartPress(bool includeSpacebar = true);

		/// Gets whether there are any back button presses at all. MUST call Update before calling this for it to work properly!
		/// @return Whether any back buttons have been pressed at all since last frame.
		bool AnyBackPress();

		/// Gets the state of the Left Ctrl key.
		/// @return The state of the Left Ctrl key.
		bool FlagLCtrlState() const { return (SDL_GetModState() & SDL_KMOD_LCTRL) > 0; }

		/// Gets the state of the Right Ctrl key.
		/// @return The state of the Right Ctrl key.
		bool FlagRCtrlState() const { return (SDL_GetModState() & SDL_KMOD_RCTRL) > 0; }

		/// Gets the state of either Ctrl key.
		/// @return The state of either Ctrl key.
		bool FlagCtrlState() const { return (SDL_GetModState() & SDL_KMOD_CTRL) > 0; }

		/// Gets the state of the Left Alt key.
		/// @return The state of the Alt key.
		bool FlagLAltState() const { return (SDL_GetModState() & SDL_KMOD_LALT) > 0; }

		/// Gets the state of the Right Alt key.
		/// @return The state of the Right Alt key.
		bool FlagRAltState() const { return (SDL_GetModState() & (SDL_KMOD_RALT | SDL_KMOD_MODE)) > 0; }

		/// Gets the state of either Alt key.
		/// @return The state of either Alt key.
		bool FlagAltState() const { return (SDL_GetModState() & SDL_KMOD_ALT | SDL_KMOD_MODE) > 0; }

		/// Gets the state of the Left Shift key.
		/// @return The state of the Left Shift key.
		bool FlagLShiftState() const { return (SDL_GetModState() & SDL_KMOD_LSHIFT) > 0; }

		/// Gets the state of the Right Shift key.
		/// @return The state of the Right Shift key.
		bool FlagRShiftState() const { return (SDL_GetModState() & SDL_KMOD_RSHIFT) > 0; }

		/// Gets the state of either Shift key.
		/// @return The state of either Shift key.
		bool FlagShiftState() const { return (SDL_GetModState() & SDL_KMOD_SHIFT) > 0; }
#pragma endregion

#pragma region Keyboard Handling
		/// Temporarily disables most of the keyboard keys. This is used when typing into a dialog box is required.
		/// @param disable Whether to disable most keys or not.
		void DisableKeys(bool disable = true) { m_DisableKeyboard = disable; }

		/// Gets whether a key is being held right now, by scancode.
		/// @param scancodeToTest A scancode to test. See SDL_Scancode enumeration.
		/// @return Whether the key is held or not.
		bool KeyHeld(SDL_Scancode scancodeToTest, int whichPlayer = -1) const { return GetKeyboardButtonState(scancodeToTest, InputState::Held, whichPlayer); }
		bool KeyHeldScancode(SDL_Scancode scancodeToTest) const { return KeyHeld(scancodeToTest, -1); } //!< Lua disambiguation helper.

		/// Gets whether a key is being held right now, by keycode.
		/// @param keycodeToTest A keycode to test. See SDL_KeyCode enumeration.
		/// @return Whether the key is held or not.
		bool KeyHeld(SDL_Keycode keycodeToTest, int whichPlayer = -1) const { return KeyHeld(SDL_GetScancodeFromKey(keycodeToTest, NULL), whichPlayer); }
		bool KeyHeldKeycode(SDL_Keycode keycodeToTest) const { return KeyHeld(keycodeToTest); } //!< Lua disambiguation helper.

		/// Gets whether a key was pressed between the last update and the one previous to it, by scancode.
		/// @param scancodeToTest A scancode to test. See SDL_Scancode enumeration.
		/// @return Whether the key is pressed or not.
		bool KeyPressed(SDL_Scancode scancodeToTest, int whichPlayer = -1) const { return GetKeyboardButtonState(scancodeToTest, InputState::Pressed, whichPlayer); }
		bool KeyPressedScancode(SDL_Scancode scancodeToTest) const { return KeyPressed(scancodeToTest); } //!< Lua disambiguation helper.

		/// Gets whether a key was pressed between the last update and the one previous to it, by keycode.
		/// @param keycodeToTest A keycode to test. See SDL_KeyCode enumeration.
		/// @return Whether the key is pressed or not.
		bool KeyPressed(SDL_Keycode keycodeToTest, int whichPlayer = -1) const { return KeyPressed(SDL_GetScancodeFromKey(keycodeToTest, NULL), whichPlayer); }
		bool KeyPressedKeycode(SDL_Keycode keycodeToTest) const { return KeyPressed(keycodeToTest); } //!< Lua disambiguation helper.

		/// Gets whether a key was released between the last update and the one previous to it, by scancode.
		/// @param scancodeToTest A scancode to test. See SDL_Scancode enumeration.
		/// @return Whether the key is released or not.
		bool KeyReleased(SDL_Scancode scancodeToTest, int whichPlayer = -1) const { return GetKeyboardButtonState(scancodeToTest, InputState::Released, whichPlayer); }
		bool KeyReleasedScancode(SDL_Scancode scancodeToTest) const { return KeyReleased(scancodeToTest); } //!< Lua disambiguation helper.

		/// Gets whether a key was released between the last update and the one previous to it, by keycode.
		/// @param keycodeToTest A keycode to test. See SDL_KeyCode enumeration.
		/// @return Whether the key is released or not.
		bool KeyReleased(SDL_Keycode keycodeToTest, int whichPlayer = -1) const { return KeyReleased(SDL_GetScancodeFromKey(keycodeToTest, NULL), whichPlayer); }
		bool KeyReleasedKeycode(SDL_Keycode keycodeToTest) const { return KeyReleased(keycodeToTest); } //!< Lua disambiguation helper.

		/// Return true if there are any keyboard button presses at all.
		/// @return Whether any keyboard buttons have been pressed at all since last frame.
		bool AnyKeyPress(SDL_KeyboardID keyboardID = 0) const;

		/// Fills the given string with the text input since the last frame (if any).
		/// @param text The std::string to fill.
		/// @return Whether there is text input.
		bool GetTextInput(std::string& text) const {
			text = m_TextInput;
			return !m_TextInput.empty();
		}

		/// Returns whether text input events are available.
		bool HasTextInput() const { return !m_TextInput.empty(); }

		/// Returns the current text input.
		/// @return The current text input.
		const std::string& GetTextInput() const { return m_TextInput; }
#pragma endregion

#pragma region Mouse Handling
		/// Reports which player is using the mouse for control at this time if any.
		/// @return Which player is using the mouse. If no one is then -1 is returned.
		int MouseUsedByPlayer() const;

		/// Will temporarily disable positioning of the mouse.
		/// This is so that when focus is switched back to the game window, it avoids having the window fly away because the user clicked the title bar of the window.
		/// @param disable Whether to disable mouse positioning or not.
		void DisableMouseMoving(bool disable = true);

		/// @brief Check if multi mouse and keyboard should be enabled.
		/// Checks through a list of human players to see if multimouse is required.
		/// @param player A list of human players.
		bool CheckMultiMouseKeyboardEnabled(std::optional<std::reference_wrapper<const std::vector<int>>> players = std::nullopt);

		bool AllPlayerInputDevicesKnown(const std::vector<int>& humanPlayers) const;

		/// Get the absolute mouse position in window coordinates.
		/// @return The absolute mouse position.
		Vector GetAbsoluteMousePosition( int whichPlayer = -1) const;

		/// Set the absolute mouse position (e.g. for player input mouse movement). Does not move the system cursor.
		/// @param pos The new mouse position.
		void SetAbsoluteMousePosition(const Vector& pos, int whichPlayer = -1);

		/// Gets the relative movement of the mouse since last update. Only returns true if the selected player is actually using the mouse.
		/// @param whichPlayer Which player to get movement for. If the player doesn't use the mouse this always returns a zero vector.
		/// @return The relative mouse movements, in both axes.
		Vector GetMouseMovement(int whichPlayer = -1) const;

		/// Set the mouse's analog emulation output to be of a specific normalized magnitude.
		/// @param magCap The normalized magnitude, between 0 and 1.0.
		/// @param whichPlayer Which player to set magnitude for. Only relevant when in online multiplayer mode.
		void SetMouseValueMagnitude(float magCap, int whichPlayer = Players::NoPlayer);

		/// Sets the mouse's analog emulation output to be in a specific direction.
		/// @param angle The direction, in radians.
		/// @param whichPlayer Which player to set magnitude for. Only relevant when in online multiplayer mode.
		void SetMouseValueAngle(float angle, int whichPlayer = Players::NoPlayer);

		/// Sets the absolute screen position of the mouse cursor.
		/// @param newPos Where to place the mouse.
		/// @param whichPlayer Which player is trying to control the mouse. Only the player with actual control over the mouse will be affected. -1 means do it regardless of player.
		void SetMousePos(const Vector& newPos, int whichPlayer = -1);

		/// Gets mouse sensitivity while in Activity.
		/// @return The current mouse sensitivity.
		float GetMouseSensitivity() const { return m_MouseSensitivity; }

		/// Sets mouse sensitivity while in Activity.
		/// @param sensitivity New sensitivity value.
		void SetMouseSensitivity(float sensitivity) { m_MouseSensitivity = std::clamp(sensitivity, 0.1F, 2.0F); }

		/// Gets whether a mouse button is being held down right now.
		/// @param whichButton Which button to check for.
		/// @param whichPlayer Which player to check for.
		/// @return Whether the mouse button is held or not.
		bool MouseButtonHeld(int whichButton, int whichPlayer = Players::PlayerOne, SDL_MouseID mouse = 0) const { return GetMouseButtonState(whichPlayer, whichButton, InputState::Held, mouse); }

		/// Gets whether a mouse button was pressed between the last update and the one previous to it.
		/// @param whichButton Which button to check for.
		/// @param whichPlayer Which player to check for.
		/// @return Whether the mouse button is pressed or not.
		bool MouseButtonPressed(int whichButton, int whichPlayer = Players::PlayerOne, SDL_MouseID mouse = 0) const { return GetMouseButtonState(whichPlayer, whichButton, InputState::Pressed, mouse); }

		/// Gets whether a mouse button was released between the last update and the one previous to it.
		/// @param whichButton Which button to check for.
		/// @param whichPlayer Which player to check for.
		/// @return Whether the mouse button is released or not.
		bool MouseButtonReleased(int whichButton, int whichPlayer = Players::PlayerOne, SDL_MouseID mouse = 0) const { return GetMouseButtonState(whichPlayer, whichButton, InputState::Released, mouse); }

		const std::array<bool, MouseButtons::MAX_MOUSE_BUTTONS>& GetMouseState(int whichPlayer = -1) const;
		const std::array<bool, MouseButtons::MAX_MOUSE_BUTTONS>& GetMouseChange(int whichPlayer = -1) const;

		void ClearMouseButtons();

		/// Gets whether the mouse wheel has been moved past the threshold limit in either direction this frame.
		/// @return The direction the mouse wheel has been moved which is past that threshold. 0 means not past, negative means moved down, positive means moved up.
		int MouseWheelMoved() const { return m_MouseStates.at(0).wheelChange; }

		/// Gets the relative mouse wheel position for the specified player.
		/// @param player The player to get mouse wheel position for.
		/// @return The relative mouse wheel position for the specified player.
		int MouseWheelMovedByPlayer(int player) const;
		/// Return true if there are any mouse button presses at all.
		/// @return Whether any mouse buttons have been pressed at all since last frame.
		bool AnyMouseButtonPress(SDL_MouseID mouseID = 0) const;

		/// Sets the mouse to be trapped in the middle of the screen so it doesn't go out and click on other windows etc.
		/// This is usually used when the cursor is invisible and only relative mouse movements are used.
		/// @param trap Whether to trap the mouse or not.
		/// @param whichPlayer
		/// Which player is trying to control the mouse.
		/// Only the player with actual control over the mouse will affect its trapping here. -1 means change mouse trapping regardless of player.
		void TrapMousePos(bool trap = true, int whichPlayer = -1);

		/// Forces the mouse within a box on the screen.
		/// @param x X value of the top left corner of the screen box to keep the mouse within, relative to the top left corner of the player's screen.
		/// @param y Y value of the top left corner of the screen box to keep the mouse within, relative to the top left corner of the player's screen.
		/// @param width The width of the box.
		/// @param height The height of the box.
		/// @param whichPlayer Which player is trying to control the mouse. Only the player with actual control over the mouse will be affected. -1 means do it regardless of player.
		void ForceMouseWithinBox(int x, int y, int width, int height, int whichPlayer = Players::NoPlayer);
#pragma endregion

#pragma region Joystick Handling
		/// Gets the number of active joysticks.
		/// @return The number of active joysticks.
		int GetJoystickCount() const { return (m_NumJoysticks > Players::MaxPlayerCount) ? Players::MaxPlayerCount : m_NumJoysticks; }

		/// Gets the index number of a joystick from InputDevice. Basically just subtract 2 from the passed in value because the Allegro joystick indices are 0-3 and ours are 2-5.
		/// @param device The InputDevice to get index from.
		/// @return The corrected index. A non-joystick device will result in an out of range value returned which will not affect any active joysticks.
		int GetJoystickIndex(InputDevice device) const { return (device >= InputDevice::DEVICE_GAMEPAD_1 && device < InputDevice::DEVICE_COUNT) ? device - InputDevice::DEVICE_GAMEPAD_1 : InputDevice::DEVICE_COUNT; }

		SDL_JoystickID GetGamepadID(InputDevice gamepad) const;

		/// Gets the number of axes of the specified joystick.
		/// @param whichJoy Joystick to check.
		/// @return The number of axes of the joystick.
		int GetJoystickAxisCount(int whichJoy) const;

		/// Gets whether the specified joystick is active. The joystick number does not correspond to the player number.
		/// @param joystickNumber Joystick to check for.
		/// @return Whether the specified joystick is active.
		bool JoystickActive(int joystickNumber) const { return joystickNumber >= Players::PlayerOne && joystickNumber < Players::MaxPlayerCount && s_PrevJoystickStates[joystickNumber].m_JoystickID != -1; }

		/// Gets whether a joystick button is being held down right now.
		/// @param whichJoy Which joystick to check for.
		/// @param whichButton Which joystick button to check for.
		/// @return Whether the joystick button is held or not.
		bool JoyButtonHeld(int whichJoy, int whichButton) const { return GetJoystickButtonState(whichJoy, whichButton, InputState::Held); }

		/// Shows the first joystick button which is currently down.
		/// @param whichJoy Which joystick to check for.
		/// @return The first button in the sequence of button enumerations that is held at the time of calling this. JOY_NONE means none.
		int WhichJoyButtonHeld(int whichJoy) const;

		/// Gets whether a joystick button was pressed between the last update and the one previous to it.
		/// @param whichJoy Which joystick to check for.
		/// @param whichButton Which joystick button to check for.
		/// @return Whether the joystick button is pressed or not.
		bool JoyButtonPressed(int whichJoy, int whichButton) const { return GetJoystickButtonState(whichJoy, whichButton, InputState::Pressed); }

		/// Shows the first joystick button which was pressed down since last frame.
		/// @param whichJoy Which joystick to check for.
		/// @return The first button in the sequence of button enumerations that is pressed since the previous frame. JOY_NONE means none.
		int WhichJoyButtonPressed(int whichJoy) const;

		/// Gets whether a joystick button was released between the last update and the one previous to it.
		/// @param whichJoy Which joystick to check for.
		/// @param whichButton Which joystick button to check for.
		/// @return Whether the joystick button is released or not.
		bool JoyButtonReleased(int whichJoy, int whichButton) const { return GetJoystickButtonState(whichJoy, whichButton, InputState::Released); }

		/// Gets whether a joystick axis is being held down in a specific direction right now. Two adjacent directions can be held down to produce diagonals.
		/// @param whichJoy Which joystick to check for.
		/// @param whichAxis Which joystick stick axis to check for.
		/// @param whichDir Which direction to check for.
		/// @return Whether the stick axis is held in the specified direction or not.
		bool JoyDirectionHeld(int whichJoy, int whichAxis, int whichDir) const { return GetJoystickDirectionState(whichJoy, whichAxis, whichDir, InputState::Held); }

		/// Gets whether a joystick axis direction was pressed between the last update and the one previous to it.
		/// @param whichJoy Which joystick to check for.
		/// @param whichAxis Which joystick stick axis to check for.
		/// @param whichDir Which direction to check for.
		/// @return Whether the stick axis is pressed or not.
		bool JoyDirectionPressed(int whichJoy, int whichAxis, int whichDir) const { return GetJoystickDirectionState(whichJoy, whichAxis, whichDir, InputState::Pressed); }

		/// Gets whether a joystick axis direction was released between the last update and the one previous to it.
		/// @param whichJoy Which joystick to check for.
		/// @param whichAxis Which joystick stick axis to check for.
		/// @param whichDir Which direction to check for.
		/// @return Whether the stick axis is released or not.
		bool JoyDirectionReleased(int whichJoy, int whichAxis, int whichDir) const { return GetJoystickDirectionState(whichJoy, whichAxis, whichDir, InputState::Released); }

		/// Gets the normalized value of a certain joystick's stick's axis.
		/// @param whichJoy Which joystick to check for.
		/// @param whichAxis Which joystick stick axis to check for.
		/// @return The analog axis value ranging between -1.0 to 1.0, or 0.0 to 1.0 if it's a throttle type control.
		float AnalogAxisValue(int whichJoy = 0, int whichAxis = 0) const;

		/// Gets whether there is any joystick input at all, buttons or D-pad.
		/// @param checkForPresses Whether to check specifically for presses since last frame.
		/// @return Whether any buttons of pads are pressed at all or since the last frame.
		bool AnyJoyInput(bool checkForPresses = false) const;

		/// Return true if there are any joystick presses at all, buttons or D-pad.
		/// @return Whether any buttons or pads have been pressed at all since last frame.
		bool AnyJoyPress() const { return AnyJoyInput(true); }

		/// Gets whether there are any joystick button presses at all, but not D-pad input, for a specific joystick.
		/// @param whichJoy Which joystick to check for.
		/// @return Whether any joystick buttons have been pressed at all since last frame, of a specific joystick.
		bool AnyJoyButtonPress(int whichJoy) const;
#pragma endregion

	private:
		/// Enumeration for the different states an input element or button can be in.
		enum InputState {
			Held,
			Pressed,
			Released,
			InputStateCount
		};

		struct Keyboard {
			SDL_KeyboardID id{0};
			std::array<bool, SDL_SCANCODE_COUNT> keyStates{};
			std::array<bool, SDL_SCANCODE_COUNT> changedKeyStates{};
		};
		std::unordered_map<SDL_KeyboardID, Keyboard> m_KeyboardStates; //!< Keyboard state when multi keyboard support is enabled.

		struct Mouse {
			SDL_MouseID id{0};
			std::array<bool, MouseButtons::MAX_MOUSE_BUTTONS> state{};
			std::array<bool, MouseButtons::MAX_MOUSE_BUTTONS> change{};
			Vector position{};
			Vector relativeMotion{};
			Vector analogAim{};
			float wheelChange{0.0f};
			bool relativeMode{};
		};
		std::unordered_map<SDL_MouseID, Mouse> m_MouseStates; //!< Mouse states. Only MouseStates[0] is guaranteed to exist and contains the combined mouse input.

		static std::vector<Gamepad> s_PrevJoystickStates; //!< Joystick states as they were the previous update.
		static std::vector<Gamepad> s_ChangedJoystickStates; //!< Joystick states that have changed.

		std::vector<SDL_Event> m_EventQueue; //!< List of incoming input events.

		bool m_SkipHandlingSpecialInput; //!< Whether to skip handling any special input (F1-F12, etc.) to avoid shenanigans during manual input mapping.

		int m_NumJoysticks; //!< The number of currently connected gamepads.

		std::string m_TextInput; //!< Buffer for passing text input from SDL event handling to the GUI.

		bool m_OverrideInput; //!< If true then this instance operates in multiplayer mode and the input is overridden by network input.

		std::array<InputScheme, Players::MaxPlayerCount> m_ControlScheme; //!< Which control scheme is being used by each player.
		const Icon* m_DeviceIcons[InputDevice::DEVICE_COUNT]; //!< The Icons representing all different devices.

		float m_MouseSensitivity; //!< Mouse sensitivity multiplier while in Activity. HAS NO EFFECT IN MENUS.

		bool m_TrapMousePos; //!< Whether the mouse is trapped in the middle of the screen each update or not.
		float m_MouseTrapRadius; //!< The radius (in pixels) of the circle trapping the mouse for analog mouse data.
		SDL_Rect m_PlayerScreenMouseBounds; //!< Rect with the position and dimensions of the player screen that the mouse is bound to, when bounding is enabled.

		InputDevice m_LastDeviceWhichControlledGUICursor; //!< Indicates which device controlled the cursor last time.

		bool m_ForceDisableMultiMouseKeyboard{false}; //!< Whether to force enable muti mouse/keyboard support.
		bool m_EnableMultiMouseKeyboard{true}; //!< Allow use of multiple mice and keyboards. (Enables relative mouse mode.)
		bool m_PlayerMouseKeyboardKnown{false}; //!< Whether all player devices are known when multiple mouse and/or keyboards are requested.
		bool m_DisableKeyboard; //!< Temporarily disable all keyboard input reading.
		bool m_DisableMouseMoving; //!< Temporary disable for positioning the mouse, for when the game window is not in focus.

		/// This is set when focus is switched back to the game window and will cause the m_DisableMouseMoving to switch to false when the mouse button is RELEASED.
		/// This is to avoid having the window fly away because the user clicked the title bar.
		bool m_PrepareToEnableMouseMoving;

		static constexpr double c_GamepadAxisLimit = 32767.0; //!< Maximum axis value as defined by SDL (int16 max).
		static constexpr int c_AxisDigitalPressedThreshold = 16384; //!< Digital Axis threshold value as defined by allegro.
		static constexpr int c_AxisDigitalReleasedThreshold = 8192; //!< Digital Axis release threshold, to debounce values.

#pragma region Mouse Handling
		/// Forces the mouse within a specific player's screen area.
		/// Player 1 will always be in the upper-left corner, Player 3 will always be in the lower-left corner, Player 4 will always be in the lower-right quadrant.
		/// Player 2 will either be in the lower-left corner or the upper-right corner depending on vertical/horizontal splitting.
		/// @param whichPlayer Which player's screen to constrain the mouse to. Only the player with actual control over the mouse will be affected.
		void ForceMouseWithinPlayerScreen(bool force, int whichPlayer);
#pragma endregion

#pragma region Input State Handling
		/// Gets whether an input element is in the specified state.
		/// @param whichPlayer Which player to check for. See Players enumeration.
		/// @param whichElement Which element to check for. See InputElements enumeration.
		/// @param whichState Which state to check for. See InputState enumeration.
		/// @return Whether the element is in the specified state or not.
		bool GetInputElementState(int whichPlayer, int whichElement, InputState whichState);

		/// Gets whether any generic button with the menu cursor is in the specified state.
		/// @param whichButton Which menu button to check for. See MenuButtons enumeration.
		/// @param whichState Which state to check for. See InputState enumeration.
		/// @return Whether the menu button is in the specified state or not.
		bool GetMenuButtonState(int whichButton, InputState whichState);

		/// Gets whether a keyboard key is in the specified state.
		/// @param scancodeToTest A scancode to test. See SDL_Scancode enumeration.
		/// @param whichState Which state to check for. See InputState enumeration.
		/// @return Whether the keyboard key is in the specified state or not.
		bool GetKeyboardButtonState(SDL_Scancode scancodeToTest, InputState whichState, int whichPlayer = NoPlayer, SDL_KeyboardID keyboard = 0) const;

		/// Gets whether a mouse button is in the specified state.
		/// @param whichPlayer Which player to check for. See Players enumeration.
		/// @param whichButton Which mouse button to check for. See MouseButtons enumeration.
		/// @param whichState Which state to check for. See InputState enumeration.
		/// @return Whether the mouse button is in the specified state or not.
		bool GetMouseButtonState(int whichPlayer, int whichButton, InputState whichState, SDL_MouseID mouse = 0) const;

		/// Gets whether a joystick button is in the specified state.
		/// @param whichJoy Which joystick to check for.
		/// @param whichButton Which joystick button to check for. See JoyButtons enumeration.
		/// @param whichState Which state to check for. See InputState enumeration.
		/// @return Whether the joystick button is in the specified state or not.
		bool GetJoystickButtonState(int whichJoy, int whichButton, InputState whichState) const;

		/// Gets whether a joystick axis direction is in the specified state or not.
		/// @param whichJoy Which joystick to check for.
		/// @param whichAxis Which joystick stick axis to check for.
		/// @param whichDir Which direction to check for. See JoyDirections enumeration.
		/// @param whichState Which state to check for. See InputState enumeration.
		/// @return Whether the joystick stick axis is in the specified state or not.
		bool GetJoystickDirectionState(int whichJoy, int whichAxis, int whichDir, InputState whichState) const;
#pragma endregion

#pragma region Update Breakdown
		/// Capture and handle special key shortcuts and combinations. This is called from Update().
		void HandleSpecialInput();

		/// Handles the mouse input in network multiplayer. This is called from Update().
		void UpdateNetworkMouseMovement();

		/// Clear all NetworkServerChanged* arrays.
		void ClearNetworkChangedState();

		/// Handles the mouse input. This is called from Update().
		void UpdateMouseInput();

		/// Handles a joystick axis input. This is called from Update().
		void UpdateJoystickAxis(std::vector<Gamepad>::iterator device, int axis, int newValue);

		/// Updates simulated digital joystick axis. This is called from Update().
		void UpdateJoystickDigitalAxis();

		/// Connect a joystick or gamepad device and add it to the joystick list if a slot is available (up to max player count).
		/// @param joystickID The SDL_JoystickID of the added Gamepad, usually from the corresponding device event.
		void HandleGamepadHotPlug(SDL_JoystickID joystickID);
#pragma endregion

		/// Clears all the member variables of this UInputMan, effectively resetting the members of this abstraction level only.
		void Clear();

		// Disallow the use of some implicit methods.
		UInputMan(const UInputMan& reference) = delete;
		UInputMan& operator=(const UInputMan& rhs) = delete;
	};
} // namespace RTE
