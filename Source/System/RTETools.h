#pragma once

// Header file for global utility methods.

#include "RTEError.h"
#include "Constants.h"

#include <random>
#include <memory>
#include <string_view>
#include <type_traits>

namespace RTE {

	class Vector;
	class Matrix;

#pragma region Random Numbers
	class RandomGenerator {
		std::mt19937 m_RNG; //!< The random number generator used for all random functions.

	public:
		/// Seed the random number generator.
		void Seed(uint64_t seed) { m_RNG.seed(seed); };

		/// Function template which returns a uniformly distributed random number in the range [-1, 1].
		/// @return Uniformly distributed random number in the range [-1, 1].
		template <typename floatType = float>
		typename std::enable_if<std::is_floating_point<floatType>::value, floatType>::type RandomNormalNum() {
			return std::uniform_real_distribution<floatType>(floatType(-1.0), std::nextafter(floatType(1.0), std::numeric_limits<floatType>::max()))(m_RNG);
		}

		/// Function template specialization for int types which returns a uniformly distributed random number in the range [-1, 1].
		/// @return Uniformly distributed random number in the range [-1, 1].
		template <typename intType>
		typename std::enable_if<std::is_integral<intType>::value, intType>::type RandomNormalNum() {
			return std::uniform_int_distribution<intType>(intType(-1), intType(1))(m_RNG);
		}

		/// Function template which returns a uniformly distributed random number in the range [0, 1].
		/// @return Uniformly distributed random number in the range [0, 1].
		template <typename floatType = float>
		typename std::enable_if<std::is_floating_point<floatType>::value, floatType>::type RandomNum() {
			return std::uniform_real_distribution<floatType>(floatType(0.0), std::nextafter(floatType(1.0), std::numeric_limits<floatType>::max()))(m_RNG);
		}

		/// Function template specialization for int types which returns a uniformly distributed random number in the range [0, 1].
		/// @return Uniformly distributed random number in the range [0, 1].
		template <typename intType>
		typename std::enable_if<std::is_integral<intType>::value, intType>::type RandomNum() {
			return std::uniform_int_distribution<intType>(intType(0), intType(1))(m_RNG);
		}

		/// Function template which returns a uniformly distributed random number in the range [min, max].
		/// @param min Lower boundary of the range to pick a number from.
		/// @param max Upper boundary of the range to pick a number from.
		/// @return Uniformly distributed random number in the range [min, max].
		template <typename floatType = float>
		typename std::enable_if<std::is_floating_point<floatType>::value, floatType>::type RandomNum(floatType min, floatType max) {
			if (max < min) {
				std::swap(min, max);
			}
			return (std::uniform_real_distribution<floatType>(floatType(0.0), std::nextafter(max - min, std::numeric_limits<floatType>::max()))(m_RNG) + min);
		}

		/// Function template specialization for int types which returns a uniformly distributed random number in the range [min, max].
		/// @param min Lower boundary of the range to pick a number from.
		/// @param max Upper boundary of the range to pick a number from.
		/// @return Uniformly distributed random number in the range [min, max].
		template <typename intType>
		typename std::enable_if<std::is_integral<intType>::value, intType>::type RandomNum(intType min, intType max) {
			if (max < min) {
				std::swap(min, max);
			}
			return (std::uniform_int_distribution<intType>(intType(0), max - min)(m_RNG) + min);
		}
	};

	extern RandomGenerator g_RandomGenerator; //!< The global random number generator used in our simulation thread.

	/// Seed global the global random number generators.
	void SeedRNG();

	// TODO: Maybe remove these passthrough functions and force the user to manually specify if they want the simulation thread random,
	// Or, in future, a render-thread random, as right now determinism isn't viable because framerate affects sim updates per draw
	template <typename floatType = float>
	typename std::enable_if<std::is_floating_point<floatType>::value, floatType>::type RandomNormalNum() {
		return g_RandomGenerator.RandomNormalNum();
	}

	template <typename intType>
	typename std::enable_if<std::is_integral<intType>::value, intType>::type RandomNormalNum() {
		return g_RandomGenerator.RandomNormalNum();
	}

	template <typename floatType = float>
	typename std::enable_if<std::is_floating_point<floatType>::value, floatType>::type RandomNum() {
		return g_RandomGenerator.RandomNum();
	}

	template <typename intType>
	typename std::enable_if<std::is_integral<intType>::value, intType>::type RandomNum() {
		return g_RandomGenerator.RandomNum();
	}

	template <typename floatType = float>
	typename std::enable_if<std::is_floating_point<floatType>::value, floatType>::type RandomNum(floatType min, floatType max) {
		return g_RandomGenerator.RandomNum(min, max);
	}

	template <typename intType>
	typename std::enable_if<std::is_integral<intType>::value, intType>::type RandomNum(intType min, intType max) {
		return g_RandomGenerator.RandomNum(min, max);
	}
#pragma endregion

#pragma region Interpolation
	/// Simple Linear Interpolation, with an added bonus: scaleStart and scaleEnd let you define your scale, where 0 and 1 would be standard scale.
	/// This scale is used to normalize your progressScalar value and Lerp accordingly.
	/// @param scaleStart The start of the scale to Lerp along.
	/// @param scaleEnd The end of the scale to Lerp along.
	/// @param startValue The start value of your Lerp.
	/// @param endValue The end value of your Lerp.
	/// @param progressScalar How far your Lerp has progressed. Automatically normalized through use of scaleStart and scaleEnd.
	/// @return Interpolated value.
	float Lerp(float scaleStart, float scaleEnd, float startValue, float endValue, float progressScalar);

	/// Simple Linear Interpolation, with an added bonus: scaleStart and scaleEnd let you define your scale, where 0 and 1 would be standard scale.
	/// This scale is used to normalize your progressScalar value and Lerp accordingly.
	/// @param scaleStart The start of the scale to Lerp along.
	/// @param scaleEnd The end of the scale to Lerp along.
	/// @param startValue The start position of your Lerp.
	/// @param endValue The end position of your Lerp.
	/// @param progressScalar How far your Lerp has progressed. Automatically normalized through use of scaleStart and scaleEnd.
	/// @return Interpolated value.
	Vector Lerp(float scaleStart, float scaleEnd, Vector startPos, Vector endPos, float progressScalar);

	/// Simple Linear Interpolation, with an added bonus: scaleStart and scaleEnd let you define your scale, where 0 and 1 would be standard scale.
	/// This scale is used to normalize your progressScalar value and Lerp accordingly.
	/// @param scaleStart The start of the scale to Lerp along.
	/// @param scaleEnd The end of the scale to Lerp along.
	/// @param startRot The start rotation of your Lerp.
	/// @param endRot The end rotation of your Lerp.
	/// @param progressScalar How far your Lerp has progressed. Automatically normalized through use of scaleStart and scaleEnd.
	/// @return Interpolated value.
	Matrix Lerp(float scaleStart, float scaleEnd, Matrix startRot, Matrix endRot, float progressScalar);

	/// Nonlinear ease-in interpolation. Starts slow.
	/// @param start Start value.
	/// @param end End value.
	/// @param progressScalar Normalized positive progress scalar (0 - 1.0).
	/// @return Interpolated value.
	float EaseIn(float start, float end, float progressScalar);

	/// Nonlinear ease-out interpolation. Slows down toward the end.
	/// @param start Start value.
	/// @param end End value.
	/// @param progressScalar Normalized positive progress scalar (0 - 1.0).
	/// @return Interpolated value.
	float EaseOut(float start, float end, float progressScalar);

	/// Nonlinear ease-in-out interpolation. Slows down in the start and end.
	/// @param start Start value.
	/// @param end End value.
	/// @param progressScalar Normalized positive progress scalar (0 - 1.0).
	/// @return Interpolated value.
	float EaseInOut(float start, float end, float progressScalar);
#pragma endregion

#pragma region Clamping
	/// Clamps a value between two limit values.
	/// @param value Value to clamp.
	/// @param upperLimit Upper limit of value.
	/// @param lowerLimit Lower limit of value.
	/// @return True if either limit is currently reached, False if not.
	bool Clamp(float& value, float upperLimit, float lowerLimit);

	/// Clamps a value between two limit values.
	/// @param value Value to clamp.
	/// @param upperLimit Upper limit of value.
	/// @param lowerLimit Lower limit of value.
	/// @return Upper/Lower limit value if limit is currently reached, value between limits if not.
	float Limit(float value, float upperLimit, float lowerLimit);
#pragma endregion

#pragma region Rounding
	/// Rounds a float to a set fixed point precision (digits after decimal point) with option to always ceil or always floor the remainder.
	/// @param inputFloat The input float to round.
	/// @param precision The precision to round to, i.e. the number of digits after the decimal points.
	/// @param roundingMode Method of rounding to use. 0 for system default, 1 for floored remainder, 2 for ceiled remainder.
	/// @return A string of the float, rounded and displayed to chosen precision.
	std::string RoundFloatToPrecision(float input, int precision, int roundingMode = 0);

	/// Rounds an integer to the specified nearest multiple.
	/// For example, if the arguments are 63 and 5, the returned value will be 65.
	/// @param num The number to round to the nearest multiple.
	/// @param multiple The multiple to round to.
	/// @return An integer rounded to the specified nearest multiple.
	inline int RoundToNearestMultiple(int num, int multiple) { return static_cast<int>(std::round(static_cast<float>(num) / static_cast<float>(multiple)) * static_cast<float>(multiple)); }
#pragma endregion

#pragma region Angle Helpers
	/// Returns a copy of the angle normalized so it's between 0 and 2PI.
	/// @param angle The angle to normalize, in radians.
	/// @return The angle, normalized so it's between 0 and 2PI
	float NormalizeAngleBetween0And2PI(float angle);

	/// Returns a copy of the angle normalized so it's between -PI and PI.
	/// @param angle The angle to normalize, in radians.
	/// @return The angle, normalized so it's between -PI and PI
	float NormalizeAngleBetweenNegativePIAndPI(float angle);

	/// Returns whether or not the angle to check is between the start and end angles. Note that, because of how angles work (when normalized), the start angle may be greater than the end angle.
	/// @param angleToCheck The angle to check, in radians.
	/// @param startAngle The starting angle for the range.
	/// @param endAngle The ending angle for the range.
	/// @return Whether or not the angle to check is between the start and end angle.
	bool AngleWithinRange(float angleToCheck, float startAngle, float endAngle);

	/// Clamps the passed in angle between the specified lower and upper limits, in a CCW direction.
	/// @param angleToClamp The angle to clamp.
	/// @param startAngle The lower limit for clamping.
	/// @param endAngle The upper limit for clamping.
	/// @return The angle, clamped between the start and end angle.
	float ClampAngle(float angleToClamp, float startAngle, float endAngle);
#pragma endregion

#pragma region Detection
	/// Tells whether a point is within a specified box.
	/// @param point Vector position of the point we're checking.
	/// @param boxPos Vector position of the box.
	/// @param width Width of the box.
	/// @param height Height of the box.
	/// @return True if point is inside box bounds.
	bool WithinBox(const Vector& point, const Vector& boxPos, float width, float height);

	/// Tells whether a point is within a specified box.
	/// @param point Vector position of the point we're checking.
	/// @param left Position of box left plane (X start).
	/// @param top Position of box top plane (Y start).
	/// @param right Position of box right plane (X end).
	/// @param bottom Position of box bottom plane (Y end).
	/// @return True if point is inside box bounds.
	bool WithinBox(const Vector& point, float left, float top, float right, float bottom);
#pragma endregion

#pragma region Conversion
	/// Returns a corrected angle value that can be used with Allegro fixed point math routines where 256 equals 360 degrees.
	/// @param angleDegrees The angle value to correct. In degrees.
	/// @return A float with the represented angle as full rotations being 256.
	inline float GetAllegroAngle(float angleDegrees) { return (angleDegrees / 360) * 256; }

	/// Returns the given angle converted from degrees to radians.
	/// @param angleDegrees The angle in degrees to be converted.
	/// @return The converted angle in radians.
	inline float DegreesToRadians(float angleDegrees) { return angleDegrees / 180.0F * c_PI; }

	/// Returns the given angle converted from radians to degrees.
	/// @param angleRadians The angle in radians to be converted.
	/// @return The converted angle in degrees.
	inline float RadiansToDegrees(float angleRadians) { return angleRadians / c_PI * 180.0F; }
#pragma endregion

#pragma region Strings
	/// Checks whether two strings are equal when the casing is disregarded.
	/// @param strA First string.
	/// @param strB Second string.
	/// @return Whether the two strings are equal case insensitively.
	inline bool StringsEqualCaseInsensitive(const std::string_view& strA, const std::string_view& strB) {
		return std::equal(strA.begin(), strA.end(), strB.begin(), strB.end(), [](char strAChar, char strBChar) { return std::tolower(strAChar) == std::tolower(strBChar); });
	}

	/// If a file "foo/Bar.txt" exists, and this method is passed "FOO/BAR.TXT", then this method will return "foo/Bar.txt".
	/// This method's purpose is to enable Linux to get the real path using a case-insensitive search.
	/// The real path is used by the Lua file I/O handling methods to ensure full Windows compatibility.
	/// @param fullPath Path to case-insensitively translate to a real path.
	/// @return The real path. If the path doesn't exist, it returns the fullPath argument with all the existing parent directories correctly capitalized.
	std::string GetCaseInsensitiveFullPath(const std::string& fullPath);

	/// Hashes a string in a cross-compiler/platform safe way (std::hash gives different results on different compilers).
	/// @param text Text string to hash.
	/// @return The hash result value.
	uint64_t Hash(const std::string& text);
#pragma endregion

#pragma region Misc
	/// Convenience method that takes in a double pointer array and returns a std::vector with its contents, because pointers-to-pointers are the devil. The passed in array is deleted in the process so no need to delete it manually.
	/// @param arrayOfType The double pointer to convert to a std::vector.
	/// @param arraySize The size of the double pointer array.
	template <typename Type> std::vector<Type*> ConvertDoublePointerToVectorOfPointers(Type** arrayOfType, size_t arraySize) {
		std::unique_ptr<Type*[]> doublePointerArray = std::unique_ptr<Type*[]>(arrayOfType);
		std::vector<Type*> outputVector;
		for (size_t i = 0; i < arraySize; ++i) {
			outputVector.emplace_back(doublePointerArray[i]);
		}
		return outputVector;
	}

	/// Returns the sign of the given input value.
	/// @return The sign as an integer -1, 0 or +1.
	template <typename Type> int Sign(const Type& value) {
		return (Type(0) < value) - (Type(0) > value);
	}
#pragma endregion
} // namespace RTE
