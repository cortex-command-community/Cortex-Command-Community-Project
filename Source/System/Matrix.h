#pragma once

#include "Serializable.h"
#include "Vector.h"

namespace RTE {

	/// A 2x2 matrix to rotate 2D Vectors with.
	class Matrix : public Serializable {

	public:
		SerializableClassNameGetter;
		SerializableOverrideMethods;

		float m_Rotation; //!< The angle, represented in radians. Pi/2 points up.
		bool m_Flipped[2]; //!< Whether or not this Matrix also mirrors the respective axes of its invoked Vectors.
		float m_Elements[2][2]; //!< The elements of the matrix, which represent the negative of the angle. Allows multiplication between matrices and vectors while considering upside-down coordinate system.
		bool m_ElementsUpdated; //!< Whether the elements are currently updated to the set angle.

#pragma region Creation
		/// Constructor method used to instantiate a Matrix object.
		Matrix() { Clear(); }

		/// Constructor method used to instantiate a Matrix object from an angle.
		/// @param radAng A float of an angle in radians that this Matrix should be set to represent.
		Matrix(float radAng) {
			Clear();
			Create(radAng);
		}

		/// Copy constructor method used to instantiate a Matrix object identical to an already existing one.
		/// @param reference A Matrix object which is passed in by reference.
		Matrix(const Matrix& reference) {
			Clear();
			Create(reference);
		}

		/// Makes the Matrix object ready for use.
		/// @return An error return value signaling success or any particular failure. Anything below 0 is an error signal.
		int Create() override;

		/// Makes the Matrix object ready for use.
		/// @param angle The float angle in radians which this rotational matrix should represent.
		/// @return An error return value signaling success or any particular failure. Anything below 0 is an error signal.
		int Create(float angle);

		/// Creates a Matrix to be identical to another, by deep copy.
		/// @param reference A reference to the Matrix to deep copy.
		/// @return An error return value signaling success or any particular failure. Anything below 0 is an error signal.
		int Create(const Matrix& reference);
#pragma endregion

#pragma region Destruction
		/// Resets this Matrix to an identity Matrix, representing a 0 angle.
		void Reset() { Clear(); }
#pragma endregion

#pragma region Getters and Setters
		/// Gets whether or not this Matrix also mirrors the X component of any Vector it is multiplied with.
		/// @return A bool with the setting whether flipping the X components or not.
		bool GetXFlipped() const { return m_Flipped[X]; }

		/// Sets whether or not this Matrix should also mirror the X component of any Vector it is multiplied with.
		/// @param flipX A bool with the setting whether to flip the X components or not.
		void SetXFlipped(bool flipX = true) { m_Flipped[X] = flipX; }

		/// Gets whether or not this Matrix also mirrors the Y component of any Vector it is multiplied with.
		/// @return A bool with the setting whether flipping the Y components or not.
		bool GetYFlipped() const { return m_Flipped[Y]; }

		/// Sets whether or not this Matrix should also mirror the Y component of any Vector it is multiplied with.
		/// @param flipY A bool with the setting whether to flip the Y components or not.
		void SetYFlipped(bool flipY = true) { m_Flipped[Y] = flipY; }

		/// Returns the angle this rotational Matrix is currently representing.
		/// @return A float with the represented angle in radians.
		float GetRadAngle() const { return m_Rotation; }

		/// Sets the angle that this rotational Matrix should represent.
		/// @param newAngle A float with the new angle, in radians.
		void SetRadAngle(float newAngle) {
			m_Rotation = newAngle;
			m_ElementsUpdated = false;
		}

		/// Returns the angle this rotational Matrix is currently representing.
		/// @return A float with the represented angle in degrees.
		float GetDegAngle() const { return (m_Rotation / c_PI) * 180.0F; }

		/// Sets the angle that this rotational Matrix should represent.
		/// @param newAngle A float with the new angle, in degrees.
		void SetDegAngle(float newAngle) {
			m_Rotation = (newAngle / 180.0F) * c_PI;
			m_ElementsUpdated = false;
		}

		/// Returns the angle difference between what this is currently representing, to another angle in radians.
		/// It will wrap and normalize and give the smallest angle difference between this and the passed in.
		/// @param otherAngle A float with the angle to get the difference to from this, in radians.
		/// @return A float with the difference angle between this and the passed-in angle.
		float GetRadAngleTo(float otherAngle) const;

		/// Returns the angle difference between what this is currently representing, to another angle in degrees.
		/// It will wrap and normalize and give the smallest angle difference between this and the passed in.
		/// @param otherAngle A float with the angle to get the difference to from this, in degrees.
		/// @return A float with the difference angle between this and the passed-in angle.
		float GetDegAngleTo(float otherAngle) const;

		/// Returns the angle this rotational Matrix is currently representing.
		/// @return A float with the represented angle as full rotations being 256.
		float GetAllegroAngle() const { return (m_Rotation / c_PI) * -128.0F; }
#pragma endregion

#pragma region Operator Overloads
		/// An assignment operator for setting one Matrix equal to another.
		/// @param rhs A Matrix reference.
		/// @return A reference to the changed Matrix.
		Matrix& operator=(const Matrix& rhs);

		/// An assignment operator for setting one Matrix to represent an angle.
		/// @param rhs A float in radians to set this rotational Matrix to.
		/// @return A reference to the changed Matrix.
		Matrix& operator=(const float& rhs) {
			m_Rotation = rhs;
			m_ElementsUpdated = false;
			return *this;
		}

		/// Unary negation overload for single Matrices.
		/// @return The resulting Matrix.
		Matrix operator-();

		/// An equality operator for testing if any two Matrices are equal.
		/// @param lhs A Matrix reference as the left hand side operand.
		/// @param rhs A Matrix reference as the right hand side operand.
		/// @return A boolean indicating whether the two operands are equal or not.
		friend bool operator==(const Matrix& lhs, const Matrix& rhs) { return lhs.m_Rotation == rhs.m_Rotation; }

		/// An inequality operator for testing if any two Matrices are unequal.
		/// @param lhs A Matrix reference as the left hand side operand.
		/// @param rhs A Matrix reference as the right hand side operand.
		/// @return A boolean indicating whether the two operands are unequal or not.
		friend bool operator!=(const Matrix& lhs, const Matrix& rhs) { return !operator==(lhs, rhs); }

		/// Self-addition operator overload for a Matrix and a float.
		/// @param rhs A float reference as the right hand side operand.
		/// @return A reference to the resulting Matrix.
		Matrix& operator+=(const float& rhs) {
			m_Rotation += rhs;
			m_ElementsUpdated = false;
			return *this;
		}

		/// Self-addition operator overload for Matrices.
		/// @param lhs A Matrix reference as the left hand side operand.
		/// @param rhs A Matrix reference as the right hand side operand.
		/// @return A reference to the resulting Matrix (the left one).
		friend Matrix& operator+=(Matrix& lhs, const Matrix& rhs) {
			lhs.m_Rotation += rhs.m_Rotation;
			lhs.m_ElementsUpdated = false;
			return lhs;
		}

		/// Self-subtraction operator overload for a Matrix and a float.
		/// @param rhs A float reference as the right hand side operand.
		/// @return A reference to the resulting Matrix.
		Matrix& operator-=(const float& rhs) {
			m_Rotation -= rhs;
			m_ElementsUpdated = false;
			return *this;
		}

		/// Self-subtraction operator overload for Matrices.
		/// @param lhs A Matrix reference as the left hand side operand.
		/// @param rhs A Matrix reference as the right hand side operand.
		/// @return A reference to the resulting Matrix (the left one).
		friend Matrix& operator-=(Matrix& lhs, const Matrix& rhs) {
			lhs.m_Rotation -= rhs.m_Rotation;
			lhs.m_ElementsUpdated = false;
			return lhs;
		}

		/// Self-multiplication operator overload for a Matrix and a float.
		/// @param rhs A float reference as the right hand side operand.
		/// @return A reference to the resulting Matrix.
		Matrix& operator*=(const float& rhs) {
			m_Rotation *= rhs;
			m_ElementsUpdated = false;
			return *this;
		}

		/// Self-multiplication operator overload for Matrices.
		/// @param lhs A Matrix reference as the left hand side operand.
		/// @param rhs A Matrix reference as the right hand side operand.
		/// @return A reference to the resulting Matrix (the left one).
		friend Matrix& operator*=(Matrix& lhs, const Matrix& rhs) {
			lhs.m_Rotation *= rhs.m_Rotation;
			lhs.m_ElementsUpdated = false;
			return lhs;
		}

		/// self-division operator overload for a Matrix and a float.
		/// @param rhs A float reference as the right hand side operand.
		/// @return A reference to the resulting Matrix.
		Matrix& operator/=(const float& rhs) {
			if (rhs) {
				m_Rotation /= rhs;
				m_ElementsUpdated = false;
			}
			return *this;
		}

		/// Self-division operator overload for Matrices.
		/// @param lhs A Matrix reference as the left hand side operand.
		/// @param rhs A Matrix reference as the right hand side operand.
		/// @return A reference to the resulting Matrix (the left one).
		friend Matrix& operator/=(Matrix& lhs, const Matrix& rhs) {
			if (rhs.m_Rotation) {
				lhs.m_Rotation /= rhs.m_Rotation;
				lhs.m_ElementsUpdated = false;
			}
			return lhs;
		}

		/// Multiplication operator overload for a Matrix and a Vector. The vector will be transformed according to the Matrix's elements.
		/// Flipping, if set, is performed before rotating.
		/// @param rhs A Vector reference as the right hand side operand.
		/// @return The resulting transformed Vector.
		Vector operator*(const Vector& rhs);

		/// Multiplication operator overload for Vectors with Matrices.
		/// @param lhs A Vector reference as the left hand side operand.
		/// @param rhs A Matrix reference as the right hand side operand.
		/// @return A reference to the resulting Vector.
		friend Vector operator*(const Vector& lhs, const Matrix& rhs) {
			Matrix m(rhs);
			return m * lhs;
		}

		/// Division operator overload for a Matrix and a Vector. The vector will be transformed according to the Matrix's elements.
		/// @param rhs A Vector reference as the right hand side operand.
		/// @return The resulting transformed Vector.
		Vector operator/(const Vector& rhs);

		/// Division operator overload for Vector:s with Matrices.
		/// @param lhs A Vector reference as the left hand side operand.
		/// @param rhs A Matrix reference as the right hand side operand.
		/// @return A reference to the resulting Vector.
		friend Vector operator/(const Vector& lhs, Matrix& rhs) { return rhs / lhs; }

		/// Self-multiplication operator overload for Vector with a Matrix.
		/// @param lhs A Vector reference as the left hand side operand.
		/// @param rhs A Matrix reference as the right hand side operand.
		/// @return A reference to the resulting Vector (the left one)
		friend Vector& operator*=(Vector& lhs, Matrix& rhs) { return lhs = rhs * lhs; }

		/// Self-division operator overload for Vector with a Matrix.
		/// @param lhs A Vector reference as the left hand side operand.
		/// @param rhs A Matrix reference as the right hand side operand.
		/// @return A reference to the resulting Vector (the left one).
		friend Vector& operator/=(Vector& lhs, Matrix& rhs) { return lhs = rhs / lhs; }
#pragma endregion

	private:
		static const std::string c_ClassName; //!< A string with the friendly-formatted type name of this.

		/// Makes the elements of this matrix update to represent the set angle.
		void UpdateElements();

		/// Clears all the member variables of this Matrix, effectively resetting the members of this abstraction level only.
		void Clear();
	};
} // namespace RTE
