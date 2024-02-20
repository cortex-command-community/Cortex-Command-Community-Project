#pragma once

#include "InputMapping.h"
#include "Constants.h"

#include <array>
#include <string>

namespace RTE {

	/// A complete input configuration scheme description for a single player.
	class InputScheme : public Serializable {

	public:
		SerializableClassNameGetter;
		SerializableOverrideMethods;

		/// Enumeration for different input scheme presets.
		enum InputPreset {
			NoPreset,
			PresetArrowKeys,
			PresetWASDKeys,
			PresetMouseWASDKeys,
			PresetGenericDPad,
			PresetGenericDualAnalog,
			PresetGamepadSNES,
			PresetGamepadDS4,
			PresetGamepadXbox360,
			InputPresetCount
		};

#pragma region Creation
		/// Constructor method used to instantiate an InputScheme object in system memory. Create() should be called before using the object.
		InputScheme() { Clear(); }

		/// Creates an InputScheme to be identical to another, by deep copy.
		/// @param reference A reference to the InputScheme to deep copy.
		/// @return An error return value signaling success or any particular failure. Anything below 0 is an error signal.
		int Create(const InputScheme& reference);
#pragma endregion

#pragma region Destruction
		/// Resets the entire InputScheme, including its inherited members, to their default settings or values.
		void Reset() override { Clear(); }

		/// Resets this InputScheme to the specified player's default input device and mappings.
		/// @param player The preset player defaults this InputScheme should reset to.
		void ResetToPlayerDefaults(Players player);
#pragma endregion

#pragma region Getters and Setters
		/// Gets the InputDevice that this scheme is using.
		/// @return The InputDevice of this scheme. See InputDevice enumeration.
		InputDevice GetDevice() const { return m_ActiveDevice; }

		/// Sets the InputDevice this scheme is supposed to use.
		/// @param activeDevice The InputDevice this scheme should use. See InputDevice enumeration.
		void SetDevice(InputDevice activeDevice = InputDevice::DEVICE_KEYB_ONLY) { m_ActiveDevice = activeDevice; }

		/// Gets the InputPreset that this scheme is using.
		/// @return The InputPreset of this scheme. See InputPreset enumeration.
		InputPreset GetPreset() const { return m_SchemePreset; }

		/// Sets up a specific preset scheme that is sensible and recommended.
		/// @param schemePreset The preset number to set the scheme to match. See InputPreset enumeration.
		void SetPreset(InputPreset schemePreset = InputPreset::NoPreset);

		/// Gets the InputMappings for this.
		/// @return The input mappings array, which is INPUT_COUNT large.
		std::array<InputMapping, InputElements::INPUT_COUNT>* GetInputMappings() { return &m_InputMappings; }
#pragma endregion

#pragma region Input Mapping Getters and Setters
		/// Gets the name of the key/mouse/joystick button/direction that a particular input element is mapped to.
		/// @param whichElement Which input element to look up.
		/// @return A string with the appropriate clear text description of the mapped thing.
		std::string GetMappingName(int whichElement) const;

		/// Gets which keyboard key is mapped to a specific input element.
		/// @param whichInput Which input element to look up.
		/// @return Which keyboard key is mapped to the specified element.
		int GetKeyMapping(int whichInput) const { return m_InputMappings.at(whichInput).GetKey(); }

		/// Sets a keyboard key as mapped to a specific input element.
		/// @param whichInput Which input element to map to.
		/// @param whichKey The scan code of which keyboard key to map to above input element.
		void SetKeyMapping(int whichInput, int whichKey) { m_InputMappings.at(whichInput).SetKey(whichKey); }

		/// Gets which joystick button is mapped to a specific input element.
		/// @param whichInput Which input element to look up.
		/// @return Which joystick button is mapped to the specified element.
		int GetJoyButtonMapping(int whichInput) const { return m_InputMappings.at(whichInput).GetJoyButton(); }

		/// Sets a joystick button as mapped to a specific input element.
		/// @param whichInput Which input element to map to.
		/// @param whichButton Which joystick button to map to the specified input element.
		void SetJoyButtonMapping(int whichInput, int whichButton) { m_InputMappings.at(whichInput).SetJoyButton(whichButton); }

		/// Get the deadzone value for this control scheme.
		/// @return Joystick dead zone from 0.0 to 1.0.
		float GetJoystickDeadzone() const { return m_JoystickDeadzone; }

		/// Set the deadzone value for this control scheme.
		/// @param deadzoneValue Joystick dead zone from 0.0 to 1.0.
		void SetJoystickDeadzone(float deadzoneValue) { m_JoystickDeadzone = deadzoneValue; }

		/// Get the DeadZoneType for this control scheme.
		/// @return The DeadZoneType this scheme is set to use. See DeadZoneType enumeration.
		DeadZoneType GetJoystickDeadzoneType() const { return m_JoystickDeadzoneType; }

		/// Set the DeadZoneType for this control scheme.
		/// @param deadzoneType The DeadZoneType this scheme should use. See DeadZoneType enumeration.
		void SetJoystickDeadzoneType(DeadZoneType deadzoneType) { m_JoystickDeadzoneType = deadzoneType; }

		/// Get the digital aim speed multiplier for this control scheme.
		/// @return The digital aim speed set to this scheme.
		float GetDigitalAimSpeed() const { return m_DigitalAimSpeed; }
#pragma endregion

#pragma region Input Mapping Capture Handling
		/// Clears all mappings for a specific input element.
		/// @param whichInput Which input element to clear all mappings of.
		void ClearMapping(int whichInput) { m_InputMappings.at(whichInput).Reset(); }

		/// Checks for any key press this frame and creates an input mapping accordingly.
		/// @param whichInput Which input element to map for.
		/// @return Whether there were any key presses this frame and therefore whether a mapping was successfully captured or not.
		bool CaptureKeyMapping(int whichInput);

		/// Checks for any button or direction press this frame and creates an input mapping accordingly.
		/// @param whichJoy Which joystick to scan for button and stick presses.
		/// @param whichInput Which input element to map for.
		/// @return Whether there were any button or stick presses this frame and therefore whether a mapping was successfully captured or not.
		bool CaptureJoystickMapping(int whichJoy, int whichInput);
#pragma endregion

	protected:
		InputDevice m_ActiveDevice; //!< The currently active device for this scheme.
		InputPreset m_SchemePreset; //!< The preset this scheme was last set to, if any.

		DeadZoneType m_JoystickDeadzoneType; //!< Which deadzone type is used.
		float m_JoystickDeadzone; //!< How much of the input to treat as a deadzone input, i.e. one not registered by the game.
		float m_DigitalAimSpeed; //!< A multiplier for the digital aim speed, where 1 represents the default legacy value.

		std::array<InputMapping, InputElements::INPUT_COUNT> m_InputMappings; //!< The input element mappings of this InputScheme.

	private:
		static const std::string c_ClassName; //!< A string with the friendly-formatted type name of this object.

		/// Clears all the member variables of this InputScheme, effectively resetting the members of this abstraction level only.
		void Clear();
	};
} // namespace RTE
