#pragma once
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

	struct FloatRect {
		FloatRect() = default;
		~FloatRect() = default;
		FloatRect(FloatRect& ref) = default;
		FloatRect(int x, int y, int w, int h) :x(x), y(y), w(w), h(h) {}
		FloatRect(float x, float y, float w, float h) :x(x), y(y), w(w), h(h) {}
		float x{0.0f};
		float y{0.0f};
		float w{0.0f};
		float h{0.0f};
	};
}