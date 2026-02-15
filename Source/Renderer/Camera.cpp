#include "Camera.h"
#include "Draw.h"
#include "Constants.h"
#include "glm/ext/matrix_transform.hpp"
#include "glm/ext/matrix_clip_space.hpp"

using namespace RTE;

Camera::Camera(const Box& viewBox, Vector viewUp, float zoom) {

}

bool Camera::IsVisible(Vector center, float radius) const {
	// https://yal.cc/rectangle-circle-intersection-test/
	float deltaX = center.m_X - std::max(m_Viewport.m_Corner.m_X, std::min(center.m_X, m_Viewport.m_Corner.m_X + m_Viewport.m_Width));
	float deltaY = center.m_Y - std::max(m_Viewport.m_Corner.m_Y, std::min(center.m_Y, m_Viewport.m_Corner.m_Y + m_Viewport.m_Height));
	return (deltaX * deltaX + deltaY * deltaY) < (radius * radius);
}

void Camera::SetViewport(const Box& viewport) {
	m_Viewport = viewport;
}

void Camera::UpdateView() {
	m_Viewport.m_Corner = (m_ViewCenter - Vector(m_Viewport.m_Width / 2, m_Viewport.m_Height / 2).GetFloored()).GetFloored();
	m_View = glm::lookAt(glm::vec3(static_cast<glm::vec2>(m_ViewCenter), 0.0f), glm::vec3(static_cast<glm::vec2>(m_ViewCenter), c_NearDepth), glm::vec3(static_cast<glm::vec2>(m_ViewUp), 0.0f));
	m_Projection = glm::ortho(0.0f, std::floor(m_Viewport.m_Width), 0.0f, std::floor(m_Viewport.m_Height), c_NearDepth, c_FarDepth);
}

void Camera::Draw() {
	Box rec;
	rec.m_Corner.m_X = m_ViewCenter.m_X - m_Viewport.GetWidth() / 2.0f;
	rec.m_Corner.m_Y = m_ViewCenter.m_Y - m_Viewport.GetHeight() / 2.0f;
	rec.m_Width = m_Viewport.GetWidth();
	rec.m_Height = m_Viewport.GetHeight();
	Draw::RectangleLines(rec, {53, 0, 0, 255});
}
