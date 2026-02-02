#pragma once
#include "Singleton.h"
#include <memory>

#define g_DebugMan DebugMan::Instance()

namespace RTE {
	class DebugMan : public Singleton<DebugMan> {
	public:
		void Draw();
		void DrawImGui();

		bool DrawSpriteBounds() { return m_DrawSpriteBounds; }
		constexpr bool DrawNoGravBoxes() { return false; }
		bool DrawBigTextureBounds() { return false; }

	private:
		bool m_ShowDebugWindow{false};

		bool m_DrawCameraBounds{false};
		bool m_DrawSpriteBounds{false};

		void DebugOptionsGUI();
	};
} // namespace RTE
