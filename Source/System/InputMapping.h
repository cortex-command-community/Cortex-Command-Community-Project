#pragma once

#include "Serializable.h"

namespace RTE {

	/// A map between an input element and specific input device elements.
	class InputMapping : public Serializable {

	public:
		SerializableClassNameGetter;
		SerializableOverrideMethods;

#pragma region Creation
		/// Constructor method used to instantiate an InputMapping object in system memory. Create() should be called before using the object.
		InputMapping() { Clear(); }

		/// Creates an InputMapping to be identical to another, by deep copy.
		/// @param reference A reference to the InputMapping to deep copy.
		/// @return An error return value signaling success or any particular failure. Anything below 0 is an error signal.
		int Create(const InputMapping& reference);
#pragma endregion

#pragma region Destruction
		/// Resets the entire InputMapping, including its inherited members, to their default settings or values.
		void Reset() override { Clear(); }
#pragma endregion

#pragma region Getters and Setters
		/// Gets the description of the input scheme preset that this element is part of, if any preset has been set for this element's scheme.
		/// @return The description associated with this element by the scheme preset, if any has been set. This string is empty otherwise.
		std::string GetPresetDescription() const { return m_PresetDescription; }

		/// Sets the description of the input scheme preset that this element is part of, if any preset has been set for this element's scheme.
		/// @param presetDescription The description associated with this element by the scheme preset, if any has been set. This string should be empty otherwise.
		void SetPresetDescription(const std::string& presetDescription) { m_PresetDescription = presetDescription; }
#pragma endregion

#pragma region Keyboard Getters and Setters
		/// Gets the keyboard key mapping.
		/// @return The keyboard key this is mapped to.
		int GetKey() const { return m_KeyMap; }

		/// Sets the keyboard button mapping.
		/// @param newKey The scan code of the new key to map to.
		void SetKey(int newKey) { m_KeyMap = newKey; }
#pragma endregion

#pragma region Mouse Getters and Setters
		/// Gets the mouse button mapping.
		/// @return The number of the mouse button this is mapped to.
		int GetMouseButton() const { return m_MouseButtonMap; }

		/// Sets the mouse button mapping.
		/// @param newButton The number of the mouse button this should be mapped to.
		void SetMouseButton(int newButton) { m_MouseButtonMap = newButton; }
#pragma endregion

#pragma region Joystick Getters and Setters
		/// Shows whether this is mapped to a joy direction or not.
		/// @return Joy direction mapped or not.
		bool JoyDirMapped() const { return m_DirectionMapped; }

		/// Gets the joystick stick number that this is mapped to.
		/// @return The direction, UInputMan::JOYDIR_ONE or UInputMan::JOYDIR_TWO.
		int GetDirection() const { return m_DirectionMap; }

		/// Sets the joystick direction mapping.
		/// @param newAxis The number of the axis this should be mapped to.
		/// @param newDirection The number of the direction this should be mapped to.
		void SetDirection(int newAxis, int newDirection) {
			m_DirectionMapped = true;
			m_AxisMap = newAxis;
			m_DirectionMap = newDirection;
		}

		/// Gets the joystick button mapping.
		/// @return The number of the joystick button this is mapped to.
		int GetJoyButton() const { return m_JoyButtonMap; }

		/// Sets the joystick button mapping.
		/// @param newButton The number of the joystick button this should be mapped to.
		void SetJoyButton(int newButton) { m_JoyButtonMap = newButton; }

		/// Gets the joystick axis number that this is mapped to.
		/// @return The joystick axis number.
		int GetAxis() const { return m_AxisMap; }
#pragma endregion

	protected:
		std::string m_PresetDescription; //!< The friendly description that is associated with the scheme preset element, if any is set.

		int m_KeyMap; //!< The keyboard key mapping.
		int m_MouseButtonMap; //!< The mouse button mapping.

		bool m_DirectionMapped; //!< Whether joystick direction mapping is enabled.

		int m_JoyButtonMap; //!< The joystick button mapping.
		int m_AxisMap; //!< The joystick axis mapping.
		int m_DirectionMap; //!< The joystick direction mapping.

	private:
		static const std::string c_ClassName; //!< A string with the friendly-formatted type name of this object.

		/// Clears all the member variables of this InputMapping, effectively resetting the members of this abstraction level only.
		void Clear();
	};
} // namespace RTE
