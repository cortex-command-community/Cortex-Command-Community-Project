#pragma once

#include "Serializable.h"
#include "RTETools.h"

namespace RTE {

	enum Axes {
		X = 0,
		Y = 1
	};

	/// A useful 2D float vector.
	class Vector : public Serializable {

	public:
		SerializableClassNameGetter;
		SerializableOverrideMethods;

		float m_X = 0.0F; //!< X value of this vector.
		float m_Y = 0.0F; //!< Y value of this vector.

#pragma region Creation
		/// Constructor method used to instantiate a Vector object with values (0, 0).
		inline Vector() = default;

		/// Constructor method used to instantiate a Vector object from X and Y values.
		/// @param inputX Float defining the initial X value of this Vector.
		/// @param inputY Float defining the initial Y value of this Vector.
		inline Vector(const float inputX, const float inputY) :
		    m_X(inputX), m_Y(inputY){};
#pragma endregion

#pragma region Destruction
		/// Sets both the X and Y of this Vector to zero.
		inline void Reset() override {
			m_X = 0.0F;
			m_Y = 0.0F;
		}
#pragma endregion

#pragma region Getters and Setters
		/// Gets the X value of this Vector.
		/// @return A float value that represents the X value of this Vector.
		inline float GetX() const { return m_X; }

		/// Sets the X value of this Vector.
		/// @param newX A float value that the X value will be set to.
		/// @return Vector reference to this after the operation.
		inline Vector& SetX(const float newX) {
			m_X = newX;
			return *this;
		}

		/// Gets the Y value of this Vector.
		/// @return A float value that represents the Y value of this Vector.
		inline float GetY() const { return m_Y; }

		/// Sets the Y value of this Vector.
		/// @param newY A float value that the Y value will be set to.
		/// @return Vector reference to this after the operation.
		inline Vector& SetY(const float newY) {
			m_Y = newY;
			return *this;
		}

		/// Sets both the X and Y values of this Vector.
		/// @param newX A float value that the X value will be set to.
		/// @param newY A float value that the Y value will be set to.
		/// @return Vector reference to this after the operation.
		inline Vector& SetXY(const float newX, const float newY) {
			m_X = newX;
			m_Y = newY;
			return *this;
		}

		/// Gets the absolute largest of the two elements. Will always be positive.
		/// @return A float describing the largest value of the two, but not the magnitude.
		inline float GetLargest() const { return std::max(std::abs(m_X), std::abs(m_Y)); }

		/// Gets the absolute smallest of the two elements. Will always be positive.
		/// @return A float describing the smallest value of the two, but not the magnitude.
		inline float GetSmallest() const { return std::min(std::abs(m_X), std::abs(m_Y)); }

		/// Gets a Vector identical to this except that its X component is flipped.
		/// @param xFlip Whether to flip the X axis of the return vector or not.
		/// @return A copy of this vector with flipped X axis.
		inline Vector GetXFlipped(const bool xFlip = true) const { return Vector(xFlip ? -m_X : m_X, m_Y); }

		/// Flips the X element of this Vector.
		/// @param flipX Whether or not to flip the X element or not.
		/// @return Vector reference to this after the operation.
		inline Vector& FlipX(const bool flipX = true) {
			*this = GetXFlipped(flipX);
			return *this;
		}

		/// Gets a Vector identical to this except that its Y component is flipped.
		/// @param yFlip Whether to flip the Y axis of the return vector or not.
		/// @return A copy of this vector with flipped Y axis.
		inline Vector GetYFlipped(const bool yFlip = true) const { return Vector(m_X, yFlip ? -m_Y : m_Y); }

		/// Flips the Y element of this Vector.
		/// @param flipY Whether or not to flip the Y element or not.
		/// @return Vector reference to this after the operation.
		inline Vector& FlipY(const bool flipY = true) {
			*this = GetYFlipped(flipY);
			return *this;
		}

		/// Indicates whether the X component of this Vector is 0.
		/// @return Whether the X component of this Vector is 0.
		inline bool XIsZero() const { return m_X == 0; }

		/// Indicates whether the Y component of this Vector is 0.
		/// @return Whether the Y component of this Vector is 0.
		inline bool YIsZero() const { return m_Y == 0; }

		/// Indicates whether both X and Y components of this Vector are 0.
		/// @return Whether both X and Y components of this Vector are 0.
		inline bool IsZero() const { return XIsZero() && YIsZero(); }

		/// Indicates whether the X and Y components of this Vector each have opposite signs to their corresponding components of a passed in Vector.
		/// @param opp The Vector to compare with.
		/// @return Whether the X and Y components of this Vector each have opposite signs to their corresponding components of a passed in Vector.
		inline bool IsOpposedTo(const Vector& opp) const { return ((XIsZero() && opp.XIsZero()) || (std::signbit(m_X) != std::signbit(opp.m_X))) && ((YIsZero() && opp.YIsZero()) || (std::signbit(m_Y) != std::signbit(opp.m_Y))); }
#pragma endregion

#pragma region Magnitude
		/// Gets the squared magnitude of this Vector.
		/// @return A float describing the squared magnitude.
		inline float GetSqrMagnitude() const { return m_X * m_X + m_Y * m_Y; }

		/// Gets the magnitude of this Vector.
		/// @return A float describing the magnitude.
		inline float GetMagnitude() const { return std::sqrt(GetSqrMagnitude()); }

		/// Gets whether this Vector's magnitude is less than the specified value.
		/// @param magnitude A float value that this Vector's magnitude will be compared against.
		/// @return Whether this Vector's magnitude is less than the specified value.
		inline bool MagnitudeIsLessThan(float magnitude) const { return GetSqrMagnitude() < magnitude * magnitude; }

		/// Gets whether this Vector's magnitude is greater than the specified value.
		/// @param magnitude A float value that this Vector's magnitude will be compared against.
		/// @return Whether this Vector's magnitude is greater than the specified value.
		inline bool MagnitudeIsGreaterThan(float magnitude) const { return GetSqrMagnitude() > magnitude * magnitude; }

		/// Sets the magnitude of this Vector. A negative magnitude will invert the Vector's direction.
		/// @param newMag A float value that the magnitude will be set to.
		/// @return Vector reference to this after the operation.
		inline Vector& SetMagnitude(const float newMag) {
			if (IsZero()) {
				SetXY(newMag, 0.0F);
			} else {
				*this *= (newMag / GetMagnitude());
			}
			return *this;
		}

		/// Caps the magnitude of this Vector to a max value and keeps its angle intact.
		/// @param capMag A float value that the magnitude will be capped by.
		/// @return Vector reference to this after the operation.
		inline Vector& CapMagnitude(const float capMag) {
			if (capMag == 0.0F) {
				Reset();
			}
			if (MagnitudeIsGreaterThan(capMag)) {
				SetMagnitude(capMag);
			}
			return *this;
		}

		/// Clamps the magnitude of this Vector between the upper and lower limits, and keeps its angle intact.
		/// @param lowerMagnitudeLimit A float value that defines the lower limit for the magnitude of this Vector.
		/// @param upperMagnitudeLimit A float value that defines the upper limit for the magnitude of this Vector.
		/// @return A reference to this after the change.
		inline Vector& ClampMagnitude(float lowerMagnitudeLimit, float upperMagnitudeLimit) {
			if (upperMagnitudeLimit < lowerMagnitudeLimit) {
				std::swap(upperMagnitudeLimit, lowerMagnitudeLimit);
			}
			if (upperMagnitudeLimit == 0.0F && lowerMagnitudeLimit == 0.0F) {
				Reset();
			} else if (MagnitudeIsLessThan(lowerMagnitudeLimit)) {
				SetMagnitude(lowerMagnitudeLimit);
			} else if (MagnitudeIsGreaterThan(upperMagnitudeLimit)) {
				SetMagnitude(upperMagnitudeLimit);
			}
			return *this;
		}

		/// Returns a Vector that has the same direction as this but with a magnitude of 1.0.
		/// @return A normalized copy of this vector.
		inline Vector GetNormalized() const { return *this / GetMagnitude(); }

		/// Scales this vector to have the same direction but a magnitude of 1.0.
		/// @return Vector reference to this after the operation.
		inline Vector& Normalize() {
			*this = GetNormalized();
			return *this;
		}
#pragma endregion

#pragma region Rotation
		/// Get this Vector's absolute angle in radians. e.g: when x = 1, y = 0, the value returned here will be 0. x = 0, y = 1 yields -pi/2 here.
		/// @return The absolute angle in radians, in the interval [-0.5 pi, 1.5 pi).
		inline float GetAbsRadAngle() const {
			const float radAngle = -std::atan2(m_Y, m_X);
			return (radAngle < -c_HalfPI) ? (radAngle + c_TwoPI) : radAngle;
		}

		/// Sets this Vector's absolute angle in radians.
		/// @param newAbsRadAngle The Vector's new absolute angle in radians.
		inline void SetAbsRadAngle(float newAbsRadAngle) { RadRotate(newAbsRadAngle - GetAbsRadAngle()); }

		/// Get this Vector's absolute angle in degrees. e.g: when x = 1, y = 0, the value returned here will be 0. x = 0, y = 1 yields -90 here.
		/// @return The absolute angle in degrees, in the interval [-90, 270).
		inline float GetAbsDegAngle() const { return GetAbsRadAngle() / c_PI * 180.0F; }

		/// Sets this Vector's absolute angle in degrees.
		/// @param newAbsDegAngle The Vector's new absolute angle in degrees.
		inline void SetAbsDegAngle(float newAbsDegAngle) { DegRotate(newAbsDegAngle - GetAbsDegAngle()); }

		/// Returns a copy of this Vector, rotated relatively by an angle in radians.
		/// @param angle The angle in radians to rotate by. Positive angles rotate counter-clockwise, and negative angles clockwise.
		/// @return A rotated copy of this Vector.
		inline Vector GetRadRotatedCopy(const float angle) {
			Vector returnVector = *this;
			const float adjustedAngle = -angle;
			returnVector.m_X = m_X * std::cos(adjustedAngle) - m_Y * std::sin(adjustedAngle);
			returnVector.m_Y = m_X * std::sin(adjustedAngle) + m_Y * std::cos(adjustedAngle);
			return returnVector;
		}

		/// Rotate this Vector relatively by an angle in radians.
		/// @param angle The angle in radians to rotate by. Positive angles rotate counter-clockwise, and negative angles clockwise.
		/// @return Vector reference to this after the operation.
		inline Vector& RadRotate(const float angle) {
			*this = GetRadRotatedCopy(angle);
			return *this;
		}

		/// Returns a copy of this Vector, rotated relatively by an angle in degrees.
		/// @param angle The angle in degrees to rotate by. Positive angles rotate counter-clockwise, and negative angles clockwise.
		/// @return A rotated copy of this Vector.
		inline Vector GetDegRotatedCopy(const float angle) { return GetRadRotatedCopy(angle * c_PI / 180.0F); };

		/// Rotate this Vector relatively by an angle in degrees.
		/// @param angle The angle in degrees to rotate by. Positive angles rotate counter-clockwise, and negative angles clockwise.
		/// @return Vector reference to this after the operation.
		inline Vector& DegRotate(const float angle) {
			*this = GetDegRotatedCopy(angle);
			return *this;
		}

		/// Set this Vector to an absolute rotation based on the absolute rotation of another Vector.
		/// @param refVector The reference Vector whose absolute angle from positive X (0 degrees) this Vector will be rotated to.
		/// @return Vector reference to this after the operation.
		inline Vector& AbsRotateTo(const Vector& refVector) { return RadRotate(refVector.GetAbsRadAngle() - GetAbsRadAngle()); }

		/// Returns a Vector that is perpendicular to this, rotated PI/2.
		/// @return A Vector that is perpendicular to this, rotated PI/2.
		inline Vector GetPerpendicular() const { return Vector(m_Y, -m_X); }

		/// Makes this vector perpendicular to its previous state, rotated PI/2. Much faster than RadRotate by PI/2.
		/// @return Vector reference to this after the operation.
		inline Vector& Perpendicularize() {
			*this = GetPerpendicular();
			return *this;
		}
#pragma endregion

#pragma region Rounding
		/// Rounds the X and Y values of this Vector upwards. E.g. 0.49 -> 0.0 and 0.5 -> 1.0.
		/// @return Vector reference to this after the operation.
		inline Vector& Round() {
			*this = GetRounded();
			return *this;
		}

		/// Sets the X and Y of this Vector to the nearest half value. E.g. 1.0 -> 1.5 and 0.9 -> 0.5.
		/// @return Vector reference to this after the operation.
		inline Vector& ToHalf() {
			m_X = std::round(m_X * 2) / 2;
			m_Y = std::round(m_Y * 2) / 2;
			return *this;
		}

		/// Sets the X and Y of this Vector to the greatest integers that are not greater than their original values. E.g. -1.02 becomes -2.0.
		/// @return Vector reference to this after the operation.
		inline Vector& Floor() {
			*this = GetFloored();
			return *this;
		}

		/// Sets the X and Y of this Vector to the lowest integers that are not less than their original values. E.g. -1.02 becomes -1.0.
		/// @return Vector reference to this after the operation.
		inline Vector& Ceiling() {
			*this = GetCeilinged();
			return *this;
		}

		/// Returns a rounded copy of this Vector. Does not alter this Vector.
		/// @return A rounded copy of this Vector.
		inline Vector GetRounded() const { return Vector(std::round(m_X), std::round(m_Y)); }

		/// Returns the rounded integer X value of this Vector.
		/// @return An int value that represents the X value of this Vector.
		inline int GetRoundIntX() const { return static_cast<int>(std::round(m_X)); }

		/// Returns the rounded integer Y value of this Vector.
		/// @return An int value that represents the Y value of this Vector.
		inline int GetRoundIntY() const { return static_cast<int>(std::round(m_Y)); }

		/// Returns a floored copy of this Vector. Does not alter this Vector.
		/// @return A floored copy of this Vector.
		inline Vector GetFloored() const { return Vector(std::floor(m_X), std::floor(m_Y)); }

		/// Returns the greatest integer that is not greater than the X value of this Vector.
		/// @return An int value that represents the X value of this Vector.
		inline int GetFloorIntX() const { return static_cast<int>(std::floor(m_X)); }

		/// Returns the greatest integer that is not greater than the Y value of this Vector.
		/// @return An int value that represents the Y value of this Vector.
		inline int GetFloorIntY() const { return static_cast<int>(std::floor(m_Y)); }

		/// Returns a ceilinged copy of this Vector. Does not alter this Vector.
		/// @return A ceilinged copy of this Vector.
		inline Vector GetCeilinged() const { return Vector(std::ceil(m_X), std::ceil(m_Y)); }

		/// Returns the lowest integer that is not less than the X value of this Vector.
		/// @return An int value that represents the X value of this Vector.
		inline int GetCeilingIntX() const { return static_cast<int>(std::ceil(m_X)); }

		/// Returns the lowest integer that is not less than the Y value of this Vector.
		/// @return An int value that represents the Y value of this Vector.
		inline int GetCeilingIntY() const { return static_cast<int>(std::ceil(m_Y)); }
#pragma endregion

#pragma region Vector Products
		/// Returns the dot product of this Vector and the passed in Vector.
		/// @param rhs The Vector which will be the right hand side operand of the dot product operation.
		/// @return The resulting dot product scalar float.
		inline float Dot(const Vector& rhs) const { return (m_X * rhs.m_X) + (m_Y * rhs.m_Y); }

		/// Returns the 2D cross product of this Vector and the passed in Vector. This is really the area of the parallelogram that the two vectors form.
		/// @param rhs The Vector which will be the right hand side operand of the cross product operation.
		/// @return The resulting 2D cross product parallelogram area.
		inline float Cross(const Vector& rhs) const { return (m_X * rhs.m_Y) - (rhs.m_X * m_Y); }
#pragma endregion

#pragma region Operator Overloads
		/// Copy assignment operator for Vectors.
		/// @param rhs A Vector reference.
		/// @return A reference to the changed Vector.
		inline Vector& operator=(const Vector& rhs) {
			m_X = rhs.m_X;
			m_Y = rhs.m_Y;
			return *this;
		}

		/// Unary negation overload for single Vectors.
		/// @return The resulting Vector.
		inline Vector operator-() { return Vector(-m_X, -m_Y); }

		/// An equality operator for testing if any two Vectors are equal.
		/// @param lhs A Vector reference as the left hand side operand.
		/// @param rhs A Vector reference as the right hand side operand.
		/// @return A boolean indicating whether the two operands are equal or not.
		inline friend bool operator==(const Vector& lhs, const Vector& rhs) { return lhs.m_X == rhs.m_X && lhs.m_Y == rhs.m_Y; }

		/// An inequality operator for testing if any two Vectors are unequal.
		/// @param lhs A Vector reference as the left hand side operand.
		/// @param rhs A Vector reference as the right hand side operand.
		/// @return A boolean indicating whether the two operands are unequal or not.
		inline friend bool operator!=(const Vector& lhs, const Vector& rhs) { return !(lhs == rhs); }

		/// A stream insertion operator for sending a Vector to an output stream.
		/// @param stream An ostream reference as the left hand side operand.
		/// @param operand A Vector reference as the right hand side operand.
		/// @return An ostream reference for further use in an expression.
		inline friend std::ostream& operator<<(std::ostream& stream, const Vector& operand) {
			stream << "{" << operand.m_X << ", " << operand.m_Y << "}";
			return stream;
		}

		/// Addition operator overload for Vectors.
		/// @param lhs A Vector reference as the left hand side operand.
		/// @param rhs A Vector reference as the right hand side operand.
		/// @return The resulting Vector.
		inline friend Vector operator+(const Vector& lhs, const Vector& rhs) { return Vector(lhs.m_X + rhs.m_X, lhs.m_Y + rhs.m_Y); }

		/// Subtraction operator overload for Vectors.
		/// @param lhs A Vector reference as the left hand side operand.
		/// @param rhs A Vector reference as the right hand side operand.
		/// @return The resulting Vector.
		inline friend Vector operator-(const Vector& lhs, const Vector& rhs) { return Vector(lhs.m_X - rhs.m_X, lhs.m_Y - rhs.m_Y); }

		/// Multiplication operator overload for a Vector and a float.
		/// @param rhs A float reference as the right hand side operand.
		/// @return The resulting Vector.
		inline Vector operator*(const float& rhs) const { return Vector(m_X * rhs, m_Y * rhs); }

		/// Multiplication operator overload for Vectors.
		/// @param lhs A Vector reference as the left hand side operand.
		/// @param rhs A Vector reference as the right hand side operand.
		/// @return The resulting Vector.
		inline friend Vector operator*(const Vector& lhs, const Vector& rhs) { return Vector(lhs.m_X * rhs.m_X, lhs.m_Y * rhs.m_Y); }

		/// Division operator overload for a Vector and a float.
		/// @param rhs A float reference as the right hand side operand.
		/// @return The resulting Vector.
		Vector operator/(const float& rhs) const { return (rhs != 0) ? Vector(m_X / rhs, m_Y / rhs) : Vector(0, 0); }

		/// Division operator overload for Vectors.
		/// @param lhs A Vector reference as the left hand side operand.
		/// @param rhs A Vector reference as the right hand side operand.
		/// @return The resulting Vector.
		inline friend Vector operator/(const Vector& lhs, const Vector& rhs) { return (rhs.m_X != 0 && rhs.m_Y != 0) ? Vector(lhs.m_X / rhs.m_X, lhs.m_Y / rhs.m_Y) : Vector(0, 0); }

		/// Self-addition operator overload for Vectors.
		/// @param lhs A Vector reference as the left hand side operand.
		/// @param rhs A Vector reference as the right hand side operand.
		/// @return A reference to the resulting Vector (the left one).
		inline friend Vector& operator+=(Vector& lhs, const Vector& rhs) {
			lhs.m_X += rhs.m_X;
			lhs.m_Y += rhs.m_Y;
			return lhs;
		}

		/// Self-subtraction operator overload for Vectors.
		/// @param lhs A Vector reference as the left hand side operand.
		/// @param rhs A Vector reference as the right hand side operand.
		/// @return A reference to the resulting Vector (the left one).
		inline friend Vector& operator-=(Vector& lhs, const Vector& rhs) {
			lhs.m_X -= rhs.m_X;
			lhs.m_Y -= rhs.m_Y;
			return lhs;
		}

		/// Self-multiplication operator overload for a Vector and a float.
		/// @param rhs A float reference as the right hand side operand.
		/// @return A reference to the resulting Vector.
		inline Vector& operator*=(const float& rhs) {
			m_X *= rhs;
			m_Y *= rhs;
			return *this;
		}

		/// Self-multiplication operator overload for Vectors.
		/// @param lhs A Vector reference as the left hand side operand.
		/// @param rhs A Vector reference as the right hand side operand.
		/// @return A reference to the resulting Vector (the left one).
		inline friend Vector& operator*=(Vector& lhs, const Vector& rhs) {
			lhs.m_X *= rhs.m_X;
			lhs.m_Y *= rhs.m_Y;
			return lhs;
		}

		/// self-division operator overload for a Vector and a float.
		/// @param rhs A float reference as the right hand side operand.
		/// @return A reference to the resulting Vector.
		inline Vector& operator/=(const float& rhs) {
			if (rhs != 0) {
				m_X /= rhs;
				m_Y /= rhs;
			}
			return *this;
		}

		/// Self-division operator overload for Vectors.
		/// @param lhs A Vector reference as the left hand side operand.
		/// @param rhs A Vector reference as the right hand side operand.
		/// @return A reference to the resulting Vector (the left one).
		inline friend Vector& operator/=(Vector& lhs, const Vector& rhs) {
			lhs.m_X /= rhs.m_X;
			lhs.m_Y /= rhs.m_Y;
			return lhs;
		}

		/// Array subscripting to access either the X or Y element of this Vector.
		/// @param rhs An int index indicating which element is requested (X = 0, Y = 1).
		/// @return The requested element.
		inline const float& operator[](const int& rhs) const { return (rhs == 0) ? m_X : m_Y; }

		/// Array subscripting to access either the X or Y element of this Vector.
		/// @param rhs An int index indicating which element is requested (X = 0, Y = 1).
		/// @return The requested element.
		inline float& operator[](const int& rhs) { return (rhs == 0) ? m_X : m_Y; }
#pragma endregion

	private:
		static const std::string c_ClassName; //!< A string with the friendly-formatted type name of this.
	};
} // namespace RTE
