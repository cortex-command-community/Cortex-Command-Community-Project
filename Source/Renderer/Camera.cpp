#include "Camera.h"
#include "Draw.h"
#include "Constants.h"

using namespace RTE;

bool Camera::IsVisible(Vector center, float radius) {
	return true;
}

void Camera::Enable() {
}

void Camera::Disable() {
}

void Camera::Draw() {
	Rectangle rec;
	rec.x = m_ViewCenter.m_X - m_Viewport.GetWidth() / 2.0f;
	rec.y = m_ViewCenter.m_Y - m_Viewport.GetHeight() / 2.0f;
	rec.width = m_Viewport.GetWidth();
	rec.height = m_Viewport.GetHeight();
	DrawRectangleLinesEx(rec, 1, {53, 0, 0, 255});
}
