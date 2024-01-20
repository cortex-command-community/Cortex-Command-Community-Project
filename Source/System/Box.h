#pragma once

#include "Serializable.h"
#include "Vector.h"

namespace RTE {

	/// A simple rectangle with integer coordinates.
	struct IntRect {

		int m_Left = 0; //!< X position of the IntRect top left corner.
		int m_Top = 0; //!< Y position of the IntRect top left corner.
		int m_Right = 0; //!< X position of the IntRect bottom right corner.
		int m_Bottom = 0; //!< Y position of the IntRect bottom right corner.

		/// Constructor method used to instantiate an IntRect object from four int values defining the initial corners of this IntRect.
		/// @param left X position of the IntRect top left corner.
		/// @param top Y position of the IntRect top left corner.
		/// @param right X position of the IntRect bottom right corner.
		/// @param bottom Y position of the IntRect bottom right corner.
		IntRect(int left, int top, int right, int bottom) :
		    m_Left(left), m_Top(top), m_Right(right), m_Bottom(bottom) {}

		/// Checks whether this IntRect is intersecting another one.
		/// @param rhs The other IntRect to check for intersection with.
		/// @return Whether this IntRect is intersecting another one.
		bool Intersects(const IntRect& rhs) const { return m_Left < rhs.m_Right && m_Right > rhs.m_Left && m_Top < rhs.m_Bottom && m_Bottom > rhs.m_Top; }

		/// If this and the passed in IntRect intersect, this will be modified to represent the boolean AND of the two.
		/// If they don't intersect, nothing happens and false is returned.
		/// @param rhs THe other IntRect to cut against.
		/// @return Whether an intersection was detected and this was cut down to the AND of the two IntRects.
		bool IntersectionCut(const IntRect& rhs);
	};

	/// A useful 2D axis-aligned rectangle class.
	class Box : public Serializable {

	public:
		SerializableClassNameGetter;
		SerializableOverrideMethods;

		Vector m_Corner; //!< Vector position of the upper left corner of this box.
		float m_Width; //!< Width of this box.
		float m_Height; //!< Height of this box.

#pragma region Creation
		/// Constructor method used to instantiate a Box object.
		Box() { Clear(); }

		/// Constructor method used to instantiate a Box object from two points.
		/// @param corner1 Vector position of the upper left corner of this box.
		/// @param corner2 Vector position of the lower right corner of this box.
		Box(const Vector& corner1, const Vector& corner2) { Create(corner1, corner2); }

		/// Constructor method used to instantiate a Box object from four float values defining the initial corners of this Box.
		/// @param x1 X position of box upper left corner.
		/// @param y1 Y position of box upper left corner.
		/// @param x2 X position of box lower right corner.
		/// @param y2 Y position of box lower right corner.
		Box(float x1, float y1, float x2, float y2) { Create(x1, y1, x2, y2); }

		/// Constructor method used to instantiate a Box object from one point and two dimensions.
		/// They can be negative but it will affect the interpretation of which corner is defined. The Box will always return positive values for width and height.
		/// @param corner Vector position of the upper left corner of this box.
		/// @param width Width of this box.
		/// @param height Height of this box.
		Box(const Vector& corner, float width, float height) { Create(corner, width, height); }

		/// Copy constructor method used to instantiate a Box object identical to an already existing one.
		/// @param reference A Box object which is passed in by reference.
		Box(const Box& reference) { Create(reference); }

		/// Makes the Box object ready for use.
		/// @param corner1 Vector position of the upper left corner of this box.
		/// @param corner2 Vector position of the lower right corner of this box.
		/// @return An error return value signaling success or any particular failure. Anything below 0 is an error signal.
		int Create(const Vector& corner1, const Vector& corner2);

		/// Makes the Box object ready for use.
		/// @param x1 X position of box upper left corner.
		/// @param y1 Y position of box upper left corner.
		/// @param x2 X position of box lower right corner.
		/// @param y2 Y position of box lower right corner.
		/// @return An error return value signaling success or any particular failure. Anything below 0 is an error signal.
		int Create(float x1, float y1, float x2, float y2);

		/// Makes the Box object ready for use.
		/// @param corner Vector position of the upper left corner of this box.
		/// @param width Width of this box.
		/// @param height Height of this box.
		/// @return An error return value signaling success or any particular failure. Anything below 0 is an error signal.
		int Create(const Vector& corner, float width, float height);

		/// Creates a Box to be identical to another, by deep copy.
		/// @param reference A reference to the Box to deep copy.
		/// @return An error return value signaling success or any particular failure. Anything below 0 is an error signal.
		int Create(const Box& reference);
#pragma endregion

#pragma region Destruction
		/// Resets the entire Box object to the default settings or values.
		void Reset() override { Clear(); }
#pragma endregion

#pragma region Concrete Methods
		/// Makes the corner of this box represent the upper left corner, and both width and height will end up positive.
		void Unflip();
#pragma endregion

#pragma region Getters and Setters
		/// Shows if this box has 0 width OR 0 height, ie can't contain anything.
		/// @return Whether either width OR height are 0.
		bool IsEmpty() const { return (m_Width == 0 || m_Height == 0); };

		/// Gets the primary corner of this box.
		/// @return A Vector with the primary corner of this box.
		Vector GetCorner() const { return m_Corner; }

		/// Sets the primary corner of this box.
		/// @param newCorner A Vector with the new primary corner.
		void SetCorner(const Vector& newCorner) { m_Corner = newCorner; }

		/// Gets the center point of this Box' area, in absolute Scene coordinates.
		/// @return The center point.
		Vector GetCenter() const { return Vector(m_Corner.m_X + (m_Width / 2), m_Corner.m_Y + (m_Height / 2)); }

		/// Sets the primary corner of this box, by specifying where the center ought to be.
		/// @param newCenter A Vector with the new center point.
		void SetCenter(const Vector& newCenter) { m_Corner.SetXY(newCenter.m_X - (m_Width / 2), newCenter.m_Y - (m_Height / 2)); }

		/// Gets the width of this box. Note that this can be negative if the box hasn't been righted with Unflip().
		/// @return A float value that represents the width value of this Box.
		float GetWidth() const { return m_Width; }

		/// Sets the width of this box. Note that this can be negative if the box hasn't been righted with Unflip().
		/// @param width A float value that represents the width value of this Box.
		void SetWidth(float width) { m_Width = width; }

		/// Gets the height of this box. Note that this can be negative if the box hasn't been righted with Unflip().
		/// @return A float value that represents the height value of this Box.
		float GetHeight() const { return m_Height; }

		/// Sets the height of this box. Note that this can be negative if the box hasn't been righted with Unflip().
		/// @param height A float value that represents the height value of this Box.
		void SetHeight(float height) { m_Height = height; }

		/// Gets the width times the height.
		/// @return The width times the height.
		float GetArea() const { return m_Width * m_Height; }

		/// Gets a random point within this box.
		/// @return The random point within the box.
		Vector GetRandomPoint() const { return Vector(m_Corner.m_X + RandomNum(0.0F, m_Width), m_Corner.m_Y + RandomNum(0.0F, m_Height)); }
#pragma endregion

#pragma region Detection
		/// Tells whether another box intersects this one.
		/// @param rhs The other Box to check for intersection with.
		/// @return Intersecting the other box or not.
		bool IntersectsBox(const Box& rhs);

		/// Tells whether a point is within the Box or not, taking potential flipping into account.
		/// @param point The Vector describing the point to test for within box bounds.
		/// @return Inside the box or not. False if the box IsEmpty()
		bool IsWithinBox(const Vector& point) const;

		/// Tells whether an X coordinate is within the Box's X-range or not, taking potential flipping into account.
		/// @param pointX The coordinate describing the X value to test for within box bounds.
		/// @return Inside the box or not in the X axis. False if the box IsEmpty()
		bool IsWithinBoxX(float pointX) const;

		/// Tells whether an Y coordinate is within the Box's Y-range or not, taking potential flipping into account.
		/// @param pointY The coordinate describing the Y value to test for within box bounds.
		/// @return Inside the box or not in the Y axis. False if the box IsEmpty()
		bool IsWithinBoxY(float pointY) const;

		/// Returns a copy of a point constrained inside this box.
		/// @param point The Vector describing the point to constrain inside the box.
		/// @return The resulting point inside the box.
		Vector GetWithinBox(const Vector& point) const { return Vector(GetWithinBoxX(point.m_X), GetWithinBoxY(point.m_Y)); }

		/// Returns an X value constrained inside the Box and returns it.
		/// @param pointX The X value to constrain inside the Box.
		/// @return The constrained value.
		float GetWithinBoxX(float pointX) const;

		/// Returns an Y value constrained inside the Box and returns it.
		/// @param pointY The Y value to constrain inside the Box.
		/// @return The constrained value.
		float GetWithinBoxY(float pointY) const;
#pragma endregion

#pragma region Operator Overloads
		/// An assignment operator for setting one Box equal to another.
		/// @param rhs A Box reference.
		/// @return A reference to the changed Box.
		Box& operator=(const Box& rhs) {
			if (*this != rhs) {
				Create(rhs);
			}
			return *this;
		}

		/// An equality operator for testing if any two Boxes are equal.
		/// @param lhs A Box reference as the left hand side operand.
		/// @param rhs A Box reference as the right hand side operand.
		/// @return A boolean indicating whether the two operands are equal or not.
		friend bool operator==(const Box& lhs, const Box& rhs) { return lhs.m_Corner == rhs.m_Corner && lhs.m_Width == rhs.m_Width && lhs.m_Height == rhs.m_Height; }

		/// An inequality operator for testing if any two Boxes are unequal.
		/// @param lhs A Box reference as the left hand side operand.
		/// @param rhs A Box reference as the right hand side operand.
		/// @return A boolean indicating whether the two operands are unequal or not.
		friend bool operator!=(const Box& lhs, const Box& rhs) { return lhs.m_Corner != rhs.m_Corner || lhs.m_Width != rhs.m_Width || lhs.m_Height != rhs.m_Height; }
#pragma endregion

	private:
		static const std::string c_ClassName; //!< A string with the friendly-formatted type name of this.

		/// Clears all the member variables of this Box, effectively resetting the members of this abstraction level only.
		void Clear() {
			m_Corner.Reset();
			m_Width = m_Height = 0;
		}
	};
} // namespace RTE
