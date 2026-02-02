#include "DebugMan.h"
#include "imgui/imgui.h"

using namespace RTE;

void Draw() {
}

void DebugMan::DrawImGui() {
	DebugOptionsGUI();
}

void DebugMan::DebugOptionsGUI() {
	if (ImGui::Begin("Debug Options", &m_ShowDebugWindow)) {
		if (ImGui::TreeNode("Debug Draw")) {
			ImGui::Checkbox("Draw Camera bounds", &m_DrawCameraBounds);
			if (ImGui::TreeNode("")) {

			}
		}
	}
	ImGui::End();
}
