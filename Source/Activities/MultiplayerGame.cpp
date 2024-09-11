#include "MultiplayerGame.h"

#include "WindowMan.h"
#include "PresetMan.h"
#include "MovableMan.h"
#include "FrameMan.h"
#include "UInputMan.h"
#include "SettingsMan.h"
#include "SLTerrain.h"
#include "Controller.h"
#include "Actor.h"
#include "AHuman.h"
#include "ACRocket.h"
#include "HeldDevice.h"
#include "Scene.h"
#include "DataModule.h"
#include "AudioMan.h"

#include "GUI.h"
#include "GUIFont.h"
#include "AllegroScreen.h"
#include "GUIInputWrapper.h"
#include "GUIControlManager.h"
#include "GUICollectionBox.h"
#include "GUITextBox.h"
#include "GUIButton.h"
#include "GUILabel.h"

#include "MultiplayerGameGUI.h"

#include "NetworkClient.h"

using namespace RTE;

ConcreteClassInfo(MultiplayerGame, Activity, 0);

MultiplayerGame::MultiplayerGame() {
	Clear();
}

MultiplayerGame::~MultiplayerGame() {
	Destroy(true);
}

void MultiplayerGame::Clear() {
	m_pGUIController = 0;
	m_pGUIInput = 0;
	m_pGUIScreen = 0;

	m_pServerNameTextBox = 0;
	m_pPlayerNameTextBox = 0;
	m_pConnectButton = 0;

	m_pNATServiceServerNameTextBox = 0;
	m_pNATServerNameTextBox = 0;
	m_pNATServerPasswordTextBox = 0;
	m_pConnectNATButton = 0;

	m_pStatusLabel = 0;

	m_BackToMainButton = nullptr;

	m_Mode = SETUP;
}

int MultiplayerGame::Create() {
	if (Activity::Create() < 0)
		return -1;
	return 0;
}

int MultiplayerGame::Create(const MultiplayerGame& reference) {
	if (Activity::Create(reference) < 0)
		return -1;

	if (m_Description.empty())
		m_Description = "Edit this Scene, including placement of all terrain objects and movable objects, AI blueprints, etc.";

	return 0;
}

int MultiplayerGame::ReadProperty(const std::string_view& propName, Reader& reader) {
	return Activity::ReadProperty(propName, reader);
}

int MultiplayerGame::Save(Writer& writer) const {
	Activity::Save(writer);
	return 0;
}

void MultiplayerGame::Destroy(bool notInherited) {
	g_FrameMan.SetDrawNetworkBackBuffer(false);
	g_NetworkClient.Disconnect();

	delete m_pGUIController;
	delete m_pGUIInput;
	delete m_pGUIScreen;

	if (!notInherited)
		Activity::Destroy();
	Clear();
}

int MultiplayerGame::Start() {
	int error = Activity::Start();

	if (!m_pGUIScreen)
		m_pGUIScreen = new AllegroScreen(g_FrameMan.GetBackBuffer8());
	if (!m_pGUIInput)
		m_pGUIInput = new GUIInputWrapper(-1, true);
	if (!m_pGUIController)
		m_pGUIController = new GUIControlManager();
	if (!m_pGUIController->Create(m_pGUIScreen, m_pGUIInput, "Base.rte/GUIs/Skins", "DefaultSkin.ini")) {
		RTEAbort("Failed to create GUI Control Manager and load it from Base.rte/GUIs/Skins/DefaultSkin.ini");
	}

	m_pGUIController->Load("Base.rte/GUIs/MultiplayerGameGUI.ini");
	m_pGUIController->EnableMouse(true);

	// Resize the invisible root container so it matches the screen rez
	GUICollectionBox* pRootBox = dynamic_cast<GUICollectionBox*>(m_pGUIController->GetControl("base"));
	if (pRootBox)
		pRootBox->SetSize(g_WindowMan.GetResX(), g_WindowMan.GetResY());

	m_BackToMainButton = dynamic_cast<GUIButton*>(m_pGUIController->GetControl("ButtonBackToMain"));

	GUICollectionBox* pDialogBox = dynamic_cast<GUICollectionBox*>(m_pGUIController->GetControl("ConnectDialogBox"));
	if (pDialogBox) {
		pDialogBox->SetPositionAbs(g_WindowMan.GetResX() / 2 - pDialogBox->GetWidth() / 2, g_WindowMan.GetResY() / 2 - pDialogBox->GetHeight() / 2);
		m_BackToMainButton->SetPositionAbs((g_WindowMan.GetResX() - m_BackToMainButton->GetWidth()) / 2, pDialogBox->GetYPos() + pDialogBox->GetHeight() + 10);
	}

	m_pServerNameTextBox = dynamic_cast<GUITextBox*>(m_pGUIController->GetControl("ServerNameTB"));
	m_pPlayerNameTextBox = dynamic_cast<GUITextBox*>(m_pGUIController->GetControl("PlayerNameTB"));
	m_pConnectButton = dynamic_cast<GUIButton*>(m_pGUIController->GetControl("ConnectButton"));

	/*
	m_pNATServiceServerNameTextBox = dynamic_cast<GUITextBox *>(m_pGUIController->GetControl("NATServiceNameTB"));
	m_pNATServerNameTextBox = dynamic_cast<GUITextBox *>(m_pGUIController->GetControl("NATServiceServerNameTB"));
	m_pNATServerPasswordTextBox = dynamic_cast<GUITextBox *>(m_pGUIController->GetControl("NATServiceServerPasswordTB"));
	m_pConnectNATButton = dynamic_cast<GUIButton *>(m_pGUIController->GetControl("ConnectNATButton"));

	m_pNATServiceServerNameTextBox->SetVisible(false);
	m_pNATServerNameTextBox->SetVisible(false);
	m_pNATServerPasswordTextBox->SetVisible(false);
	m_pConnectNATButton->SetVisible(false);
	*/

	// NOTE Instruction labels aren't dynamic so they don't really need to be gotten. Status label should be empty unless there's a status to report.
	m_pStatusLabel = dynamic_cast<GUILabel*>(m_pGUIController->GetControl("StatusLabel"));
	m_pStatusLabel->SetText("");

	m_pServerNameTextBox->SetText(g_SettingsMan.GetNetworkServerAddress());
	m_pPlayerNameTextBox->SetText(g_SettingsMan.GetPlayerNetworkName());

	/*
	m_pNATServiceServerNameTextBox->SetText(g_SettingsMan.GetNATServiceAddress());
	m_pNATServerNameTextBox->SetText(g_SettingsMan.GetNATServerName());
	m_pNATServerPasswordTextBox->SetText(g_SettingsMan.GetNATServerPassword());
	*/

	return error;
}

void MultiplayerGame::SetPaused(bool pause) {
	// Override the pause
	// m_Paused = false;
	Activity::SetPaused(pause);

	if (!pause) {
		if (m_Mode == GAMEPLAY) {
			g_UInputMan.TrapMousePos(true, 0);
		}
	}
}

void MultiplayerGame::End() {
	Activity::End();

	m_ActivityState = ActivityState::Over;
	g_FrameMan.SetDrawNetworkBackBuffer(false);
}

void MultiplayerGame::Update() {
	Activity::Update();

	/////////////////////////////////////////////////////
	// Update the editor interface
	if (m_Mode == SETUP) {
		m_pGUIController->Update();

		////////////////////////////////////////////////////////
		// Handle events for mouse input on the controls

		bool toExit = false;

		GUIEvent anEvent;
		while (m_pGUIController->GetEvent(&anEvent)) {
			if (anEvent.GetType() == GUIEvent::Command) {
				if (anEvent.GetControl() == m_BackToMainButton) {
					g_ActivityMan.PauseActivity();
					return;
				}

				if (anEvent.GetControl() == m_pConnectButton) {
					std::string serverName;
					int port;

					std::string::size_type portPos = std::string::npos;

					portPos = m_pServerNameTextBox->GetText().find(":");
					if (portPos != std::string::npos) {
						serverName = m_pServerNameTextBox->GetText().substr(0, portPos);
						std::string portStr = m_pServerNameTextBox->GetText().substr(portPos + 1, m_pServerNameTextBox->GetText().length() - 2);

						port = atoi(portStr.c_str());
						if (port == 0)
							port = 8000;
					} else {
						serverName = m_pServerNameTextBox->GetText();
						port = 8000;
					}

					std::string playerName = m_pPlayerNameTextBox->GetText();
					if (playerName == "")
						playerName = "Unnamed Player";

					g_NetworkClient.Connect(serverName, port, playerName);
					bool saveSettings = false;

					if (g_SettingsMan.GetPlayerNetworkName() != m_pPlayerNameTextBox->GetText()) {
						g_SettingsMan.SetPlayerNetworkName(m_pPlayerNameTextBox->GetText());
						saveSettings = true;
					}

					if (g_SettingsMan.GetNetworkServerAddress() != m_pServerNameTextBox->GetText()) {
						g_SettingsMan.SetNetworkServerAddress(m_pServerNameTextBox->GetText());
						saveSettings = true;
					}

					if (saveSettings)
						g_SettingsMan.UpdateSettingsFile();

					m_pGUIController->EnableMouse(false);
					m_Mode = CONNECTION;
					m_ConnectionWaitTimer.Reset();
					g_GUISound.ButtonPressSound()->Play();
				}

				if (anEvent.GetControl() == m_pConnectNATButton) {
					std::string serverName;
					int port;

					std::string::size_type portPos = std::string::npos;

					portPos = m_pNATServiceServerNameTextBox->GetText().find(":");
					if (portPos != std::string::npos) {
						serverName = m_pNATServiceServerNameTextBox->GetText().substr(0, portPos);
						std::string portStr = m_pNATServiceServerNameTextBox->GetText().substr(portPos + 1, m_pNATServiceServerNameTextBox->GetText().length() - 2);

						port = atoi(portStr.c_str());
						if (port == 0)
							port = 61111;
					} else {
						serverName = m_pNATServiceServerNameTextBox->GetText();
						port = 61111;
					}

					std::string playerName = m_pPlayerNameTextBox->GetText();
					if (playerName == "")
						playerName = "Unnamed Player";

					g_NetworkClient.PerformNATPunchThrough(serverName, port, playerName, m_pNATServerNameTextBox->GetText(), m_pNATServerPasswordTextBox->GetText());
					bool saveSettings = false;

					if (g_SettingsMan.GetPlayerNetworkName() != m_pPlayerNameTextBox->GetText()) {
						g_SettingsMan.SetPlayerNetworkName(m_pPlayerNameTextBox->GetText());
						saveSettings = true;
					}

					if (g_SettingsMan.GetNATServiceAddress() != m_pNATServiceServerNameTextBox->GetText()) {
						g_SettingsMan.SetNATServiceAddress(m_pNATServiceServerNameTextBox->GetText());
						saveSettings = true;
					}

					if (g_SettingsMan.GetNATServerName() != m_pNATServerNameTextBox->GetText()) {
						g_SettingsMan.SetNATServerName(m_pNATServerNameTextBox->GetText());
						saveSettings = true;
					}

					if (g_SettingsMan.GetNATServerPassword() != m_pNATServerPasswordTextBox->GetText()) {
						g_SettingsMan.SetNATServerPassword(m_pNATServerPasswordTextBox->GetText());
						saveSettings = true;
					}

					if (saveSettings)
						g_SettingsMan.UpdateSettingsFile();

					m_pGUIController->EnableMouse(false);
					m_Mode = CONNECTION;
					m_ConnectionWaitTimer.Reset();
					g_GUISound.ButtonPressSound()->Play();
				}
			}
			// Notifications
			else if (anEvent.GetType() == GUIEvent::Notification) {
			}
		}
	}

	if (m_Mode == CONNECTION) {
		if (g_NetworkClient.IsConnectedAndRegistered())
			m_Mode = GAMEPLAY;

		if (m_ConnectionWaitTimer.IsPastRealMS(8000)) {
			g_NetworkClient.Disconnect();
			m_Mode = SETUP;
			m_pStatusLabel->SetText("Connection failed. Check console for error messages.");
			m_pGUIController->EnableMouse(true);
		}
	}

	if (m_Mode == GAMEPLAY) {
		g_UInputMan.TrapMousePos(true, 0);
		g_FrameMan.SetDrawNetworkBackBuffer(true);
		m_pGUIController->EnableMouse(false);

		if (!g_NetworkClient.IsConnectedAndRegistered()) {
			// g_ActivityMan.EndActivity();
			// g_ActivityMan.SetRestartActivity();
			m_Mode = SETUP;
			m_pGUIController->EnableMouse(true);
			g_UInputMan.TrapMousePos(false, 0);
			g_FrameMan.SetDrawNetworkBackBuffer(false);
		}
	}

	/*if (g_UInputMan.ElementHeld(0, UInputMan::INPUT_FIRE))
	    g_FrameMan.SetScreenText("FIRE", 0, 0, -1, false);
	else
	    g_FrameMan.SetScreenText("-", 0, 0, -1, false);*/

	g_NetworkClient.Update();
}

void MultiplayerGame::DrawGUI(BITMAP* pTargetBitmap, const Vector& targetPos, int which) {
	if (m_pGUIController) {
		AllegroScreen drawScreen(pTargetBitmap);
		m_pGUIController->Draw(&drawScreen);
		if (m_Mode == SETUP)
			m_pGUIController->DrawMouse();
	}
}

void MultiplayerGame::Draw(BITMAP* pTargetBitmap, const Vector& targetPos) {
	Activity::Draw(pTargetBitmap, targetPos);
}
