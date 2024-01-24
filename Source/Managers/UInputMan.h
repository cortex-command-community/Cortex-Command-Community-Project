#pragma once

#include "Singleton.h"
#include "Vector.h"
#include "InputScheme.h"
#include "Gamepad.h"
#include "SDL_keyboard.h"
#include "SDL_events.h"

#include <algorithm>
#include <array>
#include <string>
#include <vector>

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
		void QueueInputEvent(const SDL_Event& inputEvent);

		/// Updates the state of this UInputMan. Supposed to be done every frame.
		/// @return An error return value signaling success or any particular failure. Anything below 0 is an error signal.
		int Update();
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
		InputScheme* GetControlScheme(int whichPlayer) { return IsInMultiplayerMode() ? &m_ControlScheme[Players::PlayerOne] : &m_ControlScheme.at(whichPlayer); }

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

		/// Gets the generic direction input from any and all players which can affect a shared menu cursor. Normalized to 1.0 max.
		/// @return The vector with the directional input from any or all players.
		Vector GetMenuDirectional();

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

		/// Gets the state of the Ctrl key.
		/// @return The state of the Ctrl key.
		bool FlagCtrlState() const { return ((SDL_GetModState() & KMOD_CTRL) > 0) ? true : false; }

		/// Gets the state of the Alt key.
		/// @return The state of the Alt key.
		bool FlagAltState() const { return ((SDL_GetModState() & KMOD_ALT) > 0) ? true : false; }

		/// Gets the state of the Shift key.
		/// @return The state of the Shift key.
		bool FlagShiftState() const { return ((SDL_GetModState() & KMOD_SHIFT) > 0) ? true : false; }
#pragma endregion

#pragma region Keyboard Handling
		/// Temporarily disables most of the keyboard keys. This is used when typing into a dialog box is required.
		/// @param disable Whether to disable most keys or not.
		void DisableKeys(bool disable = true) { m_DisableKeyboard = disable; }

		/// Gets whether a key is being held right now, by scancode.
		/// @param scancodeToTest A scancode to test. See SDL_Scancode enumeration.
		/// @return Whether the key is held or not.
		bool KeyHeld(SDL_Scancode scancodeToTest) const { return GetKeyboardButtonState(scancodeToTest, InputState::Held); }

		/// Gets whether a key is being held right now, by keycode.
		/// @param keycodeToTest A keycode to test. See SDL_KeyCode enumeration.
		/// @return Whether the key is held or not.
		bool KeyHeld(SDL_Keycode keycodeToTest) const { return KeyHeld(SDL_GetScancodeFromKey(keycodeToTest)); }

		/// Gets whether a key was pressed between the last update and the one previous to it, by scancode.
		/// @param scancodeToTest A scancode to test. See SDL_Scancode enumeration.
		/// @return Whether the key is pressed or not.
		bool KeyPressed(SDL_Scancode scancodeToTest) const { return GetKeyboardButtonState(scancodeToTest, InputState::Pressed); }

		/// Gets whether a key was pressed between the last update and the one previous to it, by keycode.
		/// @param keycodeToTest A keycode to test. See SDL_KeyCode enumeration.
		/// @return Whether the key is pressed or not.
		bool KeyPressed(SDL_Keycode keycodeToTest) const { return KeyPressed(SDL_GetScancodeFromKey(keycodeToTest)); }

		/// Gets whether a key was released between the last update and the one previous to it, by scancode.
		/// @param scancodeToTest A scancode to test. See SDL_Scancode enumeration.
		/// @return Whether the key is released or not.
		bool KeyReleased(SDL_Scancode scancodeToTest) const { return GetKeyboardButtonState(scancodeToTest, InputState::Released); }

		/// Gets whether a key was released between the last update and the one previous to it, by keycode.
		/// @param keycodeToTest A keycode to test. See SDL_KeyCode enumeration.
		/// @return Whether the key is released or not.
		bool KeyReleased(SDL_Keycode keycodeToTest) const { return KeyReleased(SDL_GetScancodeFromKey(keycodeToTest)); }

		/// Return true if there are any keyboard button presses at all.
		/// @return Whether any keyboard buttons have been pressed at all since last frame.
		bool AnyKeyPress() const;

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

		/// Get the absolute mouse position in window coordinates.
		/// @return The absolute mouse position.
		Vector GetAbsoluteMousePosition() const { return m_AbsoluteMousePos; }

		/// Set the absolute mouse position (e.g. for player input mouse movement). Does not move the system cursor.
		/// @param pos The new mouse position.
		void SetAbsoluteMousePosition(const Vector& pos) { m_AbsoluteMousePos = pos; }

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
		void SetMousePos(const Vector& newPos, int whichPlayer = -1) const;

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
		bool MouseButtonHeld(int whichButton, int whichPlayer = Players::PlayerOne) const { return GetMouseButtonState(whichPlayer, whichButton, InputState::Held); }

		/// Gets whether a mouse button was pressed between the last update and the one previous to it.
		/// @param whichButton Which button to check for.
		/// @param whichPlayer Which player to check for.
		/// @return Whether the mouse button is pressed or not.
		bool MouseButtonPressed(int whichButton, int whichPlayer = Players::PlayerOne) const { return GetMouseButtonState(whichPlayer, whichButton, InputState::Pressed); }

		/// Gets whether a mouse button was released between the last update and the one previous to it.
		/// @param whichButton Which button to check for.
		/// @param whichPlayer Which player to check for.
		/// @return Whether the mouse button is released or not.
		bool MouseButtonReleased(int whichButton, int whichPlayer = Players::PlayerOne) const { return GetMouseButtonState(whichPlayer, whichButton, InputState::Released); }

		/// Gets whether the mouse wheel has been moved past the threshold limit in either direction this frame.
		/// @return The direction the mouse wheel has been moved which is past that threshold. 0 means not past, negative means moved down, positive means moved up.
		int MouseWheelMoved() const { return m_MouseWheelChange; }

		/// Gets the relative mouse wheel position for the specified player.
		/// @param player The player to get mouse wheel position for.
		/// @return The relative mouse wheel position for the specified player.
		int MouseWheelMovedByPlayer(int player) const {
			return (IsInMultiplayerMode() && player >= Players::PlayerOne && player < Players::MaxPlayerCount) ? m_NetworkMouseWheelState[player] : m_MouseWheelChange;
		}

		/// Return true if there are any mouse button presses at all.
		/// @return Whether any mouse buttons have been pressed at all since last frame.
		bool AnyMouseButtonPress() const;

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
		void ForceMouseWithinBox(int x, int y, int width, int height, int whichPlayer = Players::NoPlayer) const;
#pragma endregion

#pragma region Joystick Handling
		/// Gets the number of active joysticks.
		/// @return The number of active joysticks.
		int GetJoystickCount() const { return (m_NumJoysticks > Players::MaxPlayerCount) ? Players::MaxPlayerCount : m_NumJoysticks; }

		/// Gets the index number of a joystick from InputDevice. Basically just subtract 2 from the passed in value because the Allegro joystick indices are 0-3 and ours are 2-5.
		/// @param device The InputDevice to get index from.
		/// @return The corrected index. A non-joystick device will result in an out of range value returned which will not affect any active joysticks.
		int GetJoystickIndex(InputDevice device) const { return (device >= InputDevice::DEVICE_GAMEPAD_1 && device < InputDevice::DEVICE_COUNT) ? device - InputDevice::DEVICE_GAMEPAD_1 : InputDevice::DEVICE_COUNT; }

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

#pragma region Network Handling
		/// Returns true if manager is in multiplayer mode.
		/// @return True if in multiplayer mode.
		bool IsInMultiplayerMode() const { return m_OverrideInput; }

		/// Sets the multiplayer mode flag.
		/// @param value Whether this manager should operate in multiplayer mode.
		void SetMultiplayerMode(bool value) { m_OverrideInput = value; }

		/// Gets the position of the mouse for a player during network multiplayer.
		/// @param player The player to get for.
		/// @return The position of the mouse for the specified player
		Vector GetNetworkAccumulatedRawMouseMovement(int player);

		/// Sets the position of the mouse for a player during network multiplayer.
		/// @param player The player to set for.
		/// @param input The new position of the mouse.
		void SetNetworkMouseMovement(int player, const Vector& input) { m_NetworkAccumulatedRawMouseMovement[player] += input; }

		/// Sets whether an input element is held by a player during network multiplayer.
		/// @param player Which player to set for.
		/// @param element Which input element to set for.
		/// @param state The new state of the input element. True or false.
		void SetNetworkInputElementState(int player, int element, bool state);

		/// Sets whether a mouse button is held by a player during network multiplayer.
		/// @param player Which player to set for.
		/// @param whichButton Which mouse button to set for.
		/// @param state The new state of the mouse button. True or false.
		void SetNetworkMouseButtonHeldState(int player, int whichButton, bool state) { SetNetworkMouseButtonState(player, whichButton, InputState::Held, state); }

		/// Sets whether a mouse button is pressed by a player during network multiplayer.
		/// @param player Which player to set for.
		/// @param whichButton Which mouse button to set for.
		/// @param state The new state of the mouse button. True or false.
		void SetNetworkMouseButtonPressedState(int player, int whichButton, bool state) { SetNetworkMouseButtonState(player, whichButton, InputState::Pressed, state); }

		/// Sets whether a mouse button is released by a player during network multiplayer.
		/// @param player Which player to set for.
		/// @param whichButton Which mouse button to set for.
		/// @param state The new state of the mouse button. True or false.
		void SetNetworkMouseButtonReleasedState(int player, int whichButton, bool state) { SetNetworkMouseButtonState(player, whichButton, InputState::Released, state); }

		/// Sets the state of the mouse wheel for a player during network multiplayer.
		/// @param player The player to set for.
		/// @param state The new state of the mouse wheel.
		void SetNetworkMouseWheelState(int player, int state) {
			if (player >= Players::PlayerOne && player < Players::MaxPlayerCount) {
				m_NetworkMouseWheelState[player] += state;
			}
		}

		/// Gets whether the specified input element is pressed during network multiplayer.
		/// @param element The input element to check for.
		/// @return Whether the specified input element is pressed or not.
		bool NetworkAccumulatedElementPressed(int element) const { return NetworkAccumulatedElementState(element, InputState::Pressed); }

		/// Gets whether the specified input element is released during network multiplayer.
		/// @param element The input element to check for.
		/// @return Whether the specified input element is released or not.
		bool NetworkAccumulatedElementReleased(int element) const { return NetworkAccumulatedElementState(element, InputState::Released); }

		/// Clears all the accumulated input element states.
		void ClearNetworkAccumulatedStates();
#pragma endregion

	private:
		/// Enumeration for the different states an input element or button can be in.
		enum InputState {
			Held,
			Pressed,
			Released,
			InputStateCount
		};

		static std::array<uint8_t, SDL_NUM_SCANCODES> s_PrevKeyStates; //!< Key states as they were the previous update.
		static std::array<uint8_t, SDL_NUM_SCANCODES> s_ChangedKeyStates; //!< Key states that have changed.

		static std::array<bool, MouseButtons::MAX_MOUSE_BUTTONS> s_CurrentMouseButtonStates; //!< Current mouse button states.
		static std::array<bool, MouseButtons::MAX_MOUSE_BUTTONS> s_PrevMouseButtonStates; //!< Mouse button states as they were the previous update.
		static std::array<bool, MouseButtons::MAX_MOUSE_BUTTONS> s_ChangedMouseButtonStates; //!< Mouse button states that have changed since previous update.

		static std::vector<Gamepad> s_PrevJoystickStates; //!< Joystick states as they were the previous update.
		static std::vector<Gamepad> s_ChangedJoystickStates; //!< Joystick states that have changed.

		std::vector<SDL_Event> m_EventQueue; //!< List of incoming input events.

		bool m_SkipHandlingSpecialInput; //!< Whether to skip handling any special input (F1-F12, etc.) to avoid shenanigans during manual input mapping.

		int m_NumJoysticks; //!< The number of currently connected gamepads.

		std::string m_TextInput; //!< Buffer for passing text input from SDL event handling to the GUI.

		bool m_OverrideInput; //!< If true then this instance operates in multiplayer mode and the input is overridden by network input.

		std::array<InputScheme, Players::MaxPlayerCount> m_ControlScheme; //!< Which control scheme is being used by each player.
		const Icon* m_DeviceIcons[InputDevice::DEVICE_COUNT]; //!< The Icons representing all different devices.

		Vector m_AbsoluteMousePos; //!< The absolute mouse position in screen coordinates.
		Vector m_RawMouseMovement; //!< The raw absolute movement of the mouse between the last two Updates.
		Vector m_AnalogMouseData; //!< The emulated analog stick position of the mouse.
		float m_MouseSensitivity; //!< Mouse sensitivity multiplier while in Activity. HAS NO EFFECT IN MENUS.
		int m_MouseWheelChange; //!< The relative mouse wheel position since last reset of it.

		bool m_TrapMousePos; //!< Whether the mouse is trapped in the middle of the screen each update or not.
		float m_MouseTrapRadius; //!< The radius (in pixels) of the circle trapping the mouse for analog mouse data.
		SDL_Rect m_PlayerScreenMouseBounds; //!< Rect with the position and dimensions of the player screen that the mouse is bound to, when bounding is enabled.

		InputDevice m_LastDeviceWhichControlledGUICursor; //!< Indicates which device controlled the cursor last time.

		bool m_DisableKeyboard; //!< Temporarily disable all keyboard input reading.
		bool m_DisableMouseMoving; //!< Temporary disable for positioning the mouse, for when the game window is not in focus.

		/// This is set when focus is switched back to the game window and will cause the m_DisableMouseMoving to switch to false when the mouse button is RELEASED.
		/// This is to avoid having the window fly away because the user clicked the title bar.
		bool m_PrepareToEnableMouseMoving;

		bool m_NetworkAccumulatedElementState[InputElements::INPUT_COUNT][InputState::InputStateCount]; //!< The state of a client input element during network multiplayer.
		bool m_NetworkServerChangedInputElementState[Players::MaxPlayerCount][InputElements::INPUT_COUNT]; //!< The server side state of a player's input element during network multiplayer.

		std::array<std::array<bool, InputElements::INPUT_COUNT>, Players::MaxPlayerCount> m_NetworkServerPreviousInputElementState;
		bool m_NetworkServerChangedMouseButtonState[Players::MaxPlayerCount][MouseButtons::MAX_MOUSE_BUTTONS]; //!< The state of a player's mouse button during network multiplayer.
		std::array<std::array<bool, InputElements::INPUT_COUNT>, Players::MaxPlayerCount> m_NetworkServerPreviousMouseButtonState;

		Vector m_NetworkAccumulatedRawMouseMovement[Players::MaxPlayerCount]; //!< The position of the mouse for each player during network multiplayer.
		Vector m_NetworkAnalogMoveData[Players::MaxPlayerCount]; //!< Mouse analog movement data for each player during network multiplayer.
		int m_NetworkMouseWheelState[Players::MaxPlayerCount]; //!< The position of a player's mouse wheel during network multiplayer.

		bool m_TrapMousePosPerPlayer[Players::MaxPlayerCount]; //!< Whether to trap the mouse position to the middle of the screen for each player during network multiplayer.

		static constexpr double c_GamepadAxisLimit = 32767.0; //!< Maximum axis value as defined by SDL (int16 max).
		static constexpr int c_AxisDigitalPressedThreshold = 8192; //!< Digital Axis threshold value as defined by allegro.
		static constexpr int c_AxisDigitalReleasedThreshold = c_AxisDigitalPressedThreshold - 100; //!< Digital Axis release threshold, to debounce values.

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

		bool GetNetworkInputElementState(int whichPlayer, int whichElement, InputState whichState);

		/// Gets whether any generic button with the menu cursor is in the specified state.
		/// @param whichButton Which menu button to check for. See MenuButtons enumeration.
		/// @param whichState Which state to check for. See InputState enumeration.
		/// @return Whether the menu button is in the specified state or not.
		bool GetMenuButtonState(int whichButton, InputState whichState);

		/// Gets whether a keyboard key is in the specified state.
		/// @param scancodeToTest A scancode to test. See SDL_Scancode enumeration.
		/// @param whichState Which state to check for. See InputState enumeration.
		/// @return Whether the keyboard key is in the specified state or not.
		bool GetKeyboardButtonState(SDL_Scancode scancodeToTest, InputState whichState) const;

		/// Gets whether a mouse button is in the specified state.
		/// @param whichPlayer Which player to check for. See Players enumeration.
		/// @param whichButton Which mouse button to check for. See MouseButtons enumeration.
		/// @param whichState Which state to check for. See InputState enumeration.
		/// @return Whether the mouse button is in the specified state or not.
		bool GetMouseButtonState(int whichPlayer, int whichButton, InputState whichState) const;

		/// Gets whether a multiplayer mouse button is in the specified state.
		/// @param whichPlayer Which player to check for. See Players enumeration.
		/// @param whichButton Which mouse button to check for. See MouseButtons enumeration.
		/// @param whichState Which state to check for. See InputState enumeration.
		/// @return Whether the mouse button is in the specified state or not.
		bool GetNetworkMouseButtonState(int whichPlayer, int whichButton, InputState whichState) const;

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

		/// Sets a mouse button for a player to the specified state during network multiplayer.
		/// @param player Which player to set for. See Players enumeration.
		/// @param whichButton Which mouse button to set for. See MouseButtons enumeration.
		/// @param whichState Which input state to set. See InputState enumeration.
		/// @param newState The new state of the specified InputState. True or false.
		void SetNetworkMouseButtonState(int player, int whichButton, InputState whichState, bool newState);

		/// Gets whether an input element is in the specified state during network multiplayer.
		/// @param element Which element to check for. See InputElements enumeration.
		/// @param whichState Which state to check for. See InputState enumeration.
		/// @return Whether the element is in the specified state or not.
		bool NetworkAccumulatedElementState(int element, InputState whichState) const {
			return (element < InputElements::INPUT_L_UP || element >= InputElements::INPUT_COUNT) ? false : m_NetworkAccumulatedElementState[element][whichState];
		}
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
		/// @param deviceIndex The device index (generated by the connected event or a value up to SDL_NumJoysticks()).
		void HandleGamepadHotPlug(int deviceIndex);

		/// Stores all the input events that happened during this update to be compared to in the next update. This is called from Update().
		void StoreInputEventsForNextUpdate();
#pragma endregion

		/// Clears all the member variables of this UInputMan, effectively resetting the members of this abstraction level only.
		void Clear();

		// Disallow the use of some implicit methods.
		UInputMan(const UInputMan& reference) = delete;
		UInputMan& operator=(const UInputMan& rhs) = delete;
	};
} // namespace RTE
