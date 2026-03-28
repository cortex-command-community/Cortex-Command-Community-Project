#pragma once
#include "Entity.h"
#include "Box.h"
#include "Vector.h"
#include <vector>
#include "glm/glm.hpp"

namespace RTE {
	class Camera{
	public:
		Camera(Vector viewCorner, Box viewport, float zoom = 1.0f, Vector viewUp = {0.0f, 1.0f}) :
			m_ViewCorner{viewCorner}, m_Viewport{viewport}, m_ClipRect{viewport}, m_Scale{zoom}, m_ViewUp{viewUp} { UpdateView(); }

		Camera(const Box& viewBox, Vector viewUp = {0.0f, 1.0f}, float zoom = 1.0f);

		/// Get the corner of this camera view.
		const Vector& GetViewCorner() const { return m_ViewCorner; }

		/// Get the viewport of this camera.
		const Box& GetViewport() const { return m_Viewport; }

		/// Set the view box of the camera.
		void SetViewport(const Box& viewport);

		const glm::mat4& GetView() const { return m_View; }

		const glm::mat4& GetProjection() const { return m_Projection; }

		void SetViewUp(Vector viewUp) { m_ViewUp = viewUp; }

		/// Set the post viewport scaling.
		void SetZoom(float zoom) { m_Scale = zoom; }

		/// @brief Test if a circle with \p radius around \p center overlaps with viewport.
		/// This should be used for frustum culling.
		/// @param center Center point of the object to test.
		/// @param radius Radius of the object to test.
		/// @return Whether the object is visible.
		bool IsVisible(Vector center, float radius) const;

		Activity::Teams GetTeam() const { return m_Team; }

		bool IsShowHUD() const { return m_ShowHUD; }

		void SetClipRect(const Box& clipRect) { m_ClipRect = clipRect; }
		void SetClipState(bool clip) { m_EnableClipping = clip; }

		bool GetClipState() { return m_EnableClipping; }
		const Box& GetClipRect() { return m_ClipRect; }

		void UpdateView();

		void Draw() const;
	private:
		glm::mat4 m_View{1.0f};
		glm::mat4 m_Projection{1.0f};
		Vector m_ViewCorner{0.0f, 0.0f};
		Box m_Viewport{};
		bool m_EnableClipping{false};
		Box m_ClipRect{};
		float m_Scale{1.0f};
		Vector m_ViewUp{};
		Activity::Teams m_Team{Activity::NoTeam};
		bool m_ShowHUD{true};
		bool m_ViewUpdated{true};
	};
}
