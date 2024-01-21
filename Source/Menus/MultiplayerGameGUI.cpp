#include "MultiplayerGameGUI.h"

#include "FrameMan.h"

using namespace RTE;

MultiplayerGameGUI::MultiplayerGameGUI() {
	Clear();
}

MultiplayerGameGUI::~MultiplayerGameGUI() {
	Destroy();
}

void MultiplayerGameGUI::Clear() {
	m_pController = 0;
}

int MultiplayerGameGUI::Create(Controller* pController) {
	AAssert(pController, "No controller sent to MultiplayerGameGUI on creation!");
	m_pController = pController;

	// Allocate and (re)create the Editor GUIs
	// m_pPieMenu = new PieMenuGUI();
	// m_pPieMenu->Create(pController);

	return 0;
}

void MultiplayerGameGUI::Destroy() {
	Clear();
}

void MultiplayerGameGUI::Update() {
}

void MultiplayerGameGUI::Draw(BITMAP* pTargetBitmap, const Vector& targetPos) const {
}
