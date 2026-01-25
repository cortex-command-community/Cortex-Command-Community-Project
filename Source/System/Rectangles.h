#pragma once
namespace RTE {
	/// A simple rectangle from 2 points.
	template <typename T>
	struct Rectangle2Point {
		Rectangle2Point(T left, T top, T right, T bottom) : m_Left(left), m_Top(top), m_Right(right), m_Bottom(bottom) {}

		bool Intersects(const Rectangle2Point& rhs) const { return m_Left < rhs.m_Right && m_Right > rhs.m_Left && m_Top < rhs.m_Bottom && m_Bottom > rhs.m_Top; }

		bool IntersectionCut(const Rectangle2Point& rhs);

		T m_Left{};
		T m_Top{};
		T m_Right{};
		T m_Bottom{};
	};
	using IntRect = Rectangle2Point<int>;

	template <typename T>
	struct RectangleWidthHeight {
		RectangleWidthHeight(T x, T y, T width, T height) :x(x), y(y), w(width), h(height) {}
		T x{0.0f};
		T y{0.0f};
		T w{0.0f};
		T h{0.0f};
	};

	using FloatRect = RectangleWidthHeight<float>;
}
