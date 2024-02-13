#include "SceneEditor.h"

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
#include "SLBackground.h"

#include "GUI.h"
#include "AllegroBitmap.h"
#include "GUICollectionBox.h"
#include "GUITextBox.h"
#include "GUIButton.h"
#include "GUILabel.h"
#include "GUIComboBox.h"
#include "System.h"

#include "SceneEditorGUI.h"
#include "GameActivity.h"

using namespace RTE;

ConcreteClassInfo(SceneEditor, EditorActivity, 0);

SceneEditor::SceneEditor() {
	Clear();
}

SceneEditor::~SceneEditor() {
	Destroy(true);
}

void SceneEditor::Clear() {
	m_pEditorGUI = 0;
	m_pNewTerrainCombo = 0;
	m_pNewBG1Combo = 0;
	m_pNewBG2Combo = 0;
	m_pNewBG3Combo = 0;
}

int SceneEditor::Create() {
	if (EditorActivity::Create() < 0)
		return -1;

	return 0;
}

int SceneEditor::Create(const SceneEditor& reference) {
	if (EditorActivity::Create(reference) < 0)
		return -1;

	if (m_Description.empty())
		m_Description = "Edit this Scene, including placement of all terrain objects and movable objects, AI blueprints, etc.";

	return 0;
}

int SceneEditor::ReadProperty(const std::string_view& propName, Reader& reader) {
	StartPropertyList(return EditorActivity::ReadProperty(propName, reader));
	/*
	    MatchProperty("CPUTeam", { reader >> m_CPUTeam; });
	    MatchProperty("Difficulty", { reader >> m_Difficulty; });
	    MatchProperty("DeliveryDelay", { reader >> m_DeliveryDelay; });
	*/
	EndPropertyList;
}

int SceneEditor::Save(Writer& writer) const {
	EditorActivity::Save(writer);
	return 0;
}

void SceneEditor::Destroy(bool notInherited) {
	delete m_pEditorGUI;

	if (!notInherited)
		EditorActivity::Destroy();
	Clear();
}

int SceneEditor::Start() {
	int error = EditorActivity::Start();

	//////////////////////////////////////////////
	// Allocate and (re)create the Editor GUI

	if (m_pEditorGUI)
		m_pEditorGUI->Destroy();
	else
		m_pEditorGUI = new SceneEditorGUI;
	m_pEditorGUI->Create(&(m_PlayerController[0]), SceneEditorGUI::ONLOADEDIT);

	//////////////////////////////////////////////////////////////
	// Hooking up directly to the controls defined in the GUI ini

	m_pGUIController->Load("Base.rte/GUIs/SceneEditorGUI.ini");

	// Resize the invisible root container so it matches the screen rez
	GUICollectionBox* pRootBox = dynamic_cast<GUICollectionBox*>(m_pGUIController->GetControl("base"));
	if (pRootBox)
		pRootBox->SetSize(g_WindowMan.GetResX(), g_WindowMan.GetResY());

	// Make sure we have convenient points to the containing GUI dialog boxes that we will manipulate the positions of
	if (!m_pNewDialogBox) {
		m_pNewDialogBox = dynamic_cast<GUICollectionBox*>(m_pGUIController->GetControl("NewDialogBox"));
		//        m_pNewDialogBox->SetDrawType(GUICollectionBox::Color);
		m_pNewDialogBox->SetPositionAbs((g_FrameMan.GetPlayerScreenWidth() / 2) - (m_pNewDialogBox->GetWidth() / 2), (g_FrameMan.GetPlayerScreenHeight() / 2) - (m_pNewDialogBox->GetHeight() / 2));
		m_pNewDialogBox->SetVisible(false);
	}
	m_pNewModuleCombo = dynamic_cast<GUIComboBox*>(m_pGUIController->GetControl("NewModuleCB"));
	if (g_SettingsMan.AllowSavingToBase())
		m_pNewModuleCombo->SetEnabled(true);
	else
		m_pNewModuleCombo->SetEnabled(false);
	m_pNewTerrainCombo = dynamic_cast<GUIComboBox*>(m_pGUIController->GetControl("NewTerrainCB"));
	m_pNewBG1Combo = dynamic_cast<GUIComboBox*>(m_pGUIController->GetControl("NewBG1CB"));
	m_pNewBG2Combo = dynamic_cast<GUIComboBox*>(m_pGUIController->GetControl("NewBG2CB"));
	m_pNewBG3Combo = dynamic_cast<GUIComboBox*>(m_pGUIController->GetControl("NewBG3CB"));
	m_pNewButton = dynamic_cast<GUIButton*>(m_pGUIController->GetControl("NewSceneButton"));
	m_pNewCancel = dynamic_cast<GUIButton*>(m_pGUIController->GetControl("NewCancelButton"));

	// Make sure we have convenient points to the containing GUI dialog boxes that we will manipulate the positions of
	if (!m_pLoadDialogBox) {
		m_pLoadDialogBox = dynamic_cast<GUICollectionBox*>(m_pGUIController->GetControl("LoadDialogBox"));
		//        m_pLoadDialogBox->SetDrawType(GUICollectionBox::Color);
		m_pLoadDialogBox->SetPositionAbs((g_FrameMan.GetPlayerScreenWidth() / 2) - (m_pLoadDialogBox->GetWidth() / 2), (g_FrameMan.GetPlayerScreenHeight() / 2) - (m_pLoadDialogBox->GetHeight() / 2));
		m_pLoadDialogBox->SetVisible(false);
	}
	m_pLoadNameCombo = dynamic_cast<GUIComboBox*>(m_pGUIController->GetControl("LoadSceneCB"));
	m_pLoadNameCombo->SetDropHeight(std::min(m_pLoadNameCombo->GetDropHeight(), g_WindowMan.GetResY() / 2));
	m_pLoadDialogBox->SetSize(m_pLoadDialogBox->GetWidth(), m_pLoadDialogBox->GetHeight() + m_pLoadNameCombo->GetDropHeight()); // Make sure the dropdown can fit, no matter how tall it is.
	m_pLoadToNewButton = dynamic_cast<GUIButton*>(m_pGUIController->GetControl("LoadToNewButton"));
	m_pLoadButton = dynamic_cast<GUIButton*>(m_pGUIController->GetControl("LoadSceneButton"));
	m_pLoadCancel = dynamic_cast<GUIButton*>(m_pGUIController->GetControl("LoadCancelButton"));

	if (!m_pSaveDialogBox) {
		m_pSaveDialogBox = dynamic_cast<GUICollectionBox*>(m_pGUIController->GetControl("SaveDialogBox"));

		// Set the background image of the parent collection box
		//        ContentFile backgroundFile("Base.rte/GUIs/BuyMenuBackground.png");
		//        m_pSaveDialogBox->SetDrawImage(new AllegroBitmap(backgroundFile.GetAsBitmap()));
		//        m_pSaveDialogBox->SetDrawBackground(true);
		//        m_pSaveDialogBox->SetDrawType(GUICollectionBox::Image);
		//        m_pSaveDialogBox->SetDrawType(GUICollectionBox::Color);
		m_pSaveDialogBox->SetPositionAbs((g_FrameMan.GetPlayerScreenWidth() / 2) - (m_pSaveDialogBox->GetWidth() / 2), (g_FrameMan.GetPlayerScreenHeight() / 2) - (m_pSaveDialogBox->GetHeight() / 2));
		m_pSaveDialogBox->SetVisible(false);
	}
	m_pSaveNameBox = dynamic_cast<GUITextBox*>(m_pGUIController->GetControl("SaveSceneNameTB"));
	m_pSaveModuleLabel = dynamic_cast<GUILabel*>(m_pGUIController->GetControl("SaveModuleLabel"));
	m_pSaveButton = dynamic_cast<GUIButton*>(m_pGUIController->GetControl("SaveSceneButton"));
	m_pSaveCancel = dynamic_cast<GUIButton*>(m_pGUIController->GetControl("SaveCancelButton"));

	if (!m_pChangesDialogBox) {
		m_pChangesDialogBox = dynamic_cast<GUICollectionBox*>(m_pGUIController->GetControl("ChangesDialogBox"));
		m_pChangesDialogBox->SetPositionAbs((g_FrameMan.GetPlayerScreenWidth() / 2) - (m_pChangesDialogBox->GetWidth() / 2), (g_FrameMan.GetPlayerScreenHeight() / 2) - (m_pChangesDialogBox->GetHeight() / 2));
		m_pChangesDialogBox->SetVisible(false);
	}
	m_pChangesNameLabel = dynamic_cast<GUILabel*>(m_pGUIController->GetControl("ChangesNameLabel"));
	m_pChangesYesButton = dynamic_cast<GUIButton*>(m_pGUIController->GetControl("ChangesYesButton"));
	m_pChangesNoButton = dynamic_cast<GUIButton*>(m_pGUIController->GetControl("ChangesNoButton"));

	if (!m_pOverwriteDialogBox) {
		m_pOverwriteDialogBox = dynamic_cast<GUICollectionBox*>(m_pGUIController->GetControl("OverwriteDialogBox"));
		m_pOverwriteDialogBox->SetPositionAbs((g_FrameMan.GetPlayerScreenWidth() / 2) - (m_pOverwriteDialogBox->GetWidth() / 2), (g_FrameMan.GetPlayerScreenHeight() / 2) - (m_pOverwriteDialogBox->GetHeight() / 2));
		m_pOverwriteDialogBox->SetVisible(false);
	}
	m_pOverwriteNameLabel = dynamic_cast<GUILabel*>(m_pGUIController->GetControl("OverwriteNameLabel"));
	m_pOverwriteYesButton = dynamic_cast<GUIButton*>(m_pGUIController->GetControl("OverwriteYesButton"));
	m_pOverwriteNoButton = dynamic_cast<GUIButton*>(m_pGUIController->GetControl("OverwriteNoButton"));

	return error;
}

void SceneEditor::SetPaused(bool pause) {
	// Override the pause
	m_Paused = false;
}

void SceneEditor::End() {
	EditorActivity::End();

	m_ActivityState = ActivityState::Over;
}

void SceneEditor::Update() {
	EditorActivity::Update();

	if (!g_SceneMan.GetScene())
		return;

	// Update the loaded objects of the loaded scene so they look right
	g_SceneMan.GetScene()->UpdatePlacedObjects(Scene::PLACEONLOAD);

	// All dialog boxes are gone and we're editing the scene
	if (m_EditorMode == EditorActivity::EDITINGOBJECT) {
		if (m_ModeChange) {
			// Open the picker depending on whetehr there's somehting in the cursor hand or not
			m_pEditorGUI->SetEditorGUIMode(m_pEditorGUI->GetCurrentObject() ? SceneEditorGUI::ADDINGOBJECT : SceneEditorGUI::PICKINGOBJECT);
			// Hide the cursor for this layer of interface
			m_pGUIController->EnableMouse(false);
			m_ModeChange = false;
		}
		g_UInputMan.DisableKeys(false);
	}
	// We are doing something int he dialog boxes, so don't do anything in the editor interface
	else
		m_pEditorGUI->SetEditorGUIMode(SceneEditorGUI::INACTIVE);

	/////////////////////////////////////////////////////
	// Update the editor interface

	m_pEditorGUI->Update();

	// Any edits made, dirtying the scene?
	m_NeedSave = m_pEditorGUI->EditMade() || m_NeedSave;

	// Get any mode change commands that the user gave the Editor GUI
	if (m_pEditorGUI->GetActivatedPieSlice() == PieSliceType::EditorNew && m_EditorMode != NEWDIALOG) {
		m_pEditorGUI->SetEditorGUIMode(SceneEditorGUI::INACTIVE);
		m_EditorMode = EditorActivity::NEWDIALOG;
		m_ModeChange = true;
	} else if (m_pEditorGUI->GetActivatedPieSlice() == PieSliceType::EditorLoad && m_EditorMode != LOADDIALOG) {
		m_pEditorGUI->SetEditorGUIMode(SceneEditorGUI::INACTIVE);
		m_EditorMode = EditorActivity::LOADDIALOG;
		m_ModeChange = true;
	} else if (m_pEditorGUI->GetActivatedPieSlice() == PieSliceType::EditorSave && m_EditorMode != SAVEDIALOG) {
		m_pEditorGUI->SetEditorGUIMode(SceneEditorGUI::INACTIVE);
		m_EditorMode = EditorActivity::SAVEDIALOG;
		m_ModeChange = true;
	}
	// Test the scene by starting a Skirmish Defense with it, after saving
	else if (m_pEditorGUI->GetActivatedPieSlice() == PieSliceType::EditorDone || m_EditorMode == TESTINGOBJECT) {
		m_pEditorGUI->SetEditorGUIMode(SceneEditorGUI::INACTIVE);

		if (m_NeedSave) {
			m_PreviousMode = EditorActivity::TESTINGOBJECT;
			m_EditorMode = EditorActivity::CHANGESDIALOG;
			m_ModeChange = true;
			/*
			            if (m_HasEverBeenSaved)
			                SaveScene(g_SceneMan.GetScene()->GetPresetName());
			            else
			            {
			                m_PreviousMode = TESTINGOBJECT;
			                m_EditorMode = SAVEDIALOG;
			                m_ModeChange = true;
			            }
			*/
		} else {
			g_SceneMan.SetSceneToLoad(g_SceneMan.GetScene()->GetPresetName(), Scene::PLACEONLOAD);

			const Activity* pActivityPreset = dynamic_cast<const Activity*>(g_PresetMan.GetEntityPreset("GAScripted", "Skirmish Defense"));
			Activity* pActivity = dynamic_cast<Activity*>(pActivityPreset->Clone());
			GameActivity* pTestGame = dynamic_cast<GameActivity*>(pActivity);
			RTEAssert(pTestGame, "Couldn't find the \"Skirmish Defense\" GAScripted Activity! Has it been defined?");
			pTestGame->SetTeamOfPlayer(0, 0);
			pTestGame->SetCPUTeam(1);
			pTestGame->SetStartingGold(10000);
			pTestGame->SetFogOfWarEnabled(false);
			pTestGame->SetDifficulty(DifficultySetting::MediumDifficulty);
			g_ActivityMan.SetStartActivity(pTestGame);
			g_ActivityMan.SetRestartActivity();
		}
	}

	////////////////////////////////////////////////////////
	// Handle events for mouse input on the controls

	GUIEvent anEvent;
	while (m_pGUIController->GetEvent(&anEvent)) {
		// If we're not supposed to have mouse control, then ignore these messages
		// Uh this is not right, editor always has mouse control so far
		//        if (!m_PlayerController[0].IsMouseControlled())
		//            break;

		if (anEvent.GetType() == GUIEvent::Command) {
			//////////////////////////////////////////////////////////
			// NEW button pressed; create a new scene

			if (anEvent.GetControl() == m_pNewButton) {
				// Get the selected Module
				GUIListPanel::Item* pItem = m_pNewModuleCombo->GetItem(m_pNewModuleCombo->GetSelectedIndex());
				if (pItem && !pItem->m_Name.empty()) {
					m_ModuleSpaceID = g_PresetMan.GetModuleID(pItem->m_Name);

					// Allocate Scene
					Scene* pNewScene = new Scene();
					// Get the selected Terrain and create the Scene using it
					pItem = m_pNewTerrainCombo->GetItem(m_pNewTerrainCombo->GetSelectedIndex());
					if (pItem && !pItem->m_Name.empty()) {
						SLTerrain* pNewTerrain = dynamic_cast<SLTerrain*>(g_PresetMan.GetEntityPreset("SLTerrain", pItem->m_Name, m_ModuleSpaceID)->Clone());
						RTEAssert(pNewTerrain, "No SLTerrain of that name defined!");
						pNewScene->Create(pNewTerrain);
					}

					// Add specified scene layers
					pItem = m_pNewBG1Combo->GetItem(m_pNewBG1Combo->GetSelectedIndex());
					if (pItem && !pItem->m_Name.empty()) {
						SLBackground* pNewLayer = dynamic_cast<SLBackground*>(g_PresetMan.GetEntityPreset("SLBackground", pItem->m_Name, m_ModuleSpaceID)->Clone());
						RTEAssert(pNewLayer, "No SLBackground of the name set as BG1 is defined!");
						pNewScene->GetBackLayers().push_back(pNewLayer);
					}
					pItem = m_pNewBG2Combo->GetItem(m_pNewBG2Combo->GetSelectedIndex());
					if (pItem && !pItem->m_Name.empty()) {
						SLBackground* pNewLayer = dynamic_cast<SLBackground*>(g_PresetMan.GetEntityPreset("SLBackground", pItem->m_Name, m_ModuleSpaceID)->Clone());
						RTEAssert(pNewLayer, "No SLBackground of the name set as BG2 is defined!");
						pNewScene->GetBackLayers().push_back(pNewLayer);
					}
					pItem = m_pNewBG3Combo->GetItem(m_pNewBG3Combo->GetSelectedIndex());
					if (pItem && !pItem->m_Name.empty()) {
						SLBackground* pNewLayer = dynamic_cast<SLBackground*>(g_PresetMan.GetEntityPreset("SLBackground", pItem->m_Name, m_ModuleSpaceID)->Clone());
						RTEAssert(pNewLayer, "No SLBackground of the name set as BG3 is defined!");
						pNewScene->GetBackLayers().push_back(pNewLayer);
					}

					// Make random planet coord's for this scene
					float angle = RandomNum(0.0F, c_TwoPI);
					Vector pos = Vector((int)(150 * cos(angle)), (int)(150 * sin(angle)));
					pNewScene->SetLocation(pos);

					// Actually load the scene's data and set it up as the current scene
					g_SceneMan.LoadScene(pNewScene, Scene::PLACEONLOAD);

					// Reset the rest of the editor GUI
					m_pEditorGUI->Destroy();
					if (m_ModuleSpaceID == g_PresetMan.GetModuleID(c_UserScenesModuleName))
						m_pEditorGUI->Create(&(m_PlayerController[0]), SceneEditorGUI::ONLOADEDIT, -1);
					else
						m_pEditorGUI->Create(&(m_PlayerController[0]), SceneEditorGUI::ONLOADEDIT, m_ModuleSpaceID);
				}

				m_NeedSave = false;
				m_HasEverBeenSaved = false;
				m_EditorMode = m_PreviousMode = EditorActivity::EDITINGOBJECT;
				m_ModeChange = true;
			}

			//////////////////////////////////////////////////////////
			// LOAD TO NEW button pressed; go from the load to the new dialog

			if (anEvent.GetControl() == m_pLoadToNewButton) {
				m_pEditorGUI->SetEditorGUIMode(SceneEditorGUI::INACTIVE);
				m_EditorMode = EditorActivity::NEWDIALOG;
				m_ModeChange = true;
			}

			//////////////////////////////////////////////////////////
			// LOAD button pressed; load the selected Scene

			if (anEvent.GetControl() == m_pLoadButton) {
				GUIListPanel::Item* pItem = m_pLoadNameCombo->GetItem(m_pLoadNameCombo->GetSelectedIndex());
				if (pItem && !pItem->m_Name.empty()) {
					// Attempt to load the scene, without applying its placed objects
					g_SceneMan.SetSceneToLoad(pItem->m_Name, false);
					g_SceneMan.LoadScene();
					// Get the Module ID that the scene exists in, so we can limit the picker to only show objects from that DataModule space
					if (g_SceneMan.GetScene()) {
						m_ModuleSpaceID = g_SceneMan.GetScene()->GetModuleID();
						RTEAssert(m_ModuleSpaceID >= 0, "Loaded Scene's DataModule ID is negative? Should always be a specific one..");
						m_pEditorGUI->Destroy();
						if (m_ModuleSpaceID == g_PresetMan.GetModuleID(c_UserScenesModuleName))
							m_pEditorGUI->Create(&(m_PlayerController[0]), SceneEditorGUI::ONLOADEDIT, -1);
						else
							m_pEditorGUI->Create(&(m_PlayerController[0]), SceneEditorGUI::ONLOADEDIT, m_ModuleSpaceID);
						// TODO: Should read in all the already placed objects in the loaded scene and have them appear int he editor instead
					}
				}
				m_NeedSave = false;
				m_HasEverBeenSaved = true;
				m_EditorMode = m_PreviousMode = EditorActivity::EDITINGOBJECT;
				m_ModeChange = true;
			}

			//////////////////////////////////////////////////////////
			// SAVE button pressed; save the selected Scene

			if (anEvent.GetControl() == m_pSaveButton) {
				if (!m_pSaveNameBox->GetText().empty()) {
					// Save the scene to the name specified in the text box
					if (SaveScene(m_pSaveNameBox->GetText())) {
						// Close the dialog box on success
						m_NeedSave = false;
						m_HasEverBeenSaved = true;
						// Go back to previous mode after save dialog is done, may have been on the way to test the scene
						m_EditorMode = m_PreviousMode;
						m_ModeChange = true;
					}
					// Should really leave dialog box open?
					else {
						;
					}
				}
			}

			///////////////////////////////////////////////////////////////
			// Save Changes YES pressed

			if (anEvent.GetControl() == m_pChangesYesButton) {
				if (m_HasEverBeenSaved) {
					if (SaveScene(g_SceneMan.GetScene()->GetPresetName(), true)) {
						// Close the dialog box on success
						m_NeedSave = false;
						m_HasEverBeenSaved = true;
						// Go back to previous mode after save dialog is done, may have been on the way to test the scene
						m_EditorMode = m_PreviousMode;
						m_ModeChange = true;
					}
				}
				// Open the save scene dialog to ask user where to save it then
				else {
					m_PreviousMode = m_PreviousMode;
					m_EditorMode = EditorActivity::SAVEDIALOG;
					m_ModeChange = true;
				}
			}

			///////////////////////////////////////////////////////////////
			// Save Changes NO pressed

			if (anEvent.GetControl() == m_pChangesNoButton) {
				// Just go back to previous mode
				m_EditorMode = m_PreviousMode;
				m_ModeChange = true;
				m_NeedSave = false;
			}

			///////////////////////////////////////////////////////////////
			// Overwrite Scene YES pressed

			if (anEvent.GetControl() == m_pOverwriteYesButton) {
				// Force overwrite
				if (SaveScene(g_SceneMan.GetScene()->GetPresetName(), true)) {
					// Close the dialog box on success
					m_NeedSave = false;
					m_HasEverBeenSaved = true;
					// Go back to previous mode after overwrite dialog is done, may have been on the way to test the scene
					m_EditorMode = m_PreviousMode != EditorActivity::SAVEDIALOG ? m_PreviousMode : EditorActivity::EDITINGOBJECT;
					m_ModeChange = true;
				}
				// TODO: Show overwrite error?
			}

			///////////////////////////////////////////////////////////////
			// Overwrite Scene NO pressed

			if (anEvent.GetControl() == m_pOverwriteNoButton) {
				// Just go back to previous mode
				m_EditorMode = m_PreviousMode;
				m_ModeChange = true;
			}

			///////////////////////////////////////////////////////////////
			// CANCEL button pressed; exit any active dialog box

			if (anEvent.GetControl() == m_pNewCancel || anEvent.GetControl() == m_pLoadCancel || anEvent.GetControl() == m_pSaveCancel) {
				// Don't allow canceling out of diags if we're still in the special "Editor Scene", don't allow users to edit it!
				// Just exit the whole editor into the main menu
				if (g_SceneMan.GetScene()->GetPresetName() == "Editor Scene") {
					g_ActivityMan.PauseActivity();
				}
				// Just do normal cancel of the dialog and go back to editing
				else
					m_EditorMode = m_PreviousMode = EditorActivity::EDITINGOBJECT;

				m_ModeChange = true;
			}
		}

		// Notifications
		else if (anEvent.GetType() == GUIEvent::Notification) {
			///////////////////////////////////////
			// Clicks on the New Scene Module combo

			if (anEvent.GetControl() == m_pNewModuleCombo) {
				// Closed it, IE selected somehting
				if (anEvent.GetMsg() == GUIComboBox::Closed)
					UpdateNewDialog();
			}
		}
	}
}

void SceneEditor::DrawGUI(BITMAP* pTargetBitmap, const Vector& targetPos, int which) {
	m_pEditorGUI->Draw(pTargetBitmap, targetPos);

	EditorActivity::DrawGUI(pTargetBitmap, targetPos, which);
}

void SceneEditor::Draw(BITMAP* pTargetBitmap, const Vector& targetPos) {
	EditorActivity::Draw(pTargetBitmap, targetPos);
}

bool SceneEditor::SaveScene(const std::string& saveAsName, bool forceOverwrite) {
	Scene* editedScene = g_SceneMan.GetScene();
	editedScene->SetPresetName(saveAsName);

	std::string dataModuleName = g_PresetMan.GetDataModule(m_ModuleSpaceID)->GetFileName();
	bool savingToUserScenesModule = (dataModuleName == c_UserScenesModuleName);

	std::string dataModuleFullPath = g_PresetMan.GetFullModulePath(dataModuleName);
	std::string sceneSavePath;
	std::string previewSavePath;

	if (savingToUserScenesModule) {
		sceneSavePath = dataModuleFullPath + "/" + saveAsName + ".ini";
		previewSavePath = dataModuleFullPath + "/" + saveAsName + ".preview.png";
	} else {
		sceneSavePath = dataModuleFullPath + "/Scenes/" + saveAsName + ".ini";
		previewSavePath = dataModuleFullPath + "/Scenes/" + saveAsName + ".preview.png";
	}

	if (g_PresetMan.AddEntityPreset(editedScene, m_ModuleSpaceID, forceOverwrite, sceneSavePath)) {
		if (Writer sceneWriter(sceneSavePath, false); !sceneWriter.WriterOK()) {
			RTEError::ShowMessageBox("Failed to create Writer to path:\n\n" + sceneSavePath + "\n\nTHE EDITED SCENE PRESET WAS NOT SAVED!!!");
		} else {
			// TODO: Check if the ini file already exists, and then ask if overwrite.
			sceneWriter.NewPropertyWithValue("AddScene", editedScene);
			sceneWriter.EndWrite();

			editedScene->SavePreview(previewSavePath);
			m_HasEverBeenSaved = true;

			if (!savingToUserScenesModule) {
				// First find/create a Scenes.ini file to include the new .ini into.
				std::string scenesFilePath(dataModuleFullPath + "/Scenes.ini");
				bool scenesFileExists = System::PathExistsCaseSensitive(scenesFilePath);

				if (Writer scenesFileWriter(scenesFilePath, true); !scenesFileWriter.WriterOK()) {
					RTEError::ShowMessageBox("Failed to create Writer to path:\n\n" + scenesFilePath + "\n\nThe edited Scene preset was saved but will not be loaded on next game start!\nPlease include the Scene preset manually!");
				} else {
					scenesFileWriter.NewPropertyWithValue("IncludeFile", sceneSavePath);
					scenesFileWriter.EndWrite();

					// Append to the end of the modules' Index.ini to include the newly created Scenes.ini next startup.
					// If it's somehow already included without actually existing, it doesn't matter, the definitions will just bounce the second time.
					if (!scenesFileExists) {
						std::string indexFilePath = dataModuleFullPath + "/Index.ini";

						if (Writer indexWriter(indexFilePath, true); !indexWriter.WriterOK()) {
							RTEError::ShowMessageBox("Failed to create Writer to path:\n\n" + indexFilePath + "\n\nThe edited Scene preset was saved but will not be loaded on next game start!\nPlease include the Scene preset manually!");
						} else {
							// Add extra tab since the DataModule has everything indented.
							indexWriter.NewProperty("\tIncludeFile");
							indexWriter << scenesFilePath;
							indexWriter.EndWrite();
						}
					}
				}
			}
			return true;
		}
	} else {
		// Got to ask if we can overwrite the existing preset.
		m_PreviousMode = EditorMode::SAVEDIALOG;
		m_EditorMode = EditorMode::OVERWRITEDIALOG;
		m_ModeChange = true;
	}
	return false;
}

void SceneEditor::UpdateNewDialog() {
	int scenesIndex = 0;

	// Only refill modules if empty
	if (m_pNewModuleCombo->GetCount() <= 0) {
		for (int module = 0; module < g_PresetMan.GetTotalModuleCount(); ++module) {
			m_pNewModuleCombo->AddItem(g_PresetMan.GetDataModule(module)->GetFileName());

			if (g_PresetMan.GetDataModule(module)->GetFileName() == c_UserScenesModuleName)
				scenesIndex = m_pNewModuleCombo->GetCount() - 1;
		}

		// Select the user scenes module
		m_pNewModuleCombo->SetSelectedIndex(scenesIndex);
	}

	// Get the ID of the module currently selected so we can limit the following boxes to only show stuff in that module
	int selectedModuleID = -1;
	GUIListPanel::Item* pItem = m_pNewModuleCombo->GetItem(m_pNewModuleCombo->GetSelectedIndex());
	if (pItem && !pItem->m_Name.empty())
		selectedModuleID = g_PresetMan.GetModuleID(pItem->m_Name);

	// Refill Terrains
	m_pNewTerrainCombo->ClearList();
	// Get the list of all read in terrains
	std::list<Entity*> terrainList;
	g_PresetMan.GetAllOfTypeInModuleSpace(terrainList, "SLTerrain", selectedModuleID);
	// Go through the list and add their names to the combo box
	for (std::list<Entity*>::iterator itr = terrainList.begin(); itr != terrainList.end(); ++itr) {
		if ((*itr)->GetPresetName() != "Editor Terrain" &&
		    (*itr)->GetPresetName() != "Physics Test Terrain")
			m_pNewTerrainCombo->AddItem((*itr)->GetPresetName());
	}
	// Select the first one
	m_pNewTerrainCombo->SetSelectedIndex(0);

	// Refill backdrops
	m_pNewBG1Combo->SetText("");
	m_pNewBG2Combo->SetText("");
	m_pNewBG3Combo->SetText("");
	m_pNewBG1Combo->ClearList();
	m_pNewBG2Combo->ClearList();
	m_pNewBG3Combo->ClearList();

	// Get the list of all read in NEAR background layers
	std::list<Entity*> bgList;
	g_PresetMan.GetAllOfGroupInModuleSpace(bgList, "Near Backdrops", "SLBackground", selectedModuleID);
	// Go through the list and add their names to the combo box
	for (std::list<Entity*>::iterator itr = bgList.begin(); itr != bgList.end(); ++itr)
		m_pNewBG1Combo->AddItem((*itr)->GetPresetName());

	// Get the list of all read in MID background layers
	bgList.clear();
	g_PresetMan.GetAllOfGroupInModuleSpace(bgList, "Mid Backdrops", "SLBackground", selectedModuleID);
	// Go through the list and add their names to the combo box
	for (std::list<Entity*>::iterator itr = bgList.begin(); itr != bgList.end(); ++itr)
		m_pNewBG2Combo->AddItem((*itr)->GetPresetName());

	// Get the list of all read in FAR background layers
	bgList.clear();
	g_PresetMan.GetAllOfGroupInModuleSpace(bgList, "Far Backdrops", "SLBackground", selectedModuleID);
	// Go through the list and add their names to the combo box
	for (std::list<Entity*>::iterator itr = bgList.begin(); itr != bgList.end(); ++itr)
		m_pNewBG3Combo->AddItem((*itr)->GetPresetName());

	// Select the first one for each
	m_pNewBG1Combo->SetSelectedIndex(0);
	m_pNewBG2Combo->SetSelectedIndex(0);
	m_pNewBG3Combo->SetSelectedIndex(0);
}

void SceneEditor::UpdateLoadDialog() {
	// Clear out the control
	m_pLoadNameCombo->ClearList();

	// Get the list of all read in scenes
	std::list<Entity*> sceneList;
	g_PresetMan.GetAllOfType(sceneList, "Scene");

	// Go through the list and add their names to the combo box
	for (std::list<Entity*>::iterator itr = sceneList.begin(); itr != sceneList.end(); ++itr) {
		Scene* pScene = dynamic_cast<Scene*>(*itr);
		if (pScene)
			// Don't add the special "Editor Scene" or metascenes, users shouldn't be messing with them
			if (pScene->GetPresetName() != "Editor Scene" && !pScene->IsMetagameInternal() && !pScene->IsSavedGameInternal() && (pScene->GetMetasceneParent() == "" || g_SettingsMan.ShowMetascenes()))
				m_pLoadNameCombo->AddItem(pScene->GetPresetName());
	}

	// Select the first one
	m_pLoadNameCombo->SetSelectedIndex(0);
}

void SceneEditor::UpdateSaveDialog() {
	m_pSaveNameBox->SetText((g_SceneMan.GetScene()->GetPresetName() == "None" || !m_HasEverBeenSaved) ? "New Scene" : g_SceneMan.GetScene()->GetPresetName());

	if (g_PresetMan.GetDataModule(m_ModuleSpaceID)->GetFileName() == c_UserScenesModuleName)
		m_pSaveModuleLabel->SetText("Will save in " + g_PresetMan.GetDataModule(m_ModuleSpaceID)->GetFileName() + "/");
	else
		m_pSaveModuleLabel->SetText("Will save in " + g_PresetMan.GetDataModule(m_ModuleSpaceID)->GetFileName() + "/Scenes");
}

void SceneEditor::UpdateChangesDialog() {
	if (m_HasEverBeenSaved) {
		dynamic_cast<GUILabel*>(m_pGUIController->GetControl("ChangesExpLabel"))->SetText("Do you want to save your changes to:");
		if (g_PresetMan.GetDataModule(m_ModuleSpaceID)->GetFileName() == c_UserScenesModuleName)
			m_pChangesNameLabel->SetText(g_PresetMan.GetDataModule(m_ModuleSpaceID)->GetFileName() + "/" + g_SceneMan.GetScene()->GetPresetName());
		else
			m_pChangesNameLabel->SetText(g_PresetMan.GetDataModule(m_ModuleSpaceID)->GetFileName() + "/Scenes/" + g_SceneMan.GetScene()->GetPresetName());
	} else {
		dynamic_cast<GUILabel*>(m_pGUIController->GetControl("ChangesExpLabel"))->SetText("Save your new Scene first?");
		m_pChangesNameLabel->SetText("");
	}
}

void SceneEditor::UpdateOverwriteDialog() {
	if (g_PresetMan.GetDataModule(m_ModuleSpaceID)->GetFileName() == c_UserScenesModuleName)
		m_pOverwriteNameLabel->SetText(g_PresetMan.GetDataModule(m_ModuleSpaceID)->GetFileName() + "/" + g_SceneMan.GetScene()->GetPresetName());
	else
		m_pOverwriteNameLabel->SetText(g_PresetMan.GetDataModule(m_ModuleSpaceID)->GetFileName() + "/Scenes/" + g_SceneMan.GetScene()->GetPresetName());
}
