#pragma once

#include "Serializable.h"

namespace RTE {

	/// A class representing a RGB color value.
	class Color : public Serializable {

	public:
		SerializableClassNameGetter;
		SerializableOverrideMethods;

#pragma region Creation
		/// Constructor method used to instantiate a Color object.
		Color() { Clear(); }

		/// Constructor method used to instantiate a Color object from RGB values.
		/// @param R Initial Red value of this color.
		/// @param G Initial Green value of this color.
		/// @param B Initial Blue value of this color.
		Color(int R, int G, int B) {
			Clear();
			Create(R, G, B);
		}

		/// Constructor method used to instantiate a Color object from an entry in the current color palette.
		/// @param index Palette index entry to create this color from.
		Color(int index) {
			Clear();
			SetRGBWithIndex(index);
		}

		/// Copy constructor method used to instantiate a Color object identical to an already existing one.
		/// @param reference A Color object which is passed in by reference.
		Color(const Color& reference) {
			Clear();
			Create(reference.m_R, reference.m_G, reference.m_B);
		}

		/// Makes the Color object ready for use.
		/// @return An error return value signaling success or any particular failure. Anything below 0 is an error signal.
		int Create() override;

		/// Makes the Color object ready for use.
		/// @param R Initial Red value of this color.
		/// @param G Initial Green value of this color.
		/// @param B Initial Blue value of this color.
		/// @return An error return value signaling success or any particular failure. Anything below 0 is an error signal.
		int Create(int inputR, int inputG, int inputB);
#pragma endregion

#pragma region Destruction
		/// Sets RGB of this Color to zero.
		void Reset() override { Clear(); }
#pragma endregion

#pragma region Getters and Setters
		/// Gets the entry in the current color palette that most closely matches this Color's RGB values.
		/// @return The color entry index number.
		int GetIndex() const { return m_Index; }

		/// Sets all three RGB values of this Color, using an index from the current color palette.
		/// @param index The index of the palette entry that this Color object's RGB values should be set to.
		void SetRGBWithIndex(int index);

		/// Gets the red value of this Color.
		/// @return An integer value that represents the R value of this Color. 0 - 255.
		int GetR() const { return m_R; }

		/// Sets the red value of this Color.
		/// @param newR An integer value that the R value will be set to, between 0 and 255.
		void SetR(int newR) {
			m_R = std::clamp(newR, 0, 255);
			m_Index = 0;
		}

		/// Gets the green value of this Color.
		/// @return An integer value that represents the green value of this Color. 0 - 255.
		int GetG() const { return m_G; }

		/// Sets the green value of this Color.
		/// @param newG An integer value that the green value will be set to, between 0 and 255.
		void SetG(int newG) {
			m_G = std::clamp(newG, 0, 255);
			m_Index = 0;
		}

		/// Gets the blue value of this Color.
		/// @return An integer value that represents the blue value of this Color. 0 - 255.
		int GetB() const { return m_B; }

		/// Sets the blue value of this Color.
		/// @param newB An integer value that the blue value will be set to, between 0 and 255.
		void SetB(int newB) {
			m_B = std::clamp(newB, 0, 255);
			m_Index = 0;
		}

		/// Sets all three RGB values of this Color.
		/// @param newR Integer value that the Red value will be set to, between 0 and 255.
		/// @param newG Integer value that the Green value will be set to, between 0 and 255.
		/// @param newB Integer value that the Blue value will be set to, between 0 and 255.
		void SetRGB(int newR, int newG, int newB) {
			SetR(newR);
			SetG(newG);
			SetB(newB);
			m_Index = 0;
		}
#pragma endregion

#pragma region Concrete Methods
		/// Causes recalculation of the nearest index even though there might be one cached or not.
		/// @return The new color entry index number.
		int RecalculateIndex();
#pragma endregion

	protected:
		int m_R; //!< Red value of this color.
		int m_G; //!< Green value of this color.
		int m_B; //!< Blue value of this color.
		int m_Index; //!< The closest matching index in the current color palette. If 0, this needs to be recalculated and updated.

	private:
		static const std::string c_ClassName; //!< A string with the friendly-formatted type name of this.

		/// Clears all the member variables of this Color, effectively resetting the members of this abstraction level only.
		void Clear() { m_R = m_G = m_B = m_Index = 0; }
	};
} // namespace RTE
