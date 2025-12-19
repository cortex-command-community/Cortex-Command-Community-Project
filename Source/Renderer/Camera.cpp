#include "Camera.h"
#include "Draw.h"
#include "Constants.h"

using namespace RTE;

bool Camera::IsVisible(Vector center, float radius) {
	return true;
}

void Camera::Enable() {
	rlMatrixMode(RL_MODELVIEW);
	rlLoadIdentity();
	rlTranslatef(m_Viewport.GetWidth() / 2, m_Viewport.GetHeight() / 2, 0.0f);
	rlScalef(m_Scale, m_Scale, 1.0f);
	rlRotatef(m_ViewUp.GetAbsDegAngle() + 90, 0.0f, 0.0f, 1.0f);
	rlTranslatef(-m_Viewport.GetWidth() / 2, -m_Viewport.GetHeight() / 2, 0.0f);
}

void Camera::Disable() {
	rlMatrixMode(RL_MODELVIEW);
	rlLoadIdentity();
}

void Camera::Draw() {
	Rectangle rec;
	rec.x = m_ViewCenter.m_X - m_Viewport.GetWidth() / 2.0f;
	rec.y = m_ViewCenter.m_Y - m_Viewport.GetHeight() / 2.0f;
	rec.width = m_Viewport.GetWidth();
	rec.height = m_Viewport.GetHeight();
	DrawRectangleLinesEx(rec, 1, {53, 0, 0, 255});
}