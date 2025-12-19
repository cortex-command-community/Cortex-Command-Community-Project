#pragma once
#include "Entity.h"
#include "Box.h"
#include "Vector.h"
#include <vector>

namespace RTE {
	class Camera{
	public:
		Camera(Vector viewCenter, Box viewport, float zoom = 1.0f, Vector viewUp = {0.0f, 1.0f}) :
			m_ViewCenter{viewCenter}, m_Viewport{viewport}, m_Scale{zoom}, m_ViewUp{viewUp} {}

		/// Set the view center of this camera.
		void SetViewCenter(Vector viewCenter) { m_ViewCenter = viewCenter; }

		/// Get the view center of this camera.
		const Vector& GetViewCenter() { return m_ViewCenter; }

		/// Get the viewport of this camera.
		const Box& GetViewport() { return m_Viewport; }

		/// Set the view box of the camera.
		void SetViewport(const Box& viewport) { m_Viewport = viewport; }

		/// Set the post viewport scaling.
		void SetZoom(float zoom) { m_Scale = zoom; }

		/// @brief Test if a circle with \p radius around \p center overlaps with viewport.
		/// This should be used for frustum culling.
		/// @param center Center point of the object to test.
		/// @param radius Radius of the object to test.
		/// @return Whether the object is visible.
		bool IsVisible(Vector center, float radius);

		/// Set up the view matrix so it corresponds to this camera.
		/// @remark Replaces the current view matrix.
		void Enable();

		/// Reset the modelview matrix to identity;
		void Disable();

		void Draw();
	private:
		Vector m_ViewCenter;
		Box m_Viewport;
		float m_Scale;
		Vector m_ViewUp;
	};
}