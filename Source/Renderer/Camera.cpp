#include "Camera.h"
#include "Draw.h"
#include "Constants.h"
#include "glm/ext/matrix_transform.hpp"
#include "glm/ext/matrix_clip_space.hpp"
#include "glm/gtx/vector_angle.hpp"

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
	m_Viewport.m_Corner = m_ViewCorner.GetFloored();
	m_View = glm::translate(glm::mat4(1.0f), glm::vec3(m_Viewport.m_Width / 2, m_Viewport.m_Height / 2, 0.0f));
	m_View = glm::scale(m_View, glm::vec3(m_Scale, m_Scale, -1.0f));
	m_View = glm::rotate(m_View, glm::angle(static_cast<glm::vec2>(m_ViewUp), glm::vec2(0.0f, 1.0f)), glm::vec3(0.0f, 0.0f, 1.0f));
	m_View = glm::translate(m_View, -glm::vec3(m_Viewport.m_Width / 2, m_Viewport.m_Height / 2, 0.0f));
	m_View = glm::translate(m_View, glm::vec3(static_cast<glm::vec2>(-m_ViewCorner), 0.0f));
	m_Projection = glm::ortho(0.0f, std::floor(m_Viewport.m_Width), 0.0f, std::floor(m_Viewport.m_Height), c_NearDepth, c_FarDepth);
}

void Camera::Draw() const {
	Draw::Lines::Rectangle(m_Viewport, {69, 245, 255, 255});
}
