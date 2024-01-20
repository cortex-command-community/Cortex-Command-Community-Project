//////////////////////////////////////////////////////////////////////////////////////////
// File:            BaseEditor.cpp
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Source file for the BaseEditor class.
// Project:         Retro Terrain Engine
// Author(s):       Daniel Tabar
//                  data@datarealms.com
//                  http://www.datarealms.com

//////////////////////////////////////////////////////////////////////////////////////////
// Inclusions of header files

#include "BaseEditor.h"

#include "CameraMan.h"
#include "PresetMan.h"
#include "MovableMan.h"
#include "FrameMan.h"
#include "UInputMan.h"
#include "SceneMan.h"
#include "MetaMan.h"
//#include "AHuman.h"
//#include "MOPixel.h"
#include "SLTerrain.h"
#include "Controller.h"
//#include "AtomGroup.h"
#include "Actor.h"
#include "AHuman.h"
#include "ACRocket.h"
#include "HeldDevice.h"
#include "Scene.h"
#include "DataModule.h"

#include "SceneEditorGUI.h"

namespace RTE {

	ConcreteClassInfo(BaseEditor, Activity, 0);

	//////////////////////////////////////////////////////////////////////////////////////////
	// Method:          Clear
	//////////////////////////////////////////////////////////////////////////////////////////
	// Description:     Clears all the member variables of this BaseEditor, effectively
	//                  resetting the members of this abstraction level only.

	void BaseEditor::Clear() {
		m_pEditorGUI = 0;
		m_NeedSave = false;
	}

	//////////////////////////////////////////////////////////////////////////////////////////
	// Virtual method:  Create
	//////////////////////////////////////////////////////////////////////////////////////////
	// Description:     Makes the BaseEditor object ready for use.

	int BaseEditor::Create() {
		if (Activity::Create() < 0)
			return -1;

		return 0;
	}

	//////////////////////////////////////////////////////////////////////////////////////////
	// Virtual method:  Create
	//////////////////////////////////////////////////////////////////////////////////////////
	// Description:     Creates a BaseEditor to be identical to another, by deep copy.

	int BaseEditor::Create(const BaseEditor& reference) {
		if (Activity::Create(reference) < 0)
			return -1;

		if (m_Description.empty())
			m_Description = "Build or Edit a base on this Scene.";

		return 0;
	}

	//////////////////////////////////////////////////////////////////////////////////////////
	// Virtual method:  ReadProperty
	//////////////////////////////////////////////////////////////////////////////////////////
	// Description:     Reads a property value from a reader stream. If the name isn't
	//                  recognized by this class, then ReadProperty of the parent class
	//                  is called. If the property isn't recognized by any of the base classes,
	//                  false is returned, and the reader's position is untouched.

	int BaseEditor::ReadProperty(const std::string_view& propName, Reader& reader) {
		StartPropertyList(return Activity::ReadProperty(propName, reader));
		/*
		    MatchProperty("CPUTeam", { reader >> m_CPUTeam; });
		    MatchProperty("Difficulty", { reader >> m_Difficulty; });
		    MatchProperty("DeliveryDelay", { reader >> m_DeliveryDelay; });
		*/
		EndPropertyList;
	}

	//////////////////////////////////////////////////////////////////////////////////////////
	// Virtual method:  Save
	//////////////////////////////////////////////////////////////////////////////////////////
	// Description:     Saves the complete state of this BaseEditor with a Writer for
	//                  later recreation with Create(Reader &reader);

	int BaseEditor::Save(Writer& writer) const {
		Activity::Save(writer);
		return 0;
	}

	//////////////////////////////////////////////////////////////////////////////////////////
	// Method:          Destroy
	//////////////////////////////////////////////////////////////////////////////////////////
	// Description:     Destroys and resets (through Clear()) the BaseEditor object.

	void BaseEditor::Destroy(bool notInherited) {
		delete m_pEditorGUI;

		if (!notInherited)
			Activity::Destroy();
		Clear();
	}

	//////////////////////////////////////////////////////////////////////////////////////////
	// Virtual method:  Start
	//////////////////////////////////////////////////////////////////////////////////////////
	// Description:     Officially starts this. Creates all the data etc necessary to start
	//                  the activity.

	int BaseEditor::Start() {
		// Set the split screen config before the Scene (and it SceneLayers, specifially) are loaded
		g_FrameMan.ResetSplitScreens(false, false);
		// Too diff
		//    int error = Activity::Start();
		int error = 0;

		m_ActivityState = ActivityState::Running;
		m_Paused = false;

		// Reset the mousemoving so that it won't trap the mouse if the window isn't in focus (common after loading)
		g_UInputMan.DisableMouseMoving(true);
		g_UInputMan.DisableMouseMoving(false);

		// Enable keys again
		g_UInputMan.DisableKeys(false);

		// Load the scene now
		error = g_SceneMan.LoadScene();
		if (error < 0)
			return error;

		// Open all doors so we can pathfind past them for brain placement
		g_MovableMan.OpenAllDoors(true, Teams::NoTeam);

		///////////////////////////////////////
		// Set up player - ONLY ONE ever in a base building activity

		// Figure which player is editing this base.. the first active one we find
		int editingPlayer = Players::PlayerOne;
		for (int player = Players::PlayerOne; player < Players::MaxPlayerCount; ++player)
			if (m_IsActive[player])
				editingPlayer = player;
		// TODO: support multiple coop players editing the same base?? - A: NO, silly

		//    for (int player = Players::PlayerOne; player < Players::MaxPlayerCount; ++player)
		//    {
		//        if (!m_IsActive[player])
		//            continue;
		m_ViewState[editingPlayer] = ViewState::Normal;
		g_FrameMan.ClearScreenText(ScreenOfPlayer(editingPlayer));
		// Set the team associations with the first screen so that the correct unseen are shows up
		g_CameraMan.SetScreenTeam(m_Team[editingPlayer], ScreenOfPlayer(editingPlayer));
		g_CameraMan.SetScreenOcclusion(Vector(), ScreenOfPlayer(editingPlayer));

		m_PlayerController[editingPlayer].Reset();
		m_PlayerController[editingPlayer].Create(Controller::CIM_PLAYER, editingPlayer);
		m_PlayerController[editingPlayer].SetTeam(m_Team[editingPlayer]);

		m_MessageTimer[editingPlayer].Reset();
		//    }

		// Kill off any actors not of this player's team.. they're not supposed to be here
		g_MovableMan.KillAllEnemyActors(GetTeamOfPlayer(editingPlayer));

		//////////////////////////////////////////////
		// Allocate and (re)create the Editor GUI

		if (m_pEditorGUI)
			m_pEditorGUI->Destroy();
		else
			m_pEditorGUI = new SceneEditorGUI;
		m_pEditorGUI->Create(&(m_PlayerController[editingPlayer]), SceneEditorGUI::BLUEPRINTEDIT);

		// See if we are playing a metagame and which metaplayer this would be editing in this activity
		if (g_MetaMan.GameInProgress()) {
			MetaPlayer* pMetaPlayer = g_MetaMan.GetMetaPlayerOfInGamePlayer(editingPlayer);
			if (pMetaPlayer) {
				// Set the appropriate modifiers to the prices etc of this editor
				m_pEditorGUI->SetNativeTechModule(pMetaPlayer->GetNativeTechModule());
				m_pEditorGUI->SetForeignCostMultiplier(pMetaPlayer->GetForeignCostMultiplier());
			}
		}

		// Set the view to scroll to the brain of the editing player, if there is any
		if (g_SceneMan.GetScene()->GetResidentBrain(editingPlayer))
			m_pEditorGUI->SetCursorPos(g_SceneMan.GetScene()->GetResidentBrain(editingPlayer)->GetPos());

		// Test if the resident brain is still in a valid spot, after potentially building it into a tomb since last round
		m_pEditorGUI->TestBrainResidence();

		////////////////////////////////
		// Set up teams

		for (int team = Teams::TeamOne; team < Teams::MaxTeamCount; ++team) {
			if (!m_TeamActive[team])
				continue;
			m_FundsChanged[team] = false;
			m_TeamDeaths[team] = 0;
		}

		// Move any brains resident in the Scene to the MovableMan
		// Nope - these are manipulated by the SceneEditorGUI directly where they are in the resident lists
		//    g_SceneMan.GetScene()->PlaceResidentBrains(*this);

		// The get a list of all the placed objects in the Scene and set them to not kick around
		const std::list<SceneObject*>* pSceneObjectList = g_SceneMan.GetScene()->GetPlacedObjects(Scene::BLUEPRINT);
		for (std::list<SceneObject*>::const_iterator itr = pSceneObjectList->begin(); itr != pSceneObjectList->end(); ++itr) {
			Actor* pActor = dynamic_cast<Actor*>(*itr);
			if (pActor) {
				pActor->SetStatus(Actor::INACTIVE);
				pActor->GetController()->SetDisabled(true);
			}
		}

		// Update all blueprints so they look right after load
		g_SceneMan.GetScene()->UpdatePlacedObjects(Scene::BLUEPRINT);

		return error;
	}

	//////////////////////////////////////////////////////////////////////////////////////////
	// Method:          Pause
	//////////////////////////////////////////////////////////////////////////////////////////
	// Description:     Pauses and unpauses the game.

	void BaseEditor::SetPaused(bool pause) {
		// Override the pause
		m_Paused = false;
	}

	//////////////////////////////////////////////////////////////////////////////////////////
	// Method:          End
	//////////////////////////////////////////////////////////////////////////////////////////
	// Description:     Forces the current game's end.

	void BaseEditor::End() {
		Activity::End();

		m_ActivityState = ActivityState::Over;
	}

	//////////////////////////////////////////////////////////////////////////////////////////
	// Method:          Update
	//////////////////////////////////////////////////////////////////////////////////////////
	// Description:     Updates the state of this BaseEditor. Supposed to be done every frame
	//                  before drawing.

	void BaseEditor::Update() {
		Activity::Update();

		if (!g_SceneMan.GetScene())
			return;

		// Update the loaded objects of the loaded scene so they look right
		g_SceneMan.GetScene()->UpdatePlacedObjects(Scene::BLUEPRINT);

		/////////////////////////////////////////////////////
		// Update the editor interface

		m_pEditorGUI->Update();

		// Any edits made, dirtying the scene?
		m_NeedSave = m_pEditorGUI->EditMade() || m_NeedSave;

		// Get any mode change commands that the user gave the Editor GUI
		// Done with editing for now; save and return to campaign screen
		if (m_pEditorGUI->GetActivatedPieSlice() == PieSlice::SliceType::EditorDone) {
			m_pEditorGUI->SetEditorGUIMode(SceneEditorGUI::INACTIVE);

			if (m_NeedSave)
				SaveScene(g_SceneMan.GetScene()->GetPresetName());

			// Quit to metagame view
			g_ActivityMan.PauseActivity();
		}
	}

	//////////////////////////////////////////////////////////////////////////////////////////
	// Method:          DrawGUI
	//////////////////////////////////////////////////////////////////////////////////////////
	// Description:     Draws the currently active GUI of a screen to a BITMAP of choice.

	void BaseEditor::DrawGUI(BITMAP* pTargetBitmap, const Vector& targetPos, int which) {
		m_pEditorGUI->Draw(pTargetBitmap, targetPos);

		Activity::DrawGUI(pTargetBitmap, targetPos, which);
	}

	//////////////////////////////////////////////////////////////////////////////////////////
	// Method:          Draw
	//////////////////////////////////////////////////////////////////////////////////////////
	// Description:     Draws this BaseEditor's current graphical representation to a
	//                  BITMAP of choice. This includes all game-related graphics.

	void BaseEditor::Draw(BITMAP* pTargetBitmap, const Vector& targetPos) {
		Activity::Draw(pTargetBitmap, targetPos);
	}

	//////////////////////////////////////////////////////////////////////////////////////////
	// Method:          SaveScene
	//////////////////////////////////////////////////////////////////////////////////////////
	// Description:     Saves the current scene to an appropriate ini file, and asks user if
	//                  they want to overwrite first if scene of this name exists.

	bool BaseEditor::SaveScene(std::string saveAsName, bool forceOverwrite) {
		/*
		    // Set the name of the current scene in effect
		    g_SceneMan.GetScene()->SetPresetName(saveAsName);
		    // Try to save to the data module
		    string sceneFilePath(g_PresetMan.GetDataModule(m_ModuleSpaceID)->GetFileName() + "/Scenes/" + saveAsName + ".ini");
		    if (g_PresetMan.AddEntityPreset(g_SceneMan.GetScene(), m_ModuleSpaceID, forceOverwrite, sceneFilePath))
		    {
		        // Does ini already exist? If yes, then no need to add it to a scenes.ini etc
		        bool sceneFileExisted = System::PathExistsCaseSensitive(sceneFilePath.c_str());
		        // Create the writer
		        Writer sceneWriter(sceneFilePath.c_str(), false);
		        sceneWriter.NewProperty("AddScene");
		// TODO: Check if the ini file already exists, and then ask if overwrite
		        // Write the scene out to the new ini
		        sceneWriter << g_SceneMan.GetScene();

		        if (!sceneFileExisted)
		        {
		            // First find/create  a .rte/Scenes.ini file to include the new .ini into
		            string scenesFilePath(g_PresetMan.GetDataModule(m_ModuleSpaceID)->GetFileName() + "/Scenes.ini");
		            bool scenesFileExisted = System::PathExistsCaseSensitive(scenesFilePath.c_str());
		            Writer scenesWriter(scenesFilePath.c_str(), true);
		            scenesWriter.NewProperty("\nIncludeFile");
		            scenesWriter << sceneFilePath;

		            // Also add a line to the end of the modules' Index.ini to include the newly created Scenes.ini next startup
		            // If it's already included, it doens't matter, the definitions will just bounce the second time
		            if (!scenesFileExisted)
		            {
		                string indexFilePath(g_PresetMan.GetDataModule(m_ModuleSpaceID)->GetFileName() + "/Index.ini");
		                Writer indexWriter(indexFilePath.c_str(), true);
		                // Add extra tab since the DataModule has everything indented
		                indexWriter.NewProperty("\tIncludeFile");
		                indexWriter << scenesFilePath;
		            }
		        }
		        return m_HasEverBeenSaved = true;
		    }
		*/
		return false;
	}

} // namespace RTE
