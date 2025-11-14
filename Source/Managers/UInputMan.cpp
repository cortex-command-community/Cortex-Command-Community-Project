#include "UInputMan.h"
#include "Constants.h"
#include "SceneMan.h"
#include "ActivityMan.h"
#include "MetaMan.h"
#include "WindowMan.h"
#include "FrameMan.h"
#include "ConsoleMan.h"
#include "PresetMan.h"
#include "PerformanceMan.h"
#include "MenuMan.h"
#include "Icon.h"
#include "GameActivity.h"
#include "System.h"

#include <SDL3/SDL.h>
#include <array>
#include <string>
#include <unordered_map>

using namespace RTE;

std::vector<Gamepad> UInputMan::s_PrevJoystickStates(Players::MaxPlayerCount);
std::vector<Gamepad> UInputMan::s_ChangedJoystickStates(Players::MaxPlayerCount);

UInputMan::UInputMan() {
	Clear();
}

UInputMan::~UInputMan() {
	Destroy();
}

void UInputMan::Clear() {
	m_SkipHandlingSpecialInput = false;
	m_TextInput.clear();
	m_NumJoysticks = 0;
	m_OverrideInput = false;
	m_MouseSensitivity = 0.6F;
	m_TrapMousePos = false;
	m_PlayerScreenMouseBounds = {0, 0, 0, 0};
	m_MouseTrapRadius = 350;
	m_LastDeviceWhichControlledGUICursor = InputDevice::DEVICE_KEYB_ONLY;
	m_DisableKeyboard = false;
	m_DisableMouseMoving = false;
	m_PrepareToEnableMouseMoving = false;

	std::fill(std::begin(m_DeviceIcons), std::end(m_DeviceIcons), nullptr);

	// Init the previous keys, mouse and joy buttons so they don't make it seem like things have changed and also neutralize the changed keys so that no Releases will be detected initially
	m_MouseStates.clear();
	m_MouseStates[0] = {};
	m_KeyboardStates.clear();
	m_KeyboardStates[0] = {};

	for (Gamepad& gamepad: s_PrevJoystickStates) {
		if (gamepad.m_JoystickID != -1) {
			SDL_CloseGamepad(SDL_GetGamepadFromID(gamepad.m_JoystickID));
			gamepad.m_JoystickID = -1;
		}
	}

	for (int player = Players::PlayerOne; player < Players::MaxPlayerCount; ++player) {
		m_ControlScheme[player].Reset();
		m_ControlScheme[player].ResetToPlayerDefaults(static_cast<Players>(player));
	}
}

int UInputMan::Initialize() {
	int numKeys;
	m_KeyboardStates[0] = {};
	const bool* keyboardState = SDL_GetKeyboardState(&numKeys);
	std::copy(keyboardState, keyboardState + numKeys, m_KeyboardStates[0].keyStates.begin());

	m_MouseStates[0] = {};

	int controllerIndex = 0;
	int joystickCount = 0;
	SDL_JoystickID* joysticks = SDL_GetGamepads(&joystickCount);

	for (size_t index = 0; index < std::min(joystickCount, static_cast<int>(Players::MaxPlayerCount)); ++index) {
		if (SDL_IsGamepad(joysticks[index])) {
			SDL_Gamepad* controller = SDL_OpenGamepad(joysticks[index]);
			if (!controller) {
				g_ConsoleMan.PrintString("ERROR: Failed to connect gamepad " + std::to_string(index) + " " + std::string(SDL_GetError()));
				continue;
			}
			SDL_SetGamepadPlayerIndex(controller, controllerIndex);
			s_PrevJoystickStates[controllerIndex] = Gamepad(controllerIndex, joysticks[index], SDL_GAMEPAD_AXIS_COUNT, SDL_GAMEPAD_BUTTON_COUNT);
			s_ChangedJoystickStates[controllerIndex] = Gamepad(controllerIndex, joysticks[index], SDL_GAMEPAD_AXIS_COUNT, SDL_GAMEPAD_BUTTON_COUNT);
			auto playerScheme = std::find_if(m_ControlScheme.begin(), m_ControlScheme.end(), [controllerIndex](auto& scheme) { return scheme.GetDevice() == controllerIndex + InputDevice::DEVICE_GAMEPAD_1; });
			playerScheme->SetDeviceID({.gamepad = joysticks[index]});
			controllerIndex++;
			m_NumJoysticks++;
		} else {
			SDL_Joystick* joy = SDL_OpenJoystick(joysticks[index]);
			if (!joy) {
				g_ConsoleMan.PrintString("ERROR: Failed to connect joystick.");
				continue;
			}
			s_PrevJoystickStates[controllerIndex] = Gamepad(controllerIndex, joysticks[index], SDL_GetNumJoystickAxes(joy), SDL_GetNumJoystickButtons(joy));
			s_ChangedJoystickStates[controllerIndex] = Gamepad(controllerIndex, joysticks[index], SDL_GetNumJoystickAxes(joy), SDL_GetNumJoystickButtons(joy));
			auto playerScheme = std::find_if(m_ControlScheme.begin(), m_ControlScheme.end(), [controllerIndex](auto& scheme) { return scheme.GetDevice() == controllerIndex + InputDevice::DEVICE_GAMEPAD_1; });
			playerScheme->SetDeviceID({.gamepad = joysticks[index]});
			controllerIndex++;
			m_NumJoysticks++;
		}
	}

	SDL_free(joysticks);

	int playerMouseControlled = 0;
	for (int player = PlayerOne; player < MaxPlayerCount; player++) {
		if (m_ControlScheme[player].GetDevice() == InputDevice::DEVICE_MOUSE_KEYB) {
			playerMouseControlled++;
		}
	}
	m_EnableMultiMouseKeyboard = playerMouseControlled > 1 && !m_ForceDisableMultiMouseKeyboard;

	m_PlayerScreenMouseBounds = {
	    0,
	    0,
	    static_cast<int>(g_FrameMan.GetPlayerFrameBufferWidth(Players::NoPlayer) * g_WindowMan.GetResMultiplier()),
	    static_cast<int>(g_FrameMan.GetPlayerFrameBufferHeight(Players::NoPlayer) * g_WindowMan.GetResMultiplier())};

	return 0;
}

void UInputMan::LoadDeviceIcons() {
	m_DeviceIcons[InputDevice::DEVICE_KEYB_ONLY] = dynamic_cast<const Icon*>(g_PresetMan.GetEntityPreset("Icon", "Device Keyboard"));
	m_DeviceIcons[InputDevice::DEVICE_MOUSE_KEYB] = dynamic_cast<const Icon*>(g_PresetMan.GetEntityPreset("Icon", "Device Mouse"));

	for (int gamepad = InputDevice::DEVICE_GAMEPAD_1; gamepad < InputDevice::DEVICE_COUNT; gamepad++) {
		m_DeviceIcons[gamepad] = dynamic_cast<const Icon*>(g_PresetMan.GetEntityPreset("Icon", "Device Gamepad " + std::to_string(gamepad - 1)));
	}
}

Vector UInputMan::AnalogMoveValues(int whichPlayer) {
	Vector moveValues(0, 0);
	InputDevice device = m_ControlScheme.at(whichPlayer).GetDevice();
	if (device >= InputDevice::DEVICE_GAMEPAD_1) {
		int whichJoy = GetJoystickIndex(device);
		const std::array<InputMapping, InputElements::INPUT_COUNT>* inputElements = m_ControlScheme.at(whichPlayer).GetInputMappings();

		// Assume axes are stretched out over up-down, and left-right.
		if (inputElements->at(InputElements::INPUT_L_LEFT).JoyDirMapped()) {
			moveValues.SetX(AnalogAxisValue(whichJoy, inputElements->at(InputElements::INPUT_L_LEFT).GetAxis()));
		}
		if (inputElements->at(InputElements::INPUT_L_UP).JoyDirMapped()) {
			moveValues.SetY(AnalogAxisValue(whichJoy, inputElements->at(InputElements::INPUT_L_UP).GetAxis()));
		}
	}
	return moveValues;
}

Vector UInputMan::AnalogAimValues(int whichPlayer) {
	InputDevice device = m_ControlScheme.at(whichPlayer).GetDevice();

	Vector aimValues(0, 0);
	if (device == InputDevice::DEVICE_MOUSE_KEYB) {
		if (!m_EnableMultiMouseKeyboard) {
			aimValues = (m_MouseStates.at(0).analogAim / m_MouseTrapRadius);
		} else {
			if (auto mouse = m_MouseStates.find(m_ControlScheme.at(whichPlayer).GetDeviceID().mouseKeyboard.mouse); mouse != m_MouseStates.end()) {
				aimValues = mouse->second.analogAim / m_MouseTrapRadius;
			}
		}
	}
	if (device >= InputDevice::DEVICE_GAMEPAD_1) {
		int whichJoy = GetJoystickIndex(device);
		const std::array<InputMapping, InputElements::INPUT_COUNT>* inputElements = m_ControlScheme.at(whichPlayer).GetInputMappings();

		// Assume axes are stretched out over up-down, and left-right
		if (inputElements->at(InputElements::INPUT_R_LEFT).JoyDirMapped()) {
			aimValues.SetX(AnalogAxisValue(whichJoy, inputElements->at(InputElements::INPUT_R_LEFT).GetAxis()));
		}
		if (inputElements->at(InputElements::INPUT_R_UP).JoyDirMapped()) {
			aimValues.SetY(AnalogAxisValue(whichJoy, inputElements->at(InputElements::INPUT_R_UP).GetAxis()));
		}
	}
	return aimValues;
}

Vector UInputMan::GetMenuDirectional(int whichPlayer) {
	if (whichPlayer == -1) {
		Vector allInput(0.0f, 0.0f);
		for (int player = PlayerOne; player < MaxPlayerCount; player++) {
			allInput += GetMenuDirectional(player);
		}
		allInput.CapMagnitude(1.0f);
		return allInput;
	}
	Vector playerInput(0, 0);
	InputDevice device = m_ControlScheme.at(whichPlayer).GetDevice();

	switch (device) {
		case InputDevice::DEVICE_KEYB_ONLY:
			if (ElementHeld(whichPlayer, InputElements::INPUT_L_UP)) {
				playerInput.m_Y += -1.0F;
			}
			if (ElementHeld(whichPlayer, InputElements::INPUT_L_DOWN)) {
				playerInput.m_Y += 1.0F;
			}
			if (ElementHeld(whichPlayer, InputElements::INPUT_L_LEFT)) {
				playerInput.m_X += -1.0F;
			}
			if (ElementHeld(whichPlayer, InputElements::INPUT_L_RIGHT)) {
				playerInput.m_X += 1.0F;
			}
			break;
		case InputDevice::DEVICE_MOUSE_KEYB:
			// Mouse player shouldn't be doing anything here, he should be using the mouse!
			break;
		default:
			if (device == InputDevice::DEVICE_COUNT) {
				RTEAbort("Trying to control the menu cursor with an invalid input device!");
				break;
			}

			// Analog enabled device (DEVICE_GAMEPAD_1-4)
			if (AnalogMoveValues(whichPlayer).GetLargest() > 0.05F) {
				playerInput += AnalogMoveValues(whichPlayer);
				m_LastDeviceWhichControlledGUICursor = device;
			}
			if (AnalogAimValues(whichPlayer).GetLargest() > 0.05F) {
				playerInput += AnalogAimValues(whichPlayer);
				m_LastDeviceWhichControlledGUICursor = device;
			}
			break;
	}
	playerInput.CapMagnitude(1.0f);
	return playerInput;
}

bool UInputMan::AnyKeyOrJoyInput() const {
	bool input = AnyKeyPress();
	if (!input) {
		input = AnyJoyInput();
	}
	return input;
}

bool UInputMan::AnyPress() const {
	bool pressed = false;

	if (!pressed) {
		pressed = AnyKeyPress();
	}
	if (!pressed) {
		pressed = AnyMouseButtonPress();
	}
	if (!pressed) {
		pressed = AnyJoyPress();
	}

	return pressed;
}

bool UInputMan::AnyStartPress(bool includeSpacebar) {
	if (KeyPressed(SDLK_ESCAPE) || (includeSpacebar && KeyPressed(SDLK_SPACE))) {
		return true;
	}
	for (int player = Players::PlayerOne; player < Players::MaxPlayerCount; ++player) {
		if (ElementPressed(player, InputElements::INPUT_START)) {
			return true;
		}
	}
	return false;
}

bool UInputMan::AnyBackPress() {
	for (int player = Players::PlayerOne; player < Players::MaxPlayerCount; ++player) {
		if (ElementPressed(player, InputElements::INPUT_BACK)) {
			return true;
		}
	}
	return false;
}

bool UInputMan::AnyKeyPress(SDL_KeyboardID keyboardID) const {
	if (auto keyboard = m_KeyboardStates.find(keyboardID); keyboard != m_KeyboardStates.end()) {
		for (size_t testKey = SDL_SCANCODE_A; testKey < SDL_SCANCODE_COUNT; ++testKey) {
			if (keyboard->second.keyStates[testKey] && keyboard->second.changedKeyStates[testKey]) {
				return true;
			}
		}
	}
	return false;
}

int UInputMan::MouseUsedByPlayer() const {
	for (int player = Players::PlayerOne; player < Players::MaxPlayerCount; player++) {
		if (m_ControlScheme[player].GetDevice() == InputDevice::DEVICE_MOUSE_KEYB) {
			return player;
		}
	}
	return Players::NoPlayer;
}

void UInputMan::DisableMouseMoving(bool disable) {
	if (disable) {
		SDL_SetWindowRelativeMouseMode(g_WindowMan.GetWindow(), false);
		m_DisableMouseMoving = true;
		m_PrepareToEnableMouseMoving = false;
	} else {
		SDL_SetWindowRelativeMouseMode(g_WindowMan.GetWindow(), static_cast<bool>(m_TrapMousePos));
		m_PrepareToEnableMouseMoving = true;
	}
}
bool UInputMan::CheckMultiMouseKeyboardEnabled(std::optional<std::reference_wrapper<const std::vector<int>>> players) {
	if (m_ForceDisableMultiMouseKeyboard) {
		m_EnableMultiMouseKeyboard = false;
		return false;
	}
	int playerMouseControlled{0};
	if (players) {
		for (int player: players->get()) {
			if (m_ControlScheme[player].GetDevice() == InputDevice::DEVICE_MOUSE_KEYB) {
				playerMouseControlled++;
			}
		}
	} else {
		for (int player = Players::PlayerOne; player < Players::MaxPlayerCount; player++) {
			if (m_ControlScheme[player].GetDevice() == InputDevice::DEVICE_MOUSE_KEYB) {
				playerMouseControlled++;
			}
		}
	}

	m_EnableMultiMouseKeyboard = playerMouseControlled > 1 && !m_ForceDisableMultiMouseKeyboard;
	if (m_EnableMultiMouseKeyboard) {
		SDL_SetHint(SDL_HINT_WINDOWS_RAW_KEYBOARD, "1");
	}
	return m_EnableMultiMouseKeyboard;
}

bool UInputMan::AllPlayerInputDevicesKnown(const std::vector<int>& humanPlayers) const {
	if (!m_EnableMultiMouseKeyboard) {
		return true;
	}

	for (int player: humanPlayers) {
		if (m_ControlScheme[player].GetDevice() == InputDevice::DEVICE_MOUSE_KEYB) {
			DeviceID playerDevice = m_ControlScheme[player].GetDeviceID();
			if (playerDevice.mouseKeyboard.mouse == 0 || playerDevice.mouseKeyboard.keyboard == 0) {
				return false;
			}
		}
	}
	return true;
}

Vector UInputMan::GetAbsoluteMousePosition(int whichPlayer) const {
	if (!m_EnableMultiMouseKeyboard || (whichPlayer == Players::NoPlayer)) {
		return m_MouseStates.at(0).position;
	} else if (m_ControlScheme.at(whichPlayer).GetDevice() == InputDevice::DEVICE_MOUSE_KEYB) {
		if (auto mouse = m_MouseStates.find(m_ControlScheme.at(whichPlayer).GetDeviceID().mouseKeyboard.mouse); mouse != m_MouseStates.end()) {
			return mouse->second.position;
		}
	}
	return m_MouseStates.at(0).position;
}

void UInputMan::SetAbsoluteMousePosition(const Vector& pos, int whichPlayer) {
	if (whichPlayer == Players::NoPlayer || !m_EnableMultiMouseKeyboard) {
		m_MouseStates.at(0).position = pos;
	} else if (m_ControlScheme.at(whichPlayer).GetDevice() == InputDevice::DEVICE_MOUSE_KEYB) {
		if (auto mouse = m_MouseStates.find(m_ControlScheme.at(whichPlayer).GetDeviceID().mouseKeyboard.mouse); mouse != m_MouseStates.end()) {
			mouse->second.position = pos;
		}
	}
}

Vector UInputMan::GetMouseMovement(int whichPlayer) const {
	if (whichPlayer == Players::NoPlayer || (m_ControlScheme.at(whichPlayer).GetDevice() == InputDevice::DEVICE_MOUSE_KEYB && !m_EnableMultiMouseKeyboard)) {
		return m_MouseStates.at(0).relativeMotion;
	} else if (m_ControlScheme.at(whichPlayer).GetDevice() == InputDevice::DEVICE_MOUSE_KEYB) {
		if (auto mouse = m_MouseStates.find(m_ControlScheme.at(whichPlayer).GetDeviceID().mouseKeyboard.mouse); mouse != m_MouseStates.end()) {
			return mouse->second.relativeMotion;
		}
	}
	return Vector(0, 0);
}

void UInputMan::SetMouseValueMagnitude(float magCap, int whichPlayer) {
	if (whichPlayer != Players::NoPlayer && m_ControlScheme.at(whichPlayer).GetDevice() == InputDevice::DEVICE_MOUSE_KEYB) {
		if (!m_EnableMultiMouseKeyboard) {
			m_MouseStates[0].analogAim.SetMagnitude(m_MouseTrapRadius * magCap);
		} else {
			if (auto mouse = m_MouseStates.find(m_ControlScheme.at(whichPlayer).GetDeviceID().mouseKeyboard.mouse); mouse != m_MouseStates.end()) {
				mouse->second.analogAim.SetMagnitude(m_MouseTrapRadius * magCap);
			}
		}
	}
}

void UInputMan::SetMouseValueAngle(float angle, int whichPlayer) {
	if (whichPlayer != Players::NoPlayer && m_ControlScheme.at(whichPlayer).GetDevice() == InputDevice::DEVICE_MOUSE_KEYB) {
		if (!m_EnableMultiMouseKeyboard) {
			m_MouseStates[0].analogAim.SetAbsRadAngle(angle);
		} else {
			if (auto mouse = m_MouseStates.find(m_ControlScheme.at(whichPlayer).GetDeviceID().mouseKeyboard.mouse); mouse != m_MouseStates.end()) {
				mouse->second.analogAim.SetAbsDegAngle(angle);
			}
		}
	}
}

void UInputMan::SetMousePos(const Vector& newPos, int whichPlayer) {
	// Only mess with the mouse if the original mouse position is not above the screen and may be grabbing the title bar of the game window
	if (!m_DisableMouseMoving && !m_TrapMousePos && ((whichPlayer == Players::NoPlayer) || (m_ControlScheme.at(whichPlayer).GetDevice() == InputDevice::DEVICE_MOUSE_KEYB && !m_EnableMultiMouseKeyboard))) {
		SDL_WarpMouseInWindow(g_WindowMan.GetWindow(), newPos.GetFloorIntX(), newPos.GetFloorIntY());
	} else if (whichPlayer < Players::MaxPlayerCount && whichPlayer > Players::NoPlayer && m_ControlScheme.at(whichPlayer).GetDevice() == InputDevice::DEVICE_MOUSE_KEYB) {
		m_MouseStates[m_ControlScheme.at(whichPlayer).GetDeviceID().mouseKeyboard.mouse].position = newPos;
	}
}

const std::array<bool, MouseButtons::MAX_MOUSE_BUTTONS>& UInputMan::GetMouseState(int whichPlayer) const {
	if (whichPlayer == Players::NoPlayer || !m_EnableMultiMouseKeyboard) {
		return m_MouseStates.at(0).state;
	}
	InputDevice playerInput = m_ControlScheme.at(whichPlayer).GetDevice();
	if (playerInput == InputDevice::DEVICE_MOUSE_KEYB) {
		DeviceID playerDevice = m_ControlScheme.at(whichPlayer).GetDeviceID();
		if (auto mouse = m_MouseStates.find(playerDevice.mouseKeyboard.mouse); mouse != m_MouseStates.end()) {
			return mouse->second.state;
		}
	}
	return m_MouseStates.at(0).state;
}
const std::array<bool, MouseButtons::MAX_MOUSE_BUTTONS>& UInputMan::GetMouseChange(int whichPlayer) const {
	if (whichPlayer == Players::NoPlayer || !m_EnableMultiMouseKeyboard) {
		return m_MouseStates.at(0).change;
	}
	InputDevice playerInput = m_ControlScheme.at(whichPlayer).GetDevice();
	if (playerInput == InputDevice::DEVICE_MOUSE_KEYB) {
		DeviceID playerDevice = m_ControlScheme.at(whichPlayer).GetDeviceID();
		if (auto mouse = m_MouseStates.find(playerDevice.mouseKeyboard.mouse); mouse != m_MouseStates.end()) {
			return mouse->second.change;
		}
	}
	return m_MouseStates.at(0).change;
}

void UInputMan::ClearMouseButtons() {
	for (auto& [mouseID, mouse]: m_MouseStates) {
		mouse.state.fill(false);
		mouse.change.fill(false);
	}
}

int UInputMan::MouseWheelMovedByPlayer(int player) const {
	if (player == Players::NoPlayer || player < Players::PlayerOne || player >= Players::MaxPlayerCount || !m_EnableMultiMouseKeyboard) {
		return m_MouseStates.at(0).wheelChange;
	}
	InputDevice playerInput = m_ControlScheme.at(player).GetDevice();
	if (playerInput == InputDevice::DEVICE_MOUSE_KEYB) {
		DeviceID playerDevice = m_ControlScheme.at(player).GetDeviceID();
		if (auto mouse = m_MouseStates.find(playerDevice.mouseKeyboard.mouse); mouse != m_MouseStates.end()) {
			return mouse->second.wheelChange;
		}
	}
	return m_MouseStates.at(0).wheelChange;
}

bool UInputMan::AnyMouseButtonPress(SDL_MouseID mouseID) const {
	for (int button = MouseButtons::MOUSE_LEFT; button < MouseButtons::MAX_MOUSE_BUTTONS; ++button) {
		if (MouseButtonPressed(button, NoPlayer, mouseID)) {
			return true;
		}
	}
	return false;
}

void UInputMan::TrapMousePos(bool trap, int whichPlayer) {
	if (whichPlayer == Players::NoPlayer) {
		m_TrapMousePos = trap;
		SDL_SetWindowRelativeMouseMode(g_WindowMan.GetWindow(), trap);
	} else if (m_ControlScheme.at(whichPlayer).GetDevice() == InputDevice::DEVICE_MOUSE_KEYB) {
		m_TrapMousePos = trap;
		SDL_SetWindowRelativeMouseMode(g_WindowMan.GetWindow(), m_TrapMousePos || m_EnableMultiMouseKeyboard);
		if (m_EnableMultiMouseKeyboard) {
			if (auto mouse = m_MouseStates.find(m_ControlScheme.at(whichPlayer).GetDeviceID().mouseKeyboard.mouse); mouse != m_MouseStates.end()) {
				mouse->second.relativeMode = trap;
			}
		}
	}
}

void UInputMan::ForceMouseWithinBox(int x, int y, int width, int height, int whichPlayer) {
	// Only mess with the mouse if the original mouse position is not above the screen and may be grabbing the title bar of the game window.
	int rightMostPos = m_PlayerScreenMouseBounds.x + m_PlayerScreenMouseBounds.w;
	int bottomMostPos = m_PlayerScreenMouseBounds.y + m_PlayerScreenMouseBounds.h;
	if (g_WindowMan.AnyWindowHasFocus() && !m_DisableMouseMoving && !m_TrapMousePos && (whichPlayer == Players::NoPlayer || (m_ControlScheme[whichPlayer].GetDevice() == InputDevice::DEVICE_MOUSE_KEYB && !m_EnableMultiMouseKeyboard))) {
		if (g_WindowMan.FullyCoversAllDisplays()) {
			int leftPos = std::clamp(m_PlayerScreenMouseBounds.x + x, m_PlayerScreenMouseBounds.x, rightMostPos);
			int topPos = std::clamp(m_PlayerScreenMouseBounds.y + y, m_PlayerScreenMouseBounds.y, bottomMostPos);

			// The max mouse position inside the window is -1 its dimensions so we have to -1 for this to work on the right and bottom edges of the window.
			rightMostPos = std::clamp(leftPos + width, leftPos, rightMostPos - 1);
			bottomMostPos = std::clamp(topPos + height, topPos, bottomMostPos - 1);

			if (!WithinBox(m_MouseStates[0].position, static_cast<float>(leftPos), static_cast<float>(topPos), static_cast<float>(rightMostPos), static_cast<float>(bottomMostPos))) {
				int limitX = std::clamp(m_MouseStates[0].position.GetFloorIntX(), leftPos, rightMostPos);
				int limitY = std::clamp(m_MouseStates[0].position.GetFloorIntY(), topPos, bottomMostPos);
				SDL_WarpMouseInWindow(g_WindowMan.GetWindow(), limitX, limitY);
			}
		} else {
			SDL_Rect newMouseBounds = {
			    std::clamp(m_PlayerScreenMouseBounds.x + x, m_PlayerScreenMouseBounds.x, rightMostPos),
			    std::clamp(m_PlayerScreenMouseBounds.y + y, m_PlayerScreenMouseBounds.y, bottomMostPos),
			    std::clamp(width, 0, rightMostPos - x),
			    std::clamp(height, 0, bottomMostPos - y)};

			if (newMouseBounds.x >= rightMostPos || newMouseBounds.y >= bottomMostPos) {
				g_ConsoleMan.PrintString("ERROR: Trying to force mouse wihin a box that is outside the player screen bounds!");
				newMouseBounds = m_PlayerScreenMouseBounds;
			}
			SDL_SetWindowMouseRect(g_WindowMan.GetWindow(), &newMouseBounds);
		}
	} else if (whichPlayer != Players::NoPlayer && m_ControlScheme[whichPlayer].GetDevice() == InputDevice::DEVICE_MOUSE_KEYB && m_EnableMultiMouseKeyboard) {
		if (auto mouse = m_MouseStates.find(m_ControlScheme[whichPlayer].GetDeviceID().mouseKeyboard.mouse); mouse != m_MouseStates.end()) {
			Vector& position = mouse->second.position;
			position.m_X = std::clamp(position.GetFloorIntX(), m_PlayerScreenMouseBounds.x, rightMostPos);
			position.m_Y = std::clamp(position.GetFloorIntY(), m_PlayerScreenMouseBounds.y, bottomMostPos);
		}
	}
}

void UInputMan::ForceMouseWithinPlayerScreen(bool force, int whichPlayer) {
	float resMultiplier = g_WindowMan.GetResMultiplier();

	if (force && (whichPlayer >= Players::PlayerOne && whichPlayer < Players::MaxPlayerCount)) {
		int screenWidth = g_FrameMan.GetPlayerFrameBufferWidth(whichPlayer) * resMultiplier;
		int screenHeight = g_FrameMan.GetPlayerFrameBufferHeight(whichPlayer) * resMultiplier;

		switch (g_ActivityMan.GetActivity()->ScreenOfPlayer(whichPlayer)) {
			case 0:
				m_PlayerScreenMouseBounds = {0, 0, screenWidth, screenHeight};
				break;
			case 1:
				if (g_FrameMan.GetVSplit()) {
					m_PlayerScreenMouseBounds = {screenWidth, 0, screenWidth, screenHeight};
				} else {
					m_PlayerScreenMouseBounds = {0, screenHeight, screenWidth, screenHeight};
				}
				break;
			case 2:
				m_PlayerScreenMouseBounds = {0, screenHeight, screenWidth, screenHeight};
				break;
			case 3:
				m_PlayerScreenMouseBounds = {screenWidth, screenHeight, screenWidth, screenHeight};
				break;
			default:
				force = false;
		}
	} else {
		force = false;
	}

	if (force) {
		if (g_WindowMan.FullyCoversAllDisplays() || m_EnableMultiMouseKeyboard) {
			ForceMouseWithinBox(0, 0, m_PlayerScreenMouseBounds.w, m_PlayerScreenMouseBounds.h, whichPlayer);
		} else {
			SDL_SetWindowMouseRect(g_WindowMan.GetWindow(), &m_PlayerScreenMouseBounds);
		}
	} else {
		// Set the mouse bounds to the whole window so ForceMouseWithinBox is not stuck being relative to some player screen, because it can still bind the mouse even if this doesn't.
		m_PlayerScreenMouseBounds = {0, 0, static_cast<int>(g_WindowMan.GetResX() * resMultiplier), static_cast<int>(g_WindowMan.GetResY() * resMultiplier)};
		SDL_SetWindowMouseRect(g_WindowMan.GetWindow(), nullptr);
	}
}

SDL_JoystickID UInputMan::GetGamepadID(InputDevice gamepad) const {
	if (gamepad > InputDevice::DEVICE_GAMEPAD_4 || gamepad < InputDevice::DEVICE_GAMEPAD_1) {
		return 0;
	}
	int gamepadIndex = GetJoystickIndex(gamepad);
	return s_ChangedJoystickStates[gamepadIndex].m_JoystickID;
}

int UInputMan::GetJoystickAxisCount(int whichJoy) const {
	if (whichJoy >= 0 && whichJoy < s_PrevJoystickStates.size()) {
		return s_PrevJoystickStates[whichJoy].m_Axis.size();
	}
	return 0;
}

int UInputMan::WhichJoyButtonHeld(int whichJoy) const {
	if (whichJoy >= 0 && whichJoy < s_PrevJoystickStates.size()) {
		for (int button = 0; button < s_PrevJoystickStates[whichJoy].m_Buttons.size(); ++button) {
			if (s_PrevJoystickStates[whichJoy].m_Buttons[button]) {
				return button;
			}
		}
	}

	return JoyButtons::JOY_NONE;
}

int UInputMan::WhichJoyButtonPressed(int whichJoy) const {
	if (whichJoy >= 0 && whichJoy < s_PrevJoystickStates.size()) {
		for (int button = 0; button < s_PrevJoystickStates[whichJoy].m_Buttons.size(); ++button) {
			if (s_PrevJoystickStates[whichJoy].m_Buttons[button] && s_ChangedJoystickStates[whichJoy].m_Buttons[button]) {
				return button;
			}
		}
	}
	return JoyButtons::JOY_NONE;
}

float UInputMan::AnalogAxisValue(int whichJoy, int whichAxis) const {
	if (whichJoy < s_PrevJoystickStates.size() && whichAxis < s_PrevJoystickStates[whichJoy].m_Axis.size()) {
		if (s_PrevJoystickStates[whichJoy].m_JoystickID != -1) {
			float analogValue = static_cast<float>(s_PrevJoystickStates[whichJoy].m_Axis[whichAxis]) / 32767.0F;
			return analogValue;
		}
	}
	return 0;
}

bool UInputMan::AnyJoyInput(bool checkForPresses) const {
	int gamepadIndex = 0;
	for (const Gamepad& gamepad: s_PrevJoystickStates) {
		for (int button = 0; button < gamepad.m_Buttons.size(); ++button) {
			if (!checkForPresses) {
				if (gamepad.m_Buttons[button]) {
					return true;
				}
			} else if (JoyButtonPressed(gamepadIndex, button)) {
				return true;
			}
		}
		for (int axis = 0; axis < gamepad.m_Axis.size(); ++axis) {
			if (!checkForPresses) {
				if (gamepad.m_Axis[axis] != 0) {
					return true;
				}
			} else if (JoyDirectionPressed(gamepadIndex, axis, JoyDirections::JOYDIR_ONE) || JoyDirectionPressed(gamepadIndex, axis, JoyDirections::JOYDIR_TWO)) {
				return true;
			}
		}
		gamepadIndex++;
	}
	return false;
}

bool UInputMan::AnyJoyButtonPress(int whichJoy) const {
	for (int button = 0; button < s_PrevJoystickStates[whichJoy].m_Buttons.size(); ++button) {
		if (JoyButtonPressed(whichJoy, button)) {
			return true;
		}
	}
	return false;
}

bool UInputMan::GetInputElementState(int whichPlayer, int whichElement, InputState whichState) {
	bool elementState = false;
	InputDevice device = m_ControlScheme.at(whichPlayer).GetDevice();
	const InputMapping* element = &(m_ControlScheme.at(whichPlayer).GetInputMappings()->at(whichElement));

	if ((!elementState && device == InputDevice::DEVICE_KEYB_ONLY) || (device == InputDevice::DEVICE_MOUSE_KEYB && !(whichElement == InputElements::INPUT_AIM_UP || whichElement == InputElements::INPUT_AIM_DOWN))) {
		elementState = GetKeyboardButtonState(static_cast<SDL_Scancode>(element->GetKey()), whichState, whichPlayer);
	}
	if (!elementState && device == InputDevice::DEVICE_MOUSE_KEYB) {
		bool trapMouse = m_TrapMousePos;
		if (m_EnableMultiMouseKeyboard) {
			if (auto mouse = m_MouseStates.find(m_ControlScheme.at(whichPlayer).GetDeviceID().mouseKeyboard.mouse); mouse != m_MouseStates.end()) {
				trapMouse = mouse->second.relativeMode;
			}
		}
		if (trapMouse) {
			elementState = GetMouseButtonState(whichPlayer, element->GetMouseButton(), whichState);
		}
	}

	if (!elementState && device >= InputDevice::DEVICE_GAMEPAD_1) {
		int whichJoy = GetJoystickIndex(device);
		elementState = GetJoystickButtonState(whichJoy, element->GetJoyButton(), whichState);
		if (!elementState && element->JoyDirMapped()) {
			elementState = GetJoystickDirectionState(whichJoy, element->GetAxis(), element->GetDirection(), whichState);
		}
	}
	return elementState;
}

bool UInputMan::GetMenuButtonState(int whichButton, InputState whichState) {
	for (int player = Players::PlayerOne; player < Players::MaxPlayerCount; ++player) {
		bool buttonState = false;
		InputDevice device = m_ControlScheme[player].GetDevice();
		if (!buttonState && whichButton >= MenuCursorButtons::MENU_PRIMARY) {
			buttonState = GetInputElementState(player, InputElements::INPUT_FIRE, whichState) || GetMouseButtonState(NoPlayer, MouseButtons::MOUSE_LEFT, whichState);
		}
		if (!buttonState && whichButton >= MenuCursorButtons::MENU_SECONDARY) {
			buttonState = GetInputElementState(player, InputElements::INPUT_PIEMENU_DIGITAL, whichState) || GetMouseButtonState(NoPlayer, MouseButtons::MOUSE_RIGHT, whichState);
		}
		if (buttonState) {
			m_LastDeviceWhichControlledGUICursor = device;
			return buttonState;
		}
	}
	return false;
}

bool UInputMan::GetKeyboardButtonState(SDL_Scancode scancodeToTest, InputState whichState, int whichPlayer, SDL_KeyboardID keyboardID) const {
	if (m_DisableKeyboard && (scancodeToTest >= SDL_SCANCODE_0 && scancodeToTest < SDL_SCANCODE_ESCAPE)) {
		return false;
	}

	InputDevice playerDevice = InputDevice::DEVICE_COUNT;

	if (whichPlayer != Players::NoPlayer) {
		playerDevice = m_ControlScheme.at(whichPlayer).GetDevice();
	}

	if (whichPlayer != NoPlayer && playerDevice != InputDevice::DEVICE_KEYB_ONLY && playerDevice != InputDevice::DEVICE_MOUSE_KEYB) {
		return false;
	}

	std::unordered_map<SDL_KeyboardID, Keyboard>::const_iterator keyboardIterator;

	if (m_EnableMultiMouseKeyboard && keyboardID == 0 && (playerDevice == InputDevice::DEVICE_KEYB_ONLY || playerDevice == InputDevice::DEVICE_MOUSE_KEYB)) {
		if(playerDevice == InputDevice::DEVICE_KEYB_ONLY) {
			keyboardID = m_ControlScheme.at(whichPlayer).GetDeviceID().keyboard;
		} else {
			keyboardID = m_ControlScheme.at(whichPlayer).GetDeviceID().mouseKeyboard.keyboard;
		}
	}

	keyboardIterator = m_KeyboardStates.find(keyboardID);

	if (keyboardIterator == m_KeyboardStates.end()) {
		return false;
	}

	const Keyboard& keyboard = keyboardIterator->second;

	switch (whichState) {
		case InputState::Held:
			return keyboard.keyStates[scancodeToTest];
		case InputState::Pressed:
			return keyboard.keyStates[scancodeToTest] && keyboard.changedKeyStates[scancodeToTest];
		case InputState::Released:
			return !keyboard.keyStates[scancodeToTest] && keyboard.changedKeyStates[scancodeToTest];
		default:
			RTEAbort("Undefined InputState value passed in. See InputState enumeration.");
			return false;
	}
}

bool UInputMan::GetMouseButtonState(int whichPlayer, int whichButton, InputState whichState, SDL_MouseID mouseID) const {
	if (whichButton < MouseButtons::MOUSE_LEFT || whichButton >= MouseButtons::MAX_MOUSE_BUTTONS) {
		return false;
	}

	InputDevice playerDevice = InputDevice::DEVICE_COUNT;
	if (whichPlayer != Players::NoPlayer) {
		playerDevice = m_ControlScheme.at(whichPlayer).GetDevice();
	}

	if (whichPlayer != NoPlayer && playerDevice != InputDevice::DEVICE_MOUSE_KEYB) {
		return false;
	}

	std::unordered_map<SDL_MouseID, Mouse>::const_iterator mouseIterator;

	if (m_EnableMultiMouseKeyboard && mouseID == 0 && (playerDevice == InputDevice::DEVICE_MOUSE_KEYB)) {
		mouseID = m_ControlScheme.at(whichPlayer).GetDeviceID().mouseKeyboard.mouse;
	}

	mouseIterator = m_MouseStates.find(mouseID);

	if (mouseIterator == m_MouseStates.end()) {
		return false;
	}

	switch (whichState) {
		case InputState::Held:
			return mouseIterator->second.state[whichButton];
		case InputState::Pressed:
			return mouseIterator->second.state[whichButton] && mouseIterator->second.change[whichButton];
		case InputState::Released:
			return !mouseIterator->second.state[whichButton] && mouseIterator->second.change[whichButton];
		default:
			RTEAbort("Undefined InputState value passed in. See InputState enumeration.");
			return false;
	}
}

bool UInputMan::GetJoystickButtonState(int whichJoy, int whichButton, InputState whichState) const {
	if (whichJoy < 0 || whichJoy >= s_PrevJoystickStates.size() || whichButton < 0 || whichButton >= s_PrevJoystickStates[whichJoy].m_Buttons.size()) {
		return false;
	}

	bool buttonState = s_PrevJoystickStates[whichJoy].m_Buttons[whichButton];

	switch (whichState) {
		case InputState::Held:
			return buttonState;
		case InputState::Pressed:
			return buttonState && s_ChangedJoystickStates[whichJoy].m_Buttons[whichButton];
		case InputState::Released:
			return !buttonState && s_ChangedJoystickStates[whichJoy].m_Buttons[whichButton];
		default:
			RTEAbort("Undefined InputState value passed in. See InputState enumeration.");
			return false;
	}

	return false;
}

bool UInputMan::GetJoystickDirectionState(int whichJoy, int whichAxis, int whichDir, InputState whichState) const {
	if (whichJoy < 0 || whichJoy >= s_PrevJoystickStates.size() || whichAxis < 0 || whichAxis >= s_PrevJoystickStates[whichJoy].m_DigitalAxis.size()) {
		return false;
	}
	int axisState = s_PrevJoystickStates[whichJoy].m_DigitalAxis[whichAxis];

	if (whichDir == JoyDirections::JOYDIR_ONE) {
		switch (whichState) {
			case InputState::Held:
				return axisState == -1;
			case InputState::Pressed:
				return axisState == -1 && s_ChangedJoystickStates[whichJoy].m_DigitalAxis[whichAxis] < 0;
			case InputState::Released:
				return axisState == 0 && s_ChangedJoystickStates[whichJoy].m_DigitalAxis[whichAxis] > 0;
			default:
				RTEAbort("Undefined InputState value passed in. See InputState enumeration");
				return false;
		}
	} else {
		switch (whichState) {
			case InputState::Held:
				return axisState == 1;
			case InputState::Pressed:
				return axisState == 1 && s_ChangedJoystickStates[whichJoy].m_DigitalAxis[whichAxis] > 0;
			case InputState::Released:
				return axisState == 0 && s_ChangedJoystickStates[whichJoy].m_DigitalAxis[whichAxis] < 0;
			default:
				RTEAbort("Undefined InputState value passed in. See InputState enumeration.");
				return false;
		}
	}

	return false;
}

void UInputMan::HandleInputEvent(const SDL_Event& inputEvent) {
	switch (inputEvent.type) {
		case SDL_EVENT_KEY_UP:
		case SDL_EVENT_KEY_DOWN: {
			Keyboard& keyboard = m_KeyboardStates[inputEvent.key.which];
			keyboard.id = inputEvent.key.which;
			keyboard.changedKeyStates[inputEvent.key.scancode] = (inputEvent.key.down != keyboard.keyStates[inputEvent.key.scancode]);
			keyboard.keyStates[inputEvent.key.scancode] = inputEvent.key.down;

			if (inputEvent.key.which != 0) {
				m_KeyboardStates[0].changedKeyStates[inputEvent.key.scancode] = (inputEvent.key.down != m_KeyboardStates[0].keyStates[inputEvent.key.scancode]);
				m_KeyboardStates[0].keyStates[inputEvent.key.scancode] = inputEvent.key.down;
			}

			break;
		}
		case SDL_EVENT_KEYBOARD_REMOVED: {
			SDL_KeyboardID keyboardID = inputEvent.kdevice.which;
			m_KeyboardStates.erase(keyboardID);
			bool playerKeyboardDisconnected{false};
			for (int player = PlayerOne; player < MaxPlayerCount; player++) {
				DeviceID playerDeviceID = m_ControlScheme[player].GetDeviceID();
				if (m_ControlScheme[player].GetDevice() == DEVICE_MOUSE_KEYB) {
					if (playerDeviceID.mouseKeyboard.keyboard == keyboardID) {
						playerDeviceID.mouseKeyboard.keyboard = 0;
						playerKeyboardDisconnected = true;
					}
				} else if (m_ControlScheme[player].GetDevice() == DEVICE_KEYB_ONLY) {
					if (playerDeviceID.keyboard == keyboardID) {
						playerDeviceID.keyboard = 0;
						playerKeyboardDisconnected = true;
					}
				}
				if (playerKeyboardDisconnected) {
					g_ConsoleMan.PrintString("INFO: Player " + std::to_string(player + 1) + " keyboard disconnected!");
					m_PlayerMouseKeyboardKnown = false;
				}
			}
			if (playerKeyboardDisconnected && g_ActivityMan.IsInActivity()) {
				g_ActivityMan.PauseActivity();
			}
			break;
		}
		case SDL_EVENT_TEXT_INPUT: {
			char input = inputEvent.text.text[0];
			size_t i = 0;
			while (input != 0 && i < 32) {
				++i;
				if (input <= 127) {
					m_TextInput += input;
				}
				input = inputEvent.text.text[i];
			}
			break;
		}
		case SDL_EVENT_MOUSE_MOTION: {
			if (m_DisableMouseMoving) {
				break;
			}

			Mouse& mouse = m_MouseStates[inputEvent.motion.which];
			mouse.id = inputEvent.motion.which;
			mouse.position = {inputEvent.motion.x, inputEvent.motion.y};
			mouse.relativeMotion += {inputEvent.motion.xrel, inputEvent.motion.yrel};

			m_MouseStates[0].position = {inputEvent.motion.x, inputEvent.motion.y};
			m_MouseStates[0].relativeMotion += {inputEvent.motion.xrel, inputEvent.motion.yrel};

			if (g_WindowMan.FullyCoversAllDisplays()) {
				int windowPosX = 0;
				int windowPosY = 0;
				SDL_GetWindowPosition(SDL_GetWindowFromID(inputEvent.motion.windowID), &windowPosX, &windowPosY);
				Vector windowCoord(static_cast<float>(g_WindowMan.GetDisplayArrangementAbsOffsetX() + windowPosX), static_cast<float>(g_WindowMan.GetDisplayArrangementAbsOffsetY() + windowPosY));
				mouse.position += windowCoord;
				m_MouseStates[0].position += windowCoord;
			}
			break;
		}
		case SDL_EVENT_MOUSE_BUTTON_UP:
		case SDL_EVENT_MOUSE_BUTTON_DOWN: {
			if (inputEvent.button.button > SDL_BUTTON_RIGHT) {
				break;
			}
			Mouse& mouse = m_MouseStates[inputEvent.motion.which];
			mouse.id = inputEvent.button.which;
			mouse.change[inputEvent.button.button] = inputEvent.button.down != mouse.state[inputEvent.button.button];
			mouse.state[inputEvent.button.button] = inputEvent.button.down;
			if (inputEvent.button.which != 0) {
				m_MouseStates[0].change[inputEvent.button.button] = inputEvent.button.down != m_MouseStates[0].state[inputEvent.button.button];
				m_MouseStates[0].state[inputEvent.button.button] = inputEvent.button.down;
			}
			break;
		}
		case SDL_EVENT_MOUSE_WHEEL: {
			m_MouseStates[inputEvent.wheel.which].wheelChange += inputEvent.wheel.direction == SDL_MOUSEWHEEL_NORMAL ? inputEvent.wheel.y : -inputEvent.wheel.y;
			m_MouseStates[0].wheelChange += inputEvent.wheel.direction == SDL_MOUSEWHEEL_NORMAL ? inputEvent.wheel.y : -inputEvent.wheel.y;
			break;
		}
		case SDL_EVENT_MOUSE_REMOVED: {
			SDL_MouseID mouseID = inputEvent.mdevice.which;
			m_MouseStates.erase(mouseID);
			bool playerMouseDisconnected{false};
			for (int player = PlayerOne; player < MaxPlayerCount; player++) {
				if (m_ControlScheme[player].GetDevice() == DEVICE_MOUSE_KEYB) {
					DeviceID playerDeviceID = m_ControlScheme[player].GetDeviceID();
					if (playerDeviceID.mouseKeyboard.mouse == mouseID) {
						playerMouseDisconnected = true;
						playerDeviceID.mouseKeyboard.mouse = 0;
						m_ControlScheme[player].SetDeviceID(playerDeviceID);
						g_ConsoleMan.PrintString("INFO: Player " + std::to_string(player + 1) + " mouse disconnected!");
						m_PlayerMouseKeyboardKnown = false;
					}
				}
			}
			if (playerMouseDisconnected && g_ActivityMan.IsInActivity()) {
				g_ActivityMan.PauseActivity();
			}
			break;
		}
		case SDL_EVENT_GAMEPAD_AXIS_MOTION:
		case SDL_EVENT_JOYSTICK_AXIS_MOTION:
			if (std::vector<Gamepad>::iterator device = std::find(s_PrevJoystickStates.begin(), s_PrevJoystickStates.end(), inputEvent.type == SDL_EVENT_GAMEPAD_AXIS_MOTION ? inputEvent.gaxis.which : inputEvent.jaxis.which); device != s_PrevJoystickStates.end()) {
				if (SDL_IsGamepad(device->m_JoystickID) && inputEvent.type == SDL_EVENT_GAMEPAD_AXIS_MOTION) {
					UpdateJoystickAxis(device, inputEvent.gaxis.axis, inputEvent.gaxis.value);
				} else if (!SDL_IsGamepad(device->m_JoystickID)) {
					UpdateJoystickAxis(device, inputEvent.jaxis.axis, inputEvent.jaxis.value);
				}
			}
			break;
		case SDL_EVENT_JOYSTICK_BUTTON_DOWN:
		case SDL_EVENT_JOYSTICK_BUTTON_UP:
		case SDL_EVENT_GAMEPAD_BUTTON_DOWN:
		case SDL_EVENT_GAMEPAD_BUTTON_UP: {
			bool joystickEvent = (inputEvent.type == SDL_EVENT_JOYSTICK_BUTTON_DOWN || inputEvent.type == SDL_EVENT_JOYSTICK_BUTTON_UP);
			if (std::vector<Gamepad>::iterator device = std::find(s_PrevJoystickStates.begin(), s_PrevJoystickStates.end(), joystickEvent ? inputEvent.jbutton.which : inputEvent.gbutton.which); device != s_PrevJoystickStates.end()) {
				int button = -1;
				int down = false;
				if (SDL_IsGamepad(device->m_JoystickID)) {
					if (inputEvent.type == SDL_EVENT_GAMEPAD_BUTTON_UP || inputEvent.type == SDL_EVENT_GAMEPAD_BUTTON_DOWN) {
						button = inputEvent.gbutton.button;
						down = inputEvent.gbutton.down;
					} else {
						break;
					}
				} else {
					button = inputEvent.jbutton.button;
					down = inputEvent.jbutton.down;
				}
				size_t index = device - s_PrevJoystickStates.begin();
				s_ChangedJoystickStates[index].m_Buttons[button] = down != device->m_Buttons[button];
				device->m_Buttons[button] = down;
			}
			break;
		}
		case SDL_EVENT_JOYSTICK_ADDED:
		case SDL_EVENT_GAMEPAD_ADDED: {
			HandleGamepadHotPlug(inputEvent.jdevice.which);
			break;
		}
		case SDL_EVENT_JOYSTICK_REMOVED:
		case SDL_EVENT_GAMEPAD_REMOVED:
			if (std::vector<Gamepad>::iterator prevDevice = std::find(s_PrevJoystickStates.begin(), s_PrevJoystickStates.end(), inputEvent.jdevice.which); prevDevice != s_PrevJoystickStates.end()) {
				g_ConsoleMan.PrintString("INFO: Gamepad " + std::to_string(prevDevice - s_PrevJoystickStates.begin() + 1) + " disconnected!");
				SDL_CloseGamepad(SDL_GetGamepadFromID(prevDevice->m_JoystickID));
				prevDevice->m_JoystickID = -1;
				std::fill(prevDevice->m_Axis.begin(), prevDevice->m_Axis.end(), 0);
				std::fill(prevDevice->m_Buttons.begin(), prevDevice->m_Buttons.end(), false);
				m_NumJoysticks--;
			}
			if (std::vector<Gamepad>::iterator changedDevice = std::find(s_ChangedJoystickStates.begin(), s_ChangedJoystickStates.end(), inputEvent.jdevice.which); changedDevice != s_ChangedJoystickStates.end()) {
				changedDevice->m_JoystickID = -1;
				std::fill(changedDevice->m_Axis.begin(), changedDevice->m_Axis.end(), false);
				std::fill(changedDevice->m_Buttons.begin(), changedDevice->m_Buttons.end(), false);
			}
			break;
		default:
			break;
	}
}

int UInputMan::Update() {
	for (auto& [mouseID, mouse]: m_MouseStates) {
		mouse.relativeMotion *= m_MouseSensitivity;
	}

	UpdateMouseInput();
	UpdateJoystickDigitalAxis();
	HandleSpecialInput();

	return 0;
}

void UInputMan::EndFrame() {
	m_LastDeviceWhichControlledGUICursor = InputDevice::DEVICE_KEYB_ONLY;

	for (auto& [keyboardID, keyboard] : m_KeyboardStates) {
		keyboard.changedKeyStates.fill(false);
	}

	for (Gamepad& gamepad: s_ChangedJoystickStates) {
		std::fill(gamepad.m_Buttons.begin(), gamepad.m_Buttons.end(), false);
		std::fill(gamepad.m_Axis.begin(), gamepad.m_Axis.end(), 0);
		std::fill(gamepad.m_DigitalAxis.begin(), gamepad.m_DigitalAxis.end(), 0);
	}

	m_TextInput.clear();
	for (auto& [mouseID, mouse] : m_MouseStates) {
		mouse.wheelChange = 0;
		mouse.relativeMotion.Reset();
		mouse.change.fill(false);
	}
}

void UInputMan::HandleSpecialInput() {
	// If we launched into editor directly, skip the logic and quit quickly.
	if (g_ActivityMan.IsSetToLaunchIntoEditor() && KeyPressed(SDLK_ESCAPE)) {
		System::SetQuit();
		return;
	}

	if (g_ActivityMan.IsInActivity()) {
		const GameActivity* gameActivity = dynamic_cast<GameActivity*>(g_ActivityMan.GetActivity());
		// Don't allow pausing and returning to main menu when running in server mode to not disrupt the simulation for the clients
		if (AnyStartPress(false) && (!gameActivity || !gameActivity->IsBuyGUIVisible(-1))) {
			g_ActivityMan.PauseActivity(true, FlagShiftState());
			return;
		}
		// Ctrl+R or Back button for controllers to reset activity.
		if (!g_MetaMan.GameInProgress() && !g_ActivityMan.ActivitySetToRestart()) {
			g_ActivityMan.SetRestartActivity((FlagRAltState() && KeyPressed(SDLK_R)) || AnyBackPress());
		}
		if (g_ActivityMan.ActivitySetToRestart()) {
			return;
		}
	}

	if (m_SkipHandlingSpecialInput) {
		return;
	}

	if (FlagRAltState()) {
		// RAlt+S to save continuous ScreenDumps
		if (KeyHeld(SDLK_S)) {
			g_FrameMan.SaveScreenToPNG("ScreenDump");
			// RAlt+W to save a WorldDump
		} else if (KeyPressed(SDLK_W)) {
			g_FrameMan.SaveWorldToPNG("WorldDump");
			// RAlt+M to cycle draw modes
		} else if (KeyPressed(SDLK_M)) {
			g_SceneMan.SetLayerDrawMode((g_SceneMan.GetLayerDrawMode() + 1) % 3);
			// RAlt+P to toggle performance stats
		} else if (KeyPressed(SDLK_P)) {
			g_PerformanceMan.ShowPerformanceStats(!g_PerformanceMan.IsShowingPerformanceStats());
		} else if (KeyPressed(SDLK_F2)) {
			g_PresetMan.QuickReloadEntityPreset();
		} else if (KeyPressed(SDLK_F9)) {
			g_ActivityMan.LoadAndLaunchGame("AutoSave");
		} else if (g_PerformanceMan.IsShowingPerformanceStats()) {
			if (KeyHeld(SDLK_1)) {
				g_TimerMan.SetTimeScale(1.0F);
			} else if (KeyHeld(SDLK_5)) {
				g_TimerMan.SetDeltaTimeSecs(c_DefaultDeltaTimeS);
			}
		}
	} else if (FlagLAltState()) {
		if (KeyPressed(SDLK_F2)) {
			ContentFile::ReloadAllBitmaps();
			// Alt+Enter to switch resolution multiplier
		} else if (KeyPressed(SDLK_RETURN)) {
			g_WindowMan.ToggleFullscreen();
			// Alt+W to save ScenePreviewDump (miniature WorldDump)
		} else if (KeyPressed(SDLK_W)) {
			g_FrameMan.SaveWorldPreviewToPNG("ScenePreviewDump");
		} else if (g_PerformanceMan.IsShowingPerformanceStats()) {
			if (KeyPressed(SDLK_P)) {
				g_PerformanceMan.ShowAdvancedPerformanceStats(!g_PerformanceMan.AdvancedPerformanceStatsEnabled());
			}
		}
	} else {
		if (KeyPressed(SDLK_F1)) {
			g_ConsoleMan.ShowShortcuts();
		} else if (KeyPressed(SDLK_F2)) {
			g_PresetMan.ReloadAllScripts();
		} else if (KeyPressed(SDLK_F3)) {
			g_ConsoleMan.SaveAllText("Console.dump.log");
		} else if (KeyPressed(SDLK_F4)) {
			g_ConsoleMan.SaveInputLog("Console.input.log");
		} else if (KeyPressed(SDLK_F5)) {
			if (g_ActivityMan.GetActivity() && g_ActivityMan.GetActivity()->CanBeUserSaved()) {
				g_ActivityMan.SaveCurrentGame("QuickSave");
			} else {
				RTEError::ShowMessageBox("Cannot Save Game - This Activity Does Not Allow QuickSaving!");
			}
		} else if (KeyPressed(SDLK_F9)) {
			g_ActivityMan.LoadAndLaunchGame("QuickSave");
		} else if (KeyPressed(SDLK_F10)) {
			g_ConsoleMan.ClearLog();
			// F12 to save a single ScreenDump - Note that F12 triggers a breakpoint when the VS debugger is attached, regardless of config - this is by design. Thanks Microsoft.
		} else if (KeyPressed(SDLK_F12)) {
			g_FrameMan.SaveScreenToPNG("ScreenDump");
		}

		if (g_PerformanceMan.IsShowingPerformanceStats()) {
			// Manipulate time scaling
			if (KeyHeld(SDLK_2)) {
				g_TimerMan.SetTimeScale(g_TimerMan.GetTimeScale() + 0.01F);
			}
			if (KeyHeld(SDLK_1) && g_TimerMan.GetTimeScale() - 0.01F > 0.001F) {
				g_TimerMan.SetTimeScale(g_TimerMan.GetTimeScale() - 0.01F);
			}

			// Manipulate DeltaTime
			if (KeyHeld(SDLK_6)) {
				g_TimerMan.SetDeltaTimeSecs(g_TimerMan.GetDeltaTimeSecs() + 0.001F);
			}
			if (KeyHeld(SDLK_5) && g_TimerMan.GetDeltaTimeSecs() > 0) {
				g_TimerMan.SetDeltaTimeSecs(g_TimerMan.GetDeltaTimeSecs() - 0.001F);
			}
		}
	}
}

void UInputMan::UpdateMouseInput() {
	// Detect and store mouse movement input, translated to analog stick emulation
	// TODO: Figure out a less shit solution to updating the mouse in GUIs when there are no mouse players configured, i.e. no player input scheme is using mouse+keyboard. For not just check if we're out of Activity.
	for (auto& [mouseID, mouse]: m_MouseStates) {
		mouse.analogAim += mouse.relativeMotion * 3;
		mouse.analogAim.CapMagnitude(m_MouseTrapRadius);
	}

	// Only mess with the mouse pos if the original mouse position is not above the screen and may be grabbing the title bar of the game window
	if (g_WindowMan.AnyWindowHasFocus() && !m_DisableMouseMoving && (!m_TrapMousePos || m_EnableMultiMouseKeyboard)) {
		// The mouse cursor is visible and can move about the screen/window, but it should still be contained within the mouse player's part of the window
		for (int player = PlayerOne; player < MaxPlayerCount; player++) {
			if (m_ControlScheme[player].GetDevice() == InputDevice::DEVICE_MOUSE_KEYB) {
				ForceMouseWithinPlayerScreen(g_ActivityMan.IsInActivity() && !g_MenuMan.GetIsInMenuScreen(), player);
			}
		}
	}

	// Enable the mouse cursor positioning again after having been disabled. Only do this when the mouse is within the drawing area so it
	// won't cause the whole window to move if the user clicks the title bar and unintentionally drags it due to programmatic positioning.
	int mousePosX = m_MouseStates.at(0).position.m_X / g_WindowMan.GetResMultiplier();
	int mousePosY = m_MouseStates.at(0).position.m_Y / g_WindowMan.GetResMultiplier();
	if (m_DisableMouseMoving && m_PrepareToEnableMouseMoving && (mousePosX >= 0 && mousePosX < g_WindowMan.GetResX() && mousePosY >= 0 && mousePosY < g_WindowMan.GetResY())) {
		m_DisableMouseMoving = m_PrepareToEnableMouseMoving = false;
	}
}

void UInputMan::UpdateJoystickAxis(std::vector<Gamepad>::iterator device, int axis, int value) {
	if (device != s_PrevJoystickStates.end()) {
		int joystickIndex = device - s_PrevJoystickStates.begin();

		int prevAxisValue = device->m_Axis[axis];

		s_ChangedJoystickStates[joystickIndex].m_Axis[axis] = Sign(value - device->m_Axis[axis]);
		device->m_Axis[axis] = value;

		Players joystickPlayer = Players::NoPlayer;
		float deadZone = 0.0F;
		int deadZoneType = DeadZoneType::CIRCLE;

		for (int playerToCheck = Players::PlayerOne; playerToCheck < Players::MaxPlayerCount; ++playerToCheck) {
			InputDevice playerInputDevice = m_ControlScheme[playerToCheck].GetDevice();
			int whichJoy = GetJoystickIndex(playerInputDevice);
			if (whichJoy == joystickIndex) {
				deadZone = m_ControlScheme[playerToCheck].GetJoystickDeadzone();
				deadZoneType = m_ControlScheme[playerToCheck].GetJoystickDeadzoneType();
				joystickPlayer = static_cast<Players>(playerToCheck);
			}
		}

		bool isAxisMapped = false;
		if (joystickPlayer != Players::NoPlayer && deadZone > 0.0F) {
			Vector aimValues;
			const std::array<InputMapping, InputElements::INPUT_COUNT>* inputElements = m_ControlScheme[joystickPlayer].GetInputMappings();
			std::array<InputElements, 4> elementsToCheck = {InputElements::INPUT_L_LEFT, InputElements::INPUT_L_UP, InputElements::INPUT_R_LEFT, InputElements::INPUT_R_UP};

			for (size_t i = 0; i < elementsToCheck.size(); i += 2) {
				int axisLeft{SDL_GAMEPAD_AXIS_INVALID};
				int axisUp{SDL_GAMEPAD_AXIS_INVALID};
				if (inputElements->at(elementsToCheck[i]).JoyDirMapped()) {
					axisLeft = inputElements->at(elementsToCheck[i]).GetAxis();
					aimValues.m_X = AnalogAxisValue(joystickIndex, axisLeft);
					isAxisMapped = isAxisMapped || (axisLeft == axis);
				}
				if (inputElements->at(elementsToCheck[i + 1]).JoyDirMapped()) {
					axisUp = inputElements->at(elementsToCheck[i + 1]).GetAxis();
					aimValues.m_Y = AnalogAxisValue(joystickIndex, axisUp);
					isAxisMapped = isAxisMapped || (axisUp == axis);
				}
				if (!isAxisMapped) {
					continue;
				}

				if (deadZoneType == DeadZoneType::CIRCLE) {
					if (aimValues.MagnitudeIsLessThan(deadZone)) {
						if (axisLeft != SDL_GAMEPAD_AXIS_INVALID) {
							s_ChangedJoystickStates[joystickIndex].m_Axis[axisLeft] = Sign(axisLeft == axis ? -prevAxisValue : -device->m_Axis[axisLeft]);
							device->m_Axis[axisLeft] = 0;
						}
						if (axisUp != SDL_GAMEPAD_AXIS_INVALID) {
							s_ChangedJoystickStates[joystickIndex].m_Axis[axisUp] = Sign(axisUp == axis ? -prevAxisValue : -device->m_Axis[axisUp]);
							device->m_Axis[axisUp] = 0;
						}
					}
				} else if (deadZoneType == DeadZoneType::SQUARE && deadZone > 0.0F) {
					if (std::abs(static_cast<double>(value) / c_GamepadAxisLimit) < deadZone) {
						s_ChangedJoystickStates[joystickIndex].m_Axis[axis] = Sign(-prevAxisValue);
						device->m_Axis[axis] = 0;
					}
				}
			}
		}
	}
}

void UInputMan::UpdateJoystickDigitalAxis() {
	for (size_t i = 0; i < s_PrevJoystickStates.size(); ++i) {
		for (size_t axis = 0; axis < s_PrevJoystickStates[i].m_DigitalAxis.size(); ++axis) {
			int prevDigitalValue = s_PrevJoystickStates[i].m_DigitalAxis[axis];
			int newDigitalValue = 0;
			if (prevDigitalValue != 0 && std::abs(s_PrevJoystickStates[i].m_Axis[axis]) > c_AxisDigitalReleasedThreshold) {
				newDigitalValue = prevDigitalValue;
			}
			if (s_PrevJoystickStates[i].m_Axis[axis] > c_AxisDigitalPressedThreshold) {
				newDigitalValue = 1;
			} else if (s_PrevJoystickStates[i].m_Axis[axis] < -c_AxisDigitalPressedThreshold) {
				newDigitalValue = -1;
			}
			s_ChangedJoystickStates[i].m_DigitalAxis[axis] = Sign(newDigitalValue - prevDigitalValue);
			s_PrevJoystickStates[i].m_DigitalAxis[axis] = newDigitalValue;
		}
	}
}

void UInputMan::HandleGamepadHotPlug(SDL_JoystickID joystickID) {
	SDL_Joystick* controller = nullptr;
	int controllerIndex = 0;

	for (controllerIndex = 0; controllerIndex < s_PrevJoystickStates.size(); ++controllerIndex) {
		if (s_PrevJoystickStates[controllerIndex].m_JoystickID == joystickID) {
			return;
		}
		if (s_PrevJoystickStates[controllerIndex].m_JoystickID == -1) {
			if (SDL_IsGamepad(joystickID)) {
				SDL_Gamepad* gameController = SDL_OpenGamepad(joystickID);
				if (!gameController) {
					g_ConsoleMan.PrintString("ERROR: Failed to connect Gamepad!");
					break;
				}
				controller = SDL_GetGamepadJoystick(gameController);
				SDL_SetGamepadPlayerIndex(gameController, controllerIndex);
			} else {
				controller = SDL_OpenJoystick(joystickID);
			}
			if (!controller) {
				g_ConsoleMan.PrintString("ERROR: Failed to connect Gamepad!");
				break;
			}
			g_ConsoleMan.PrintString("INFO: Gamepad " + std::to_string(controllerIndex + 1) + " connected.");
			break;
		}
	}

	if (!controller && controllerIndex == s_PrevJoystickStates.size()) {
		g_ConsoleMan.PrintString("ERROR: Too many gamepads connected!");
	}

	if (controller) {
		int numAxis = 0;
		int numButtons = 0;
		if (SDL_IsGamepad(joystickID)) {
			numAxis = SDL_GAMEPAD_AXIS_COUNT;
			numButtons = SDL_GAMEPAD_BUTTON_COUNT;
		} else {
			numAxis = SDL_GetNumJoystickAxes(controller);
			numButtons = SDL_GetNumJoystickButtons(controller);
		}
		s_PrevJoystickStates[controllerIndex] = Gamepad(controllerIndex, joystickID, numAxis, numButtons);
		s_ChangedJoystickStates[controllerIndex] = Gamepad(controllerIndex, joystickID, numAxis, numButtons);
		m_NumJoysticks++;
	}
}
