#include "SceneEditorGUI.h"

#include "CameraMan.h"
#include "FrameMan.h"
#include "PresetMan.h"
#include "ActivityMan.h"
#include "GameActivity.h"
#include "SceneEditor.h"
#include "UInputMan.h"

#include "Controller.h"
#include "SceneObject.h"
#include "MOSprite.h"
#include "HeldDevice.h"
#include "HDFirearm.h"
#include "TDExplosive.h"
#include "TerrainObject.h"
#include "AHuman.h"
#include "ACrab.h"
#include "SLTerrain.h"
#include "ObjectPickerGUI.h"
#include "Scene.h"
#include "SettingsMan.h"
#include "Deployment.h"
#include "BunkerAssemblyScheme.h"

using namespace RTE;

#define MAXBRAINPATHCOST 10000
#define BLUEPRINTREVEALRATE 150
#define BLUEPRINTREVEALPAUSE 1500

BITMAP* SceneEditorGUI::s_pValidPathDot = 0;
BITMAP* SceneEditorGUI::s_pInvalidPathDot = 0;

void SceneEditorGUI::Clear() {
	m_pController = 0;
	m_FeatureSet = INGAMEEDIT;
	m_EditMade = false;
	m_EditorGUIMode = PICKINGOBJECT;
	m_PreviousMode = ADDINGOBJECT;
	m_ModeChanged = true;
	m_BlinkTimer.Reset();
	m_BlinkMode = NOBLINK;
	m_RepeatStartTimer.Reset();
	m_RepeatTimer.Reset();
	m_RevealTimer.Reset();
	m_RevealIndex = 0;
	m_PieMenu = nullptr;
	m_pPicker = 0;
	m_NativeTechModule = 0;
	m_ForeignCostMult = 4.0;
	m_GridSnapping = true;
	m_CursorPos.Reset();
	m_CursorOffset.Reset();
	m_CursorInAir = true;
	m_FacingLeft = false;
	m_PlaceTeam = Activity::TeamOne;
	m_pCurrentObject = 0;
	m_ObjectListOrder = -1;
	m_DrawCurrentObject = true;
	m_pObjectToBlink = 0;
	m_BrainSkyPath.clear();
	m_BrainSkyPathCost = 0;
	m_RequireClearPathToOrbit = false;
	m_PathRequest.reset();
}

int SceneEditorGUI::Create(Controller* pController, FeatureSets featureSet, int whichModuleSpace, int nativeTechModule, float foreignCostMult) {
	RTEAssert(pController, "No controller sent to SceneEditorGUI on creation!");
	m_pController = pController;

	SetFeatureSet(featureSet);

	// Update the brain path
	UpdateBrainPath();

	// Allocate and (re)create the Editor GUIs
	if (!m_pPicker)
		m_pPicker = new ObjectPickerGUI();
	else
		m_pPicker->Reset();
	m_pPicker->Create(pController, whichModuleSpace);

	m_NativeTechModule = nativeTechModule;
	m_ForeignCostMult = foreignCostMult;
	// Also apply these to the picker
	m_pPicker->SetNativeTechModule(m_NativeTechModule);
	m_pPicker->SetForeignCostMultiplier(m_ForeignCostMult);

	// Cursor init
	m_CursorPos = g_SceneMan.GetSceneDim() / 2;

	// Set initial focus, category std::list, and label settings
	m_EditorGUIMode = PICKINGOBJECT;
	m_ModeChanged = true;
	m_pCurrentObject = 0;

	// Reset repeat timers
	m_RepeatStartTimer.Reset();
	m_RepeatTimer.Reset();
	m_RevealTimer.Reset();
	m_RevealTimer.SetRealTimeLimitMS(100);

	// Check if we need to check for a clear path to orbit
	m_RequireClearPathToOrbit = false;
	GameActivity* gameActivity = dynamic_cast<GameActivity*>(g_ActivityMan.GetActivity());
	if (gameActivity) {
		m_RequireClearPathToOrbit = gameActivity->GetRequireClearPathToOrbit();
	}

	// Always disable clear path requirement in scene editor
	SceneEditor* editorActivity = dynamic_cast<SceneEditor*>(g_ActivityMan.GetActivity());
	if (editorActivity) {
		m_RequireClearPathToOrbit = false;
	}

	// Only load the static dot bitmaps once
	if (!s_pValidPathDot) {
		ContentFile dotFile("Base.rte/GUIs/Indicators/PathDotValid.png");
		s_pValidPathDot = dotFile.GetAsBitmap();
		dotFile.SetDataPath("Base.rte/GUIs/Indicators/PathDotInvalid.png");
		s_pInvalidPathDot = dotFile.GetAsBitmap();
	}

	return 0;
}

void SceneEditorGUI::Destroy() {
	delete m_pPicker;

	delete m_pCurrentObject;

	Clear();
}

void SceneEditorGUI::SetController(Controller* pController) {
	m_pController = pController;
	m_PieMenu->SetMenuController(pController);
	m_pPicker->SetController(pController);
}

void SceneEditorGUI::SetFeatureSet(SceneEditorGUI::FeatureSets newFeatureSet) {
	m_FeatureSet = newFeatureSet;
	if (m_PieMenu) {
		m_PieMenu = nullptr;
	}
	std::string pieMenuName;
	switch (m_FeatureSet) {
		case FeatureSets::ONLOADEDIT:
			pieMenuName = "Scene Editor Full Pie Menu";
			break;
		case FeatureSets::BLUEPRINTEDIT:
			pieMenuName = "Scene Editor Blueprint Pie Menu";
			break;
		case FeatureSets::AIPLANEDIT:
			pieMenuName = "Scene Editor AI Build Plan Pie Menu";
			break;
		case FeatureSets::INGAMEEDIT:
			pieMenuName = "Scene Editor In-Game Pie Menu";
			break;
		default:
			RTEAbort("Unhandled SceneEditorGUI FeatureSet when setting up PieMenu.");
	}
	m_PieMenu = std::unique_ptr<PieMenu>(dynamic_cast<PieMenu*>(g_PresetMan.GetEntityPreset("PieMenu", pieMenuName)->Clone()));
	// m_PieMenu->Create();
	m_PieMenu->SetMenuController(m_pController);
}

void SceneEditorGUI::SetPosOnScreen(int newPosX, int newPosY) {
	m_pPicker->SetPosOnScreen(newPosX, newPosY);
}

bool SceneEditorGUI::SetCurrentObject(SceneObject* pNewObject) {
	if (m_pCurrentObject == pNewObject)
		return true;

	// Replace the current object with the new one
	delete m_pCurrentObject;
	m_pCurrentObject = pNewObject;

	if (!m_pCurrentObject)
		return false;

	m_pCurrentObject->SetTeam(m_FeatureSet == ONLOADEDIT ? m_PlaceTeam : m_pController->GetTeam());
	m_pCurrentObject->SetPlacedByPlayer(m_pController->GetPlayer());

	// Disable any controller, if an actor
	if (Actor* pActor = dynamic_cast<Actor*>(m_pCurrentObject)) {
		pActor->GetController()->SetDisabled(true);
		pActor->SetStatus(Actor::INACTIVE);
	}

	return true;
}

PieSlice::SliceType SceneEditorGUI::GetActivatedPieSlice() const {
	return m_PieMenu->GetPieCommand();
}

void SceneEditorGUI::SetModuleSpace(int moduleSpaceID) {
	m_pPicker->SetModuleSpace(moduleSpaceID);
}

void SceneEditorGUI::SetNativeTechModule(int whichModule) {
	if (whichModule >= 0 && whichModule < g_PresetMan.GetTotalModuleCount()) {
		m_NativeTechModule = whichModule;
		m_pPicker->SetNativeTechModule(m_NativeTechModule);
	}
}

void SceneEditorGUI::SetForeignCostMultiplier(float newMultiplier) {
	m_ForeignCostMult = newMultiplier;
	m_pPicker->SetForeignCostMultiplier(m_ForeignCostMult);
}

bool SceneEditorGUI::TestBrainResidence(bool noBrainIsOK) {
	// Do we have a resident at all?
	SceneObject* pBrain = g_SceneMan.GetScene()->GetResidentBrain(m_pController->GetPlayer());

	// If not, can we find an unassigned brain and make it the resident?
	if (!pBrain) {
		pBrain = g_MovableMan.GetUnassignedBrain(g_ActivityMan.GetActivity()->GetTeamOfPlayer(m_pController->GetPlayer()));
		// Found one, so make it the resident brain (passing ownership) and remove it from the sim
		if (pBrain) {
			g_SceneMan.GetScene()->SetResidentBrain(m_pController->GetPlayer(), pBrain);
			g_MovableMan.RemoveMO(dynamic_cast<MovableObject*>(pBrain));
		}
	}

	// We do we have a resident brain now, so let's check that it's in a legit spot
	if (pBrain) {
		// Got to update the pathfinding graphs so the latest terrain is used for the below tests
		g_SceneMan.GetScene()->UpdatePathFinding();
		UpdateBrainSkyPathAndCost(pBrain->GetPos());
	} else {
		// No brain found, so we better place one
		if (!noBrainIsOK) {
			m_EditorGUIMode = INSTALLINGBRAIN;
			m_ModeChanged = true;
			UpdateBrainPath();
			g_GUISound.UserErrorSound()->Play(m_pController->GetPlayer());
		}
		return false;
	}

	// Block on our path request completing
	while (m_PathRequest && !m_PathRequest->complete) {};

	// Nope! Not valid spot for this brain we found, need to force user to re-place it
	if (m_BrainSkyPathCost > MAXBRAINPATHCOST && m_RequireClearPathToOrbit) {
		// Jump to where the bad brain is
		m_CursorPos = pBrain->GetPos();
		// Put the resident clone as the current object we need to place
		SetCurrentObject(dynamic_cast<SceneObject*>(pBrain->Clone()));
		// Get rid of the resident - it can't be helped there
		g_SceneMan.GetScene()->SetResidentBrain(m_pController->GetPlayer(), 0);
		// Switch to brain placement mode
		m_EditorGUIMode = INSTALLINGBRAIN;
		m_ModeChanged = true;
		UpdateBrainPath();
		g_GUISound.UserErrorSound()->Play(m_pController->GetPlayer());
		return false;
	}

	// Brain is fine, leave it be
	return true;
}

void SceneEditorGUI::Update() {
	// Update the user controller
	//    m_pController->Update();

	m_EditMade = false;
	m_pObjectToBlink = 0;
	// Which set of placed objects in the scene we're editing
	int editedSet = m_FeatureSet == ONLOADEDIT ? Scene::PLACEONLOAD : (m_FeatureSet == AIPLANEDIT ? Scene::AIPLAN : Scene::BLUEPRINT);

	////////////////////////////////////////////
	// Blinking logic
	/*
	    if (m_BlinkMode == OBJECTBLINK)
	    {
	        m_pCostLabel->SetVisible(m_BlinkTimer.AlternateSim(250));
	    }
	    else if (m_BlinkMode == NOCRAFT)
	    {
	        bool blink = m_BlinkTimer.AlternateSim(250);
	        m_pCraftLabel->SetVisible(blink);
	        m_pCraftBox->SetVisible(blink);
	    }

	    // Time out the blinker
	    if (m_BlinkMode != NOBLINK && m_BlinkTimer.IsPastSimMS(1500))
	    {
	        m_pCostLabel->SetVisible(true);
	        m_pCraftLabel->SetVisible(true);
	        m_pCraftBox->SetVisible(true);
	        m_BlinkMode = NOBLINK;
	    }
	*/

	if (m_pCurrentObject && m_EditorGUIMode != PICKINGOBJECT && g_PresetMan.GetReloadEntityPresetCalledThisUpdate()) {
		m_pCurrentObject = dynamic_cast<SceneObject*>(g_PresetMan.GetEntityPreset(m_pCurrentObject->GetClassName(), m_pCurrentObject->GetPresetName(), m_pCurrentObject->GetModuleName())->Clone());
	}

	/////////////////////////////////////////////
	// Repeating input logic

	bool pressLeft = m_pController->IsState(PRESS_LEFT);
	bool pressRight = m_pController->IsState(PRESS_RIGHT);
	bool pressUp = m_pController->IsState(PRESS_UP);
	bool pressDown = m_pController->IsState(PRESS_DOWN);

	// If no direciton is held down, then cancel the repeating
	if (!(m_pController->IsState(MOVE_RIGHT) || m_pController->IsState(MOVE_LEFT) || m_pController->IsState(MOVE_UP) || m_pController->IsState(MOVE_DOWN))) {
		m_RepeatStartTimer.Reset();
		m_RepeatTimer.Reset();
	}

	// Check if any direction has been held for the starting amount of time to get into repeat mode
	if (m_RepeatStartTimer.IsPastRealMS(200)) {
		// Check for the repeat interval
		if (m_RepeatTimer.IsPastRealMS(30)) {
			if (m_pController->IsState(MOVE_RIGHT))
				pressRight = true;
			else if (m_pController->IsState(MOVE_LEFT))
				pressLeft = true;

			if (m_pController->IsState(MOVE_UP))
				pressUp = true;
			else if (m_pController->IsState(MOVE_DOWN))
				pressDown = true;

			m_RepeatTimer.Reset();
		}
	}

	///////////////////////////////////////////////
	// Analog cursor input

	Vector analogInput;
	if (m_pController->GetAnalogMove().MagnitudeIsGreaterThan(0.1F))
		analogInput = m_pController->GetAnalogMove();
	//    else if (m_pController->GetAnalogAim().MagnitudeIsGreaterThan(0.1F))
	//        analogInput = m_pController->GetAnalogAim();

	/////////////////////////////////////////////
	// PIE MENU

	m_PieMenu->Update();

	// Show the pie menu only when the secondary button is held down
	if (m_pController->IsState(PIE_MENU_ACTIVE) && m_EditorGUIMode != INACTIVE && m_EditorGUIMode != PICKINGOBJECT) {
		m_PieMenu->SetEnabled(true);
		m_PieMenu->SetPos(m_GridSnapping ? g_SceneMan.SnapPosition(m_CursorPos) : m_CursorPos);

		std::array<PieSlice*, 2> infrontAndBehindPieSlices = {m_PieMenu->GetFirstPieSliceByType(PieSlice::SliceType::EditorInFront), m_PieMenu->GetFirstPieSliceByType(PieSlice::SliceType::EditorBehind)};
		for (PieSlice* pieSlice: infrontAndBehindPieSlices) {
			if (pieSlice) {
				pieSlice->SetEnabled(m_EditorGUIMode == ADDINGOBJECT);
			}
		}
	} else {
		m_PieMenu->SetEnabled(false);
	}

	///////////////////////////////////////
	// Handle pie menu selections

	if (m_PieMenu->GetPieCommand() != PieSlice::SliceType::NoType) {
		if (m_PieMenu->GetPieCommand() == PieSlice::SliceType::EditorPick) {
			m_EditorGUIMode = PICKINGOBJECT;
		} else if (m_PieMenu->GetPieCommand() == PieSlice::SliceType::EditorMove) {
			m_EditorGUIMode = MOVINGOBJECT;
		} else if (m_PieMenu->GetPieCommand() == PieSlice::SliceType::EditorRemove) {
			m_EditorGUIMode = DELETINGOBJECT;
		} else if (m_PieMenu->GetPieCommand() == PieSlice::SliceType::BrainHunt) {
			m_EditorGUIMode = INSTALLINGBRAIN;
		} else if (m_PieMenu->GetPieCommand() == PieSlice::SliceType::EditorDone) {
			m_EditorGUIMode = DONEEDITING;
		} else if (m_PieMenu->GetPieCommand() == PieSlice::SliceType::EditorInFront) {
			m_PreviousMode = m_EditorGUIMode;
			m_EditorGUIMode = PLACEINFRONT;
		} else if (m_PieMenu->GetPieCommand() == PieSlice::SliceType::EditorBehind) {
			m_PreviousMode = m_EditorGUIMode;
			m_EditorGUIMode = PLACEBEHIND;
		} else if (m_PieMenu->GetPieCommand() == PieSlice::SliceType::EditorTeam1) {
			m_PlaceTeam = Activity::TeamOne;
		} else if (m_PieMenu->GetPieCommand() == PieSlice::SliceType::EditorTeam2) {
			m_PlaceTeam = Activity::TeamTwo;
		} else if (m_PieMenu->GetPieCommand() == PieSlice::SliceType::EditorTeam3) {
			m_PlaceTeam = Activity::TeamThree;
		} else if (m_PieMenu->GetPieCommand() == PieSlice::SliceType::EditorTeam4) {
			m_PlaceTeam = Activity::TeamFour;
			// Toggle between normal scene object editing, and AI plan editing
		} else if (m_PieMenu->GetPieCommand() == PieSlice::SliceType::Map) {
			SetFeatureSet(m_FeatureSet == FeatureSets::ONLOADEDIT ? FeatureSets::AIPLANEDIT : FeatureSets::ONLOADEDIT);
		}

		UpdateBrainPath();
		m_ModeChanged = true;
	}

	//////////////////////////////////////////
	// Picker logic

	// Enable or disable the picker
	m_pPicker->SetEnabled(m_EditorGUIMode == PICKINGOBJECT);

	// Update the picker GUI
	m_pPicker->Update();

	if (m_EditorGUIMode == PICKINGOBJECT && m_pPicker->ObjectPicked()) {
		// Assign a copy of the picked object to be the currently held one.
		if (SetCurrentObject(dynamic_cast<SceneObject*>(m_pPicker->ObjectPicked()->Clone()))) {
			// Set the team
			if (m_FeatureSet != ONLOADEDIT)
				m_pCurrentObject->SetTeam(m_pController->GetTeam());
			// Set the std::list order to be at the end so new objects are added there
			m_ObjectListOrder = -1;
			// Update the object
			m_pCurrentObject->FullUpdate();
			// Update the path to the brain, or clear it if there's none
			UpdateBrainPath();

			// If done picking, revert to moving object mode
			if (m_pPicker->DonePicking()) {
				// If picked a Brain Actor, then enter install brain mode. Allow to use deployments in brain mode only while editing in-game
				if (m_pCurrentObject->IsInGroup("Brains") && (m_FeatureSet == INGAMEEDIT || !dynamic_cast<Deployment*>(m_pCurrentObject)))
					m_EditorGUIMode = INSTALLINGBRAIN;
				else
					m_EditorGUIMode = ADDINGOBJECT;

				UpdateBrainPath();
				m_ModeChanged = true;
			}
		}
	}

	if (!m_pPicker->IsVisible())
		g_CameraMan.SetScreenOcclusion(Vector(), g_ActivityMan.GetActivity()->ScreenOfPlayer(m_pController->GetPlayer()));
	else
		g_FrameMan.SetScreenText("Pick what you want to place next", g_ActivityMan.GetActivity()->ScreenOfPlayer(m_pController->GetPlayer()));

	/////////////////////////////////////
	// ADDING OBJECT MODE

	if (m_EditorGUIMode == ADDINGOBJECT && !m_PieMenu->IsEnabled()) {
		if (m_ModeChanged) {

			m_ModeChanged = false;
		}
		g_FrameMan.SetScreenText("Click to ADD a new object - Drag for precision", g_ActivityMan.GetActivity()->ScreenOfPlayer(m_pController->GetPlayer()));

		m_DrawCurrentObject = true;

		// Trap the mouse cursor
		g_UInputMan.TrapMousePos(true, m_pController->GetPlayer());

		// Move the cursor according to analog or mouse input
		if (!analogInput.IsZero()) {
			m_CursorPos += analogInput * 8;
			// Re-enable snapping only when the cursor is moved again
			m_GridSnapping = true;
		} else if (!m_pController->GetMouseMovement().IsZero()) {
			m_CursorPos += m_pController->GetMouseMovement();
			// Re-enable snapping only when the cursor is moved again
			m_GridSnapping = true;
		}
		// Digital input?
		else {
			if (pressUp)
				m_CursorPos.m_Y -= SCENESNAPSIZE;
			if (pressRight)
				m_CursorPos.m_X += SCENESNAPSIZE;
			if (pressDown)
				m_CursorPos.m_Y += SCENESNAPSIZE;
			if (pressLeft)
				m_CursorPos.m_X -= SCENESNAPSIZE;
			// Re-enable snapping only when the cursor is moved again
			if (pressUp || pressRight || pressDown || pressLeft)
				m_GridSnapping = true;
		}

		// Detect whether the cursor is in the air, or if it's overlapping some terrain
		Vector snappedPos = g_SceneMan.SnapPosition(m_CursorPos, m_GridSnapping);
		m_CursorInAir = g_SceneMan.GetTerrMatter(snappedPos.GetFloorIntX(), snappedPos.GetFloorIntY()) == g_MaterialAir;

		// Mousewheel is used as shortcut for getting next and prev items in teh picker's object std::list
		if (m_pController->IsState(SCROLL_UP) || m_pController->IsState(ControlState::ACTOR_NEXT)) {
			// Assign a copy of the next picked object to be the currently held one.
			const SceneObject* pNewObject = m_pPicker->GetPrevObject();
			if (pNewObject) {
				// Set and update the cursor object
				if (SetCurrentObject(dynamic_cast<SceneObject*>(pNewObject->Clone())))
					m_pCurrentObject->FullUpdate();
			}
		} else if (m_pController->IsState(SCROLL_DOWN) || m_pController->IsState(ControlState::ACTOR_PREV)) {
			// Assign a copy of the next picked object to be the currently held one.
			const SceneObject* pNewObject = m_pPicker->GetNextObject();
			if (pNewObject) {
				// Set and update the object
				if (SetCurrentObject(dynamic_cast<SceneObject*>(pNewObject->Clone())))
					m_pCurrentObject->FullUpdate();
			}
		}

		// Start the timer when the button is first pressed, and when the picker has deactivated
		if (m_pController->IsState(PRESS_PRIMARY) && !m_pPicker->IsVisible()) {
			m_BlinkTimer.Reset();
			m_EditorGUIMode = PLACINGOBJECT;
			m_PreviousMode = ADDINGOBJECT;
			m_ModeChanged = true;
			g_GUISound.PlacementBlip()->Play(m_pController->GetPlayer());
		}

		// Apply the team to the current actor, if applicable
		if (m_pCurrentObject && m_DrawCurrentObject) {
			// Set the team of SceneObject based on what's been selected
			// Only if full featured mode, otherwise it's based on the controller when placed
			if (m_FeatureSet == ONLOADEDIT)
				m_pCurrentObject->SetTeam(m_PlaceTeam);
		}
	}

	/////////////////////////////////////
	// INSTALLING BRAIN MODE

	if (m_EditorGUIMode == INSTALLINGBRAIN && !m_PieMenu->IsEnabled()) {
		if (m_ModeChanged) {
			// Check if we already have a resident brain to replace/re-place
			SceneObject* pBrain = g_SceneMan.GetScene()->GetResidentBrain(m_pController->GetPlayer());
			if (pBrain) {
				// Fly over to where the brain was previously found
				SetCursorPos(pBrain->GetPos());
				// If the brain in hand is not appropriate replacement, then just use the resident brain as a copy to re-place it
				if (!m_pCurrentObject->IsInGroup("Brains")) {
					// Make sure the player's resident brain is the one being held in cursor
					SetCurrentObject(dynamic_cast<SceneObject*>(pBrain->Clone()));
					// Clear the brain of the scene; it will be reinstated when we place it again
					// NOPE, keep it here in case placing the brain goes awry - it won't be drawn anyway
					//                  g_SceneMan.GetScene()->SetResidentBrain(m_pController->GetPlayer(), 0);
				}
			}
			// Ok, so no resident brain - do we have one in hand already??
			else if (m_pCurrentObject && m_pCurrentObject->IsInGroup("Brains")) {
				// Continue with placing it as per usual
				;
			}
			// Pick a brain to install if no one existed already in scene or in hand
			else {
				m_pPicker->SelectGroupByName("Brains");
				m_EditorGUIMode = PICKINGOBJECT;
				m_ModeChanged = true;
				UpdateBrainPath();
			}

			m_ModeChanged = false;
		}
		g_FrameMan.SetScreenText("Click to INSTALL your governor brain with a clear path to orbit - Drag for precision", g_ActivityMan.GetActivity()->ScreenOfPlayer(m_pController->GetPlayer()));

		m_DrawCurrentObject = true;

		// Trap the mouse cursor
		g_UInputMan.TrapMousePos(true, m_pController->GetPlayer());

		// Move the cursor according to analog or mouse input
		if (!analogInput.IsZero()) {
			m_CursorPos += analogInput * 8;
			// Re-enable snapping only when the cursor is moved again
			m_GridSnapping = true;
		} else if (!m_pController->GetMouseMovement().IsZero()) {
			m_CursorPos += m_pController->GetMouseMovement();
			// Re-enable snapping only when the cursor is moved again
			m_GridSnapping = true;
		}
		// Digital input?
		else {
			if (pressUp)
				m_CursorPos.m_Y -= SCENESNAPSIZE;
			if (pressRight)
				m_CursorPos.m_X += SCENESNAPSIZE;
			if (pressDown)
				m_CursorPos.m_Y += SCENESNAPSIZE;
			if (pressLeft)
				m_CursorPos.m_X -= SCENESNAPSIZE;
			// Re-enable snapping only when the cursor is moved again
			if (pressUp || pressRight || pressDown || pressLeft)
				m_GridSnapping = true;
		}

		// Detect whether the cursor is in the air, or if it's overlapping some terrain
		Vector snappedPos = g_SceneMan.SnapPosition(m_CursorPos, m_GridSnapping);
		m_CursorInAir = g_SceneMan.GetTerrMatter(snappedPos.GetFloorIntX(), snappedPos.GetFloorIntY()) == g_MaterialAir;

		// Check brain position validity with pathfinding and show a path to the sky
		UpdateBrainSkyPathAndCost(m_CursorPos);
		/*
		        // Process the new path we now have, if any
		        if (!m_BrainSkyPath.empty())
		        {
		            // Smash all airborne waypoints down to just above the ground, except for when it makes the path intersect terrain or it is the final destination
		            std::list<Vector>::iterator finalItr = m_BrainSkyPath.end();
		            finalItr--;
		            Vector smashedPoint;
		            Vector previousPoint = *(m_BrainSkyPath.begin());
		            std::list<Vector>::iterator nextItr = m_BrainSkyPath.begin();
		            for (std::list<Vector>::iterator lItr = m_BrainSkyPath.begin(); lItr != finalItr; ++lItr)
		            {
		                nextItr++;
		                smashedPoint = g_SceneMan.MovePointToGround((*lItr), 20, 10);
		          Vector notUsed;

		                // Only smash if the new location doesn't cause the path to intersect hard terrain ahead or behind of it
		                // Try three times to halve the height to see if that won't intersect
		                for (int i = 0; i < 3; i++)
		                {
		                    if (!g_SceneMan.CastStrengthRay(previousPoint, smashedPoint - previousPoint, 5, notUsed, 3, g_MaterialDoor) &&
		                        nextItr != m_BrainSkyPath.end() && !g_SceneMan.CastStrengthRay(smashedPoint, (*nextItr) - smashedPoint, 5, notUsed, 3, g_MaterialDoor))
		                    {
		                        (*lItr) = smashedPoint;
		                        break;
		                    }
		                    else
		                        smashedPoint.m_Y -= ((smashedPoint.m_Y - (*lItr).m_Y) / 2);
		                }

		                previousPoint = (*lItr);
		            }
		        }
		*/
		// Start the timer when the button is first pressed, and when the picker has deactivated
		if (m_pController->IsState(PRESS_PRIMARY) && !m_pPicker->IsVisible()) {
			m_BlinkTimer.Reset();
			m_EditorGUIMode = PLACINGOBJECT;
			m_PreviousMode = INSTALLINGBRAIN;
			m_ModeChanged = true;
			g_GUISound.PlacementBlip()->Play(m_pController->GetPlayer());
		}

		// Apply the team to the current actor, if applicable
		if (m_pCurrentObject && m_DrawCurrentObject) {
			// Set the team of SceneObject based on what's been selected
			// Only if full featured mode, otherwise it's based on the controller when placed
			if (m_FeatureSet == ONLOADEDIT)
				m_pCurrentObject->SetTeam(m_PlaceTeam);
		}
	}

	/////////////////////////////////////////////////////////////
	// PLACING MODE

	else if (m_EditorGUIMode == PLACINGOBJECT) {
		if (m_ModeChanged) {

			m_ModeChanged = false;
		}

		if (m_PreviousMode == MOVINGOBJECT)
			g_FrameMan.SetScreenText("Click and drag on a placed object to MOVE it - Click quickly to DETACH", g_ActivityMan.GetActivity()->ScreenOfPlayer(m_pController->GetPlayer()));
		else if (m_PreviousMode == INSTALLINGBRAIN)
			g_FrameMan.SetScreenText("Release to INSTALL the governor brain - Tap other button to cancel", g_ActivityMan.GetActivity()->ScreenOfPlayer(m_pController->GetPlayer()));
		else
			g_FrameMan.SetScreenText("Release to ADD the new object - Tap other button to cancel", g_ActivityMan.GetActivity()->ScreenOfPlayer(m_pController->GetPlayer()));

		// Check brain position validity with pathfinding and show a path to the sky
		if (m_PreviousMode == INSTALLINGBRAIN) {
			UpdateBrainSkyPathAndCost(m_CursorPos);
		}

		m_DrawCurrentObject = true;

		// Freeze when first pressing down and grid snapping is still engaged
		if (!(m_pController->IsState(PRIMARY_ACTION) && m_GridSnapping)) {
			if (!analogInput.IsZero()) {
				m_CursorPos += analogInput;
				m_FacingLeft = analogInput.m_X < 0 || (m_FacingLeft && analogInput.m_X == 0);
			}
			// Try the mouse
			else if (!m_pController->GetMouseMovement().IsZero()) {
				m_CursorPos += m_pController->GetMouseMovement();
				m_FacingLeft = m_pController->GetMouseMovement().m_X < 0 || (m_FacingLeft && m_pController->GetMouseMovement().m_X == 0);
			}
			// Digital input?
			else {
				if (pressUp)
					m_CursorPos.m_Y -= 1;
				if (pressRight) {
					m_CursorPos.m_X += 1;
					m_FacingLeft = false;
				}
				if (pressDown)
					m_CursorPos.m_Y += 1;
				if (pressLeft) {
					m_CursorPos.m_X -= 1;
					m_FacingLeft = true;
				}
			}

			// Detect whether the cursor is in the air, or if it's overlapping some terrain
			Vector snappedPos = g_SceneMan.SnapPosition(m_CursorPos, m_GridSnapping);
			m_CursorInAir = g_SceneMan.GetTerrMatter(snappedPos.GetFloorIntX(), snappedPos.GetFloorIntY()) == g_MaterialAir;
			// Also check that it isn't over unseen areas, can't place there
			m_CursorInAir = m_CursorInAir && !g_SceneMan.IsUnseen(snappedPos.GetFloorIntX(), snappedPos.GetFloorIntY(), m_pController->GetTeam());
		}

		// Constrain the cursor to only be within specific scene areas
		// TODO: THIS!!!

		// Disable snapping after a small interval of holding down the button, to avoid unintentional nudges when just placing on the grid
		if (m_pController->IsState(PRIMARY_ACTION) && m_BlinkTimer.IsPastRealMS(333) && m_GridSnapping) {
			m_GridSnapping = false;
			m_CursorPos = g_SceneMan.SnapPosition(m_CursorPos);
		}

		// Cancel placing if secondary button is pressed
		if (m_pController->IsState(PRESS_SECONDARY) || m_pController->IsState(PIE_MENU_ACTIVE)) {
			m_EditorGUIMode = m_PreviousMode;
			m_ModeChanged = true;
		}
		// If previous mode was moving, tear the gib loose if the button is released to soo
		else if (m_PreviousMode == MOVINGOBJECT && m_pController->IsState(RELEASE_PRIMARY) && !m_BlinkTimer.IsPastRealMS(150)) {
			m_EditorGUIMode = ADDINGOBJECT;
			m_ModeChanged = true;
		}
		// Only place if the picker and pie menus are completely out of view, to avoid immediate placing after picking
		else if (m_pCurrentObject && m_pController->IsState(RELEASE_PRIMARY) && !m_pPicker->IsVisible()) {
			m_pCurrentObject->FullUpdate();

			// Placing governor brain, which actually just puts it back into the resident brain roster
			if (m_PreviousMode == INSTALLINGBRAIN) {
				// Force our path request to complete so we know whether we can place or not
				while (m_PathRequest && !m_PathRequest->complete) {};

				// Only place if the brain has a clear path to the sky!
				if (m_BrainSkyPathCost <= MAXBRAINPATHCOST || !m_RequireClearPathToOrbit) {
					bool placeBrain = true;

					// Brain deployment's are translated into the appropriate loadout and put in place in the scene
					// but only while editing ingame
					Deployment* pDep = dynamic_cast<Deployment*>(m_pCurrentObject);
					if (pDep) {
						if (m_FeatureSet == INGAMEEDIT) {
							float cost;
							Actor* pActor = pDep->CreateDeployedActor(pDep->GetPlacedByPlayer(), cost);
							if (pActor && pActor->IsInGroup("Brains")) {
								delete m_pCurrentObject;
								m_pCurrentObject = pActor;
							} else {
								placeBrain = false;
							}
						} else {
							placeBrain = false;
						}
					}

					if (placeBrain) {
						// Place and let go (passing ownership) of the new governor brain
						g_SceneMan.GetScene()->SetResidentBrain(m_pController->GetPlayer(), m_pCurrentObject);
						// NO! This deletes the brain we just passed ownership of! just let go, man
						//                    SetCurrentObject(0);
						m_pCurrentObject = 0;
						// Nothing in cursor now, so force to pick somehting else to place right away
						m_EditorGUIMode = PICKINGOBJECT;
						m_ModeChanged = true;
						// Try to switch away from brains so it's clear we placed it
						if (!m_pPicker->SelectGroupByName("Weapons - Primary"))
							m_pPicker->SelectGroupByName("Weapons");
						// Also jolt the cursor off the newly placed brain to make it even clearer
						m_CursorPos.m_X += 40;
						m_CursorPos.m_Y += 10;
						UpdateBrainPath();
						g_GUISound.PlacementThud()->Play(m_pController->GetPlayer());
					}
				}
				// If no clear path to the sky, just reject the placment and keep the brain in hand
				else {
					g_FrameMan.ClearScreenText(g_ActivityMan.GetActivity()->ScreenOfPlayer(m_pController->GetPlayer()));
					g_FrameMan.SetScreenText("Your brain can only be placed with a clear access path to orbit!", g_ActivityMan.GetActivity()->ScreenOfPlayer(m_pController->GetPlayer()), 333, 3500);
					g_GUISound.UserErrorSound()->Play(m_pController->GetPlayer());
				}
			}
			// Non-brain thing is being placed
			else {
				// If we're not editing in-game, then just add to the placed objects std::list
				if (m_FeatureSet != INGAMEEDIT) {
					// If true we need to place object in the end, if false, then it was already given to an actor
					bool toPlace = true;

					// If we're placing an item then give that item to actor instead of dropping it nearby
					HeldDevice* pHeldDevice = dynamic_cast<HeldDevice*>(m_pCurrentObject);
					if (pHeldDevice)
						if (dynamic_cast<HDFirearm*>(pHeldDevice) || dynamic_cast<TDExplosive*>(pHeldDevice)) {
							int objectListPosition = -1;

							// Find out if we have AHuman under the cursor
							const SceneObject* pPickedSceneObject = g_SceneMan.GetScene()->PickPlacedObject(editedSet, m_CursorPos, &objectListPosition);
							const Actor* pAHuman = 0;

							// Looks like we got nothing, search for actors in range then
							if (!pPickedSceneObject)
								pPickedSceneObject = g_SceneMan.GetScene()->PickPlacedActorInRange(editedSet, m_CursorPos, 20, &objectListPosition);

							if (pPickedSceneObject) {
								pAHuman = dynamic_cast<const AHuman*>(pPickedSceneObject);

								if (!pAHuman) {
									// Maybe we clicked the underlying bunker module, search for actor in range
									pPickedSceneObject = g_SceneMan.GetScene()->PickPlacedActorInRange(editedSet, m_CursorPos, 20, &objectListPosition);

									if (pPickedSceneObject)
										pAHuman = dynamic_cast<const AHuman*>(pPickedSceneObject);
								}

								if (pAHuman) {
									// Create a new AHuman instead of old one, give him an item, and delete old AHuman
									SceneObject* pNewObject = dynamic_cast<SceneObject*>(pPickedSceneObject->Clone());
									g_SceneMan.GetScene()->RemovePlacedObject(editedSet, objectListPosition);

									AHuman* pAHuman = dynamic_cast<AHuman*>(pNewObject);
									if (pAHuman) {
										pAHuman->AddInventoryItem(dynamic_cast<MovableObject*>(m_pCurrentObject->Clone()));
										pAHuman->FlashWhite(150);
									}

									g_SceneMan.GetScene()->AddPlacedObject(editedSet, pNewObject, objectListPosition);

									m_EditMade = true;
									g_GUISound.PlacementThud()->Play(m_pController->GetPlayer());
									toPlace = false;
								}
							} else {
								// No human? Look for brains robots then
								SceneObject* pBrain = g_SceneMan.GetScene()->GetResidentBrain(m_pController->GetPlayer());
								if (pBrain) {
									if (g_SceneMan.ShortestDistance(pBrain->GetPos(), m_CursorPos, true).MagnitudeIsLessThan(20.0F)) {
										AHuman* pBrainAHuman = dynamic_cast<AHuman*>(pBrain);
										if (pBrainAHuman) {
											pBrainAHuman->AddInventoryItem(dynamic_cast<MovableObject*>(m_pCurrentObject->Clone()));
											pBrainAHuman->FlashWhite(150);
											m_EditMade = true;
											g_GUISound.PlacementThud()->Play(m_pController->GetPlayer());
											toPlace = false;
										}
									}
								}
							}
						}

					if (toPlace) {
						g_SceneMan.GetScene()->AddPlacedObject(editedSet, dynamic_cast<SceneObject*>(m_pCurrentObject->Clone()), m_ObjectListOrder);
						// Increment the std::list order so we place over last placed item
						if (m_ObjectListOrder >= 0)
							m_ObjectListOrder++;
						g_GUISound.PlacementThud()->Play(m_pController->GetPlayer());
						m_EditMade = true;
					}
				}
				// If in-game editing, then place into the sim
				else {
					// Check if team can afford the placed object and if so, deduct the cost
					if (g_ActivityMan.GetActivity()->GetTeamFunds(m_pController->GetTeam()) < m_pCurrentObject->GetTotalValue(m_NativeTechModule, m_ForeignCostMult)) {
						g_FrameMan.ClearScreenText(g_ActivityMan.GetActivity()->ScreenOfPlayer(m_pController->GetPlayer()));
						g_FrameMan.SetScreenText("You can't afford to place that!", g_ActivityMan.GetActivity()->ScreenOfPlayer(m_pController->GetPlayer()), 333, 1500);
						g_GUISound.UserErrorSound()->Play(m_pController->GetPlayer());
					} else {
						// TODO: Experimental! clean up this messiness
						SceneObject* pPlacedClone = dynamic_cast<SceneObject*>(m_pCurrentObject->Clone());
						pPlacedClone->SetTeam(m_pController->GetTeam());

						TerrainObject* pTO = dynamic_cast<TerrainObject*>(pPlacedClone);
						if (pTO) {
							// Deduct the cost from team funds
							g_ActivityMan.GetActivity()->ChangeTeamFunds(-m_pCurrentObject->GetTotalValue(m_NativeTechModule, m_ForeignCostMult), m_pController->GetTeam());

							pTO->PlaceOnTerrain(g_SceneMan.GetTerrain());
							g_SceneMan.GetTerrain()->CleanAir();

							Vector terrainObjectPos = pTO->GetPos() + pTO->GetBitmapOffset();
							if (pTO->HasBGColorBitmap()) {
								g_SceneMan.RegisterTerrainChange(terrainObjectPos.GetFloorIntX(), terrainObjectPos.GetFloorIntY(), pTO->GetBitmapWidth(), pTO->GetBitmapHeight(), ColorKeys::g_MaskColor, true);
							}
							if (pTO->HasFGColorBitmap()) {
								g_SceneMan.RegisterTerrainChange(terrainObjectPos.GetFloorIntX(), terrainObjectPos.GetFloorIntY(), pTO->GetBitmapWidth(), pTO->GetBitmapHeight(), ColorKeys::g_MaskColor, false);
							}

							// TODO: Make IsBrain function to see if one was placed
							if (pTO->GetPresetName() == "Brain Vault") {
								// Register the brain as this player's
								//                            g_ActivityMan.GetActivity()->SetPlayerBrain(pBrain, m_pController->GetPlayer());
								m_EditorGUIMode = PICKINGOBJECT;
								m_ModeChanged = true;
							}

							delete pPlacedClone;
							pPlacedClone = 0;
							g_GUISound.PlacementThud()->Play(m_pController->GetPlayer());
							g_GUISound.PlacementGravel()->Play(m_pController->GetPlayer());
							m_EditMade = true;
						}
						// Only place if the cursor is clear of terrain obstructions
						else if (m_CursorInAir) {
							float value = m_pCurrentObject->GetTotalValue(m_NativeTechModule, m_ForeignCostMult);

							// Deployment:s are translated into the appropriate loadout and put in place in the scene
							Deployment* pDep = dynamic_cast<Deployment*>(pPlacedClone);
							if (pDep) {
								// Ownership IS transferred here; pass it along into the MovableMan
								float cost = 0;
								Actor* pActor = pDep->CreateDeployedActor(pDep->GetPlacedByPlayer(), cost);
								if (pActor) {
									value = cost;
									g_MovableMan.AddActor(pActor);
								}
								// Just a simple Device in the Deployment?
								else {
									value = cost;
									// Get the Item/Device and add to scene, passing ownership
									SceneObject* pObject = pDep->CreateDeployedObject(pDep->GetPlacedByPlayer(), cost);
									MovableObject* pMO = dynamic_cast<MovableObject*>(pObject);
									if (pMO)
										g_MovableMan.AddMO(pMO);
									else {
										delete pObject;
										pObject = 0;
									}
								}
								delete pPlacedClone;
								pPlacedClone = 0;
								g_GUISound.PlacementThud()->Play(m_pController->GetPlayer());
								g_GUISound.PlacementGravel()->Play(m_pController->GetPlayer());
								m_EditMade = true;
							}

							// Deduct the cost from team funds
							g_ActivityMan.GetActivity()->ChangeTeamFunds(-value, m_pController->GetTeam());

							Actor* pActor = dynamic_cast<Actor*>(pPlacedClone);
							HeldDevice* pDevice = 0;
							if (pActor) {
								g_MovableMan.AddActor(pActor);
								m_EditMade = true;
							} else if (pDevice = dynamic_cast<HeldDevice*>(pPlacedClone)) {
								// If we have a friendly actor or brain nearby then give him an item instead of placing it
								bool toPlace = true;

								Vector distanceToActor;
								Actor* pNearestActor = g_MovableMan.GetClosestTeamActor(m_pController->GetTeam(), m_pController->GetPlayer(), pPlacedClone->GetPos(), 20, distanceToActor);

								// If we could not find an ordinary actor, then look for brain actor
								if (!pNearestActor) {
									// Find a brain and check if it's close enough
									SceneObject* pBrain = g_SceneMan.GetScene()->GetResidentBrain(m_pController->GetPlayer());
									if (pBrain) {
										if (g_SceneMan.ShortestDistance(pBrain->GetPos(), pPlacedClone->GetPos(), true).MagnitudeIsLessThan(20.0F)) {
											AHuman* pBrainAHuman = dynamic_cast<AHuman*>(pBrain);
											if (pBrainAHuman) {
												pNearestActor = pBrainAHuman;
											}
										}
									}
								}

								// If we have any AHuman actor then give it an item
								if (pNearestActor) {
									AHuman* pNearestAHuman = dynamic_cast<AHuman*>(pNearestActor);
									if (pNearestAHuman) {
										pNearestAHuman->AddInventoryItem(pDevice);
										// TODO: Resident brain remains white when flashed, don't know how to avoid that. Better than nothing though
										pNearestAHuman->FlashWhite(150);
										toPlace = false;
										m_EditMade = true;
									}
								}

								if (toPlace) {
									g_MovableMan.AddItem(pDevice);
									m_EditMade = true;
								}
							}
							// Something else
							else {
								MovableObject* pObj = dynamic_cast<MovableObject*>(pPlacedClone);
								if (pObj)
									g_MovableMan.AddParticle(pObj);
								m_EditMade = true;
							}
						}
						// Something was wrong with placement
						else {
							delete pPlacedClone;
							g_GUISound.UserErrorSound()->Play(m_pController->GetPlayer());
						}
					}
				}
			}
			// TEMP REMOVE WEHN YOU CLEAN UP THE ABOVE HARDCODED BRAIN PLACEMENT
			if (m_EditorGUIMode != PICKINGOBJECT)
				// TEMP REMOVE ABOVE
				// Go back to previous mode
				m_EditorGUIMode = m_PreviousMode;
			m_ModeChanged = true;
		}

		// Set the facing of AHumans based on right/left cursor movements
		// TODO: Improve
		Actor* pActor = dynamic_cast<Actor*>(m_pCurrentObject);
		if (pActor && dynamic_cast<AHuman*>(pActor) || dynamic_cast<ACrab*>(pActor))
			pActor->SetHFlipped(m_FacingLeft);

		Deployment* pDeployment = dynamic_cast<Deployment*>(m_pCurrentObject);
		if (pDeployment)
			pDeployment->SetHFlipped(m_FacingLeft);
	}

	/////////////////////////////////////////////////////////////
	// POINTING AT MODES

	else if ((m_EditorGUIMode == MOVINGOBJECT || m_EditorGUIMode == DELETINGOBJECT || m_EditorGUIMode == PLACEINFRONT || m_EditorGUIMode == PLACEBEHIND) && !m_PieMenu->IsEnabled()) {
		m_DrawCurrentObject = false;

		// Trap the mouse cursor
		g_UInputMan.TrapMousePos(true, m_pController->GetPlayer());

		// Move the cursor according to analog or mouse input
		if (!analogInput.IsZero())
			m_CursorPos += analogInput * 4;
		else if (!m_pController->GetMouseMovement().IsZero())
			m_CursorPos += m_pController->GetMouseMovement() / 2;
		// Digital input?
		else {
			if (pressUp)
				m_CursorPos.m_Y -= 1;
			if (pressRight)
				m_CursorPos.m_X += 1;
			if (pressDown)
				m_CursorPos.m_Y += 1;
			if (pressLeft)
				m_CursorPos.m_X -= 1;
		}

		/////////////////////////////////
		// MOVING OBJECT MODE

		if (m_EditorGUIMode == MOVINGOBJECT) {
			if (m_ModeChanged) {

				m_ModeChanged = false;
			}
			g_FrameMan.SetScreenText("Click and drag on a placed object to MOVE it - Click quickly to DETACH", g_ActivityMan.GetActivity()->ScreenOfPlayer(m_pController->GetPlayer()));

			// Pick an object under the cursor and start moving it
			if (m_pController->IsState(PRESS_PRIMARY) && !m_pPicker->IsVisible()) {
				const SceneObject* pPicked = g_SceneMan.GetScene()->PickPlacedObject(editedSet, m_CursorPos, &m_ObjectListOrder);
				if (pPicked) {
					// Grab the position and a copy of the the object itself before killing it from the scene
					SetCurrentObject(dynamic_cast<SceneObject*>(pPicked->Clone()));
					m_CursorOffset = m_CursorPos - m_pCurrentObject->GetPos();
					g_SceneMan.GetScene()->RemovePlacedObject(editedSet, m_ObjectListOrder);
					m_EditMade = true;

					// Go to placing mode to move it around
					m_EditorGUIMode = PLACINGOBJECT;
					m_PreviousMode = MOVINGOBJECT;
					m_ModeChanged = true;
					m_BlinkTimer.Reset();
					g_GUISound.PlacementBlip()->Play(m_pController->GetPlayer());
					g_GUISound.PlacementGravel()->Play(m_pController->GetPlayer());
				} else
					g_GUISound.UserErrorSound()->Play(m_pController->GetPlayer());
			}
		}

		////////////////////////////
		// REMOVING OBJECT MODE

		else if (m_EditorGUIMode == DELETINGOBJECT) {
			if (m_ModeChanged) {

				m_ModeChanged = false;
			}
			g_FrameMan.SetScreenText("Click and hold to select an object - release to DELETE it", g_ActivityMan.GetActivity()->ScreenOfPlayer(m_pController->GetPlayer()));

			// When primary is held down, pick object and show which one will be nuked if released
			if (m_pController->IsState(PRIMARY_ACTION) && !m_pPicker->IsVisible()) {
				m_pObjectToBlink = g_SceneMan.GetScene()->PickPlacedObject(editedSet, m_CursorPos);
			} else if (m_pController->IsState(RELEASE_PRIMARY)) {
				if (g_SceneMan.GetScene()->PickPlacedObject(editedSet, m_CursorPos, &m_ObjectListOrder)) {
					// Nuke it!
					g_SceneMan.GetScene()->RemovePlacedObject(editedSet, m_ObjectListOrder);
					m_EditMade = true;
					// TODO: Add awesome destruction sound here
				} else
					g_GUISound.UserErrorSound()->Play(m_pController->GetPlayer());
			}
		}

		/////////////////////////////////////
		// PLACE IN FRONT AND BEHIND OF MODES

		else if (m_EditorGUIMode == PLACEINFRONT || m_EditorGUIMode == PLACEBEHIND) {
			if (m_ModeChanged) {

				m_ModeChanged = false;
			}
			if (m_EditorGUIMode == PLACEINFRONT)
				g_FrameMan.SetScreenText(m_FeatureSet == ONLOADEDIT ? "Click an object to place the next one IN FRONT of it" : "Click an object to place the next one AFTER it in the build order", g_ActivityMan.GetActivity()->ScreenOfPlayer(m_pController->GetPlayer()));
			else if (m_EditorGUIMode == PLACEBEHIND)
				g_FrameMan.SetScreenText(m_FeatureSet == ONLOADEDIT ? "Click an object to place the next one BEHIND it" : "Click an object to insert the next one BEFORE it in the build order", g_ActivityMan.GetActivity()->ScreenOfPlayer(m_pController->GetPlayer()));

			// When primary is held down, pick object and show which one will be nuked if released
			if (m_pController->IsState(PRIMARY_ACTION) && !m_pPicker->IsVisible()) {
				m_pObjectToBlink = g_SceneMan.GetScene()->PickPlacedObject(editedSet, m_CursorPos);
			} else if (m_pController->IsState(RELEASE_PRIMARY)) {
				if (g_SceneMan.GetScene()->PickPlacedObject(editedSet, m_CursorPos, &m_ObjectListOrder)) {
					// Adjust the next std::list order to be in front if applicable (it's automatically behind if same order index)
					if (m_EditorGUIMode == PLACEINFRONT)
						m_ObjectListOrder++;

					// Go back to previous mode
					m_EditorGUIMode = m_PreviousMode;
					m_ModeChanged = true;
				} else
					g_GUISound.UserErrorSound()->Play(m_pController->GetPlayer());
			}
		}
	} else if (m_EditorGUIMode == DONEEDITING) {
		// Check first that the brain is in a good spot if finishing up a base edit
		if (m_FeatureSet != ONLOADEDIT)
			TestBrainResidence();
		//        if (m_FeatureSet != ONLOADEDIT)
		//            g_FrameMan.SetScreenText("DONE editing, wait for all other players to finish too...", g_ActivityMan.GetActivity()->ScreenOfPlayer(m_pController->GetPlayer()));
	}

	// Remove cursor offset if not applicable anymore
	if (m_EditorGUIMode != PLACINGOBJECT)
		m_CursorOffset.Reset();

	// Keep the cursor position within the world
	g_SceneMan.ForceBounds(m_CursorPos);
	// TODO: make setscrolltarget with 'sloppy' target
	// Scroll to the cursor's scene position
	g_CameraMan.SetScrollTarget(m_CursorPos, 0.3, g_ActivityMan.GetActivity()->ScreenOfPlayer(m_pController->GetPlayer()));
	// Apply the cursor position to the currently held object
	if (m_pCurrentObject && m_DrawCurrentObject) {
		m_pCurrentObject->SetPos(g_SceneMan.SnapPosition(m_CursorPos - m_CursorOffset, m_GridSnapping));
		// If an actor, set it to be inactive so it doesn't scan around and reveal unseen areas
		if (Actor* pCurrentActor = dynamic_cast<Actor*>(m_pCurrentObject)) {
			pCurrentActor->SetStatus(Actor::INACTIVE);
			pCurrentActor->GetController()->SetDisabled(true);
		}
		m_pCurrentObject->FullUpdate();
	}

	// Animate the reveal index so it is clear which order blueprint things are placed/built
	if (m_RevealTimer.IsPastRealTimeLimit()) {
		// Make the animation stop at full built and only reset after the pause
		if (m_RevealTimer.GetRealTimeLimitMS() > (BLUEPRINTREVEALRATE * 2))
			m_RevealIndex = 0;
		else
			m_RevealIndex++;

		// Loop
		if (m_RevealIndex >= g_SceneMan.GetScene()->GetPlacedObjects(m_FeatureSet == BLUEPRINTEDIT ? Scene::BLUEPRINT : Scene::AIPLAN)->size())
			// Set a long time when reset so the animation won't look so spazzy if there's only a few objects yet
			m_RevealTimer.SetRealTimeLimitMS(BLUEPRINTREVEALPAUSE);
		else
			m_RevealTimer.SetRealTimeLimitMS(BLUEPRINTREVEALRATE);

		m_RevealTimer.Reset();
	}
}

void SceneEditorGUI::Draw(BITMAP* pTargetBitmap, const Vector& targetPos) const {
	// Done, so don't draw the UI
	if (m_EditorGUIMode == DONEEDITING)
		return;

	// The get a std::list of the currently edited set of placed objects in the Scene
	const std::list<SceneObject*>* pSceneObjectList = 0;
	if (m_FeatureSet == ONLOADEDIT)
		pSceneObjectList = g_SceneMan.GetScene()->GetPlacedObjects(Scene::PLACEONLOAD);
	else if (m_FeatureSet == BLUEPRINTEDIT) {
		pSceneObjectList = g_SceneMan.GetScene()->GetPlacedObjects(Scene::BLUEPRINT);
		// Draw the 'original' set of placed scene objects as solid before the blueprints
		const std::list<SceneObject*>* pOriginalsList = g_SceneMan.GetScene()->GetPlacedObjects(Scene::PLACEONLOAD);
		for (std::list<SceneObject*>::const_iterator itr = pOriginalsList->begin(); itr != pOriginalsList->end(); ++itr) {
			(*itr)->Draw(pTargetBitmap, targetPos);
			// Draw basic HUD if an actor
			Actor* pActor = dynamic_cast<Actor*>(*itr);
			//            if (pActor)
			//                pActor->DrawHUD(pTargetBitmap, targetPos);
		}
	} else if (m_FeatureSet == AIPLANEDIT) {
		pSceneObjectList = g_SceneMan.GetScene()->GetPlacedObjects(Scene::AIPLAN);
		// Draw the 'original' set of placed scene objects as solid before the planned base
		const std::list<SceneObject*>* pOriginalsList = g_SceneMan.GetScene()->GetPlacedObjects(Scene::PLACEONLOAD);
		for (std::list<SceneObject*>::const_iterator itr = pOriginalsList->begin(); itr != pOriginalsList->end(); ++itr) {
			(*itr)->Draw(pTargetBitmap, targetPos);
			// Draw basic HUD if an actor
			Actor* pActor = dynamic_cast<Actor*>(*itr);
			//            if (pActor)
			//                pActor->DrawHUD(pTargetBitmap, targetPos);
		}
	}

	// Draw the set of currently edited objects already placed in the Scene
	if (pSceneObjectList) {
		// Draw all already placed Objects, and the currently held one in the order it is about to be placed in the scene
		int i = 0;
		Actor* pActor = 0;
		//        HeldDevice *pDevice = 0;
		for (std::list<SceneObject*>::const_iterator itr = pSceneObjectList->begin(); itr != pSceneObjectList->end(); ++itr, ++i) {
			// Draw the currently held object into the order of the std::list if it is to be placed inside
			if (m_pCurrentObject && m_DrawCurrentObject && i == m_ObjectListOrder) {
				g_FrameMan.SetTransTableFromPreset(m_BlinkTimer.AlternateReal(333) || m_EditorGUIMode == PLACINGOBJECT ? TransparencyPreset::LessTrans : TransparencyPreset::HalfTrans);
				m_pCurrentObject->Draw(pTargetBitmap, targetPos, g_DrawTrans);
				pActor = dynamic_cast<Actor*>(m_pCurrentObject);
				if (pActor)
					pActor->DrawHUD(pTargetBitmap, targetPos);
			}

			// Is the placed object an actor?
			pActor = dynamic_cast<Actor*>(*itr);
			//            pItem = dynamic_cast<MovableObject *>(*itr);

			// Blink trans if we are supposed to blink this one
			if ((*itr) == m_pObjectToBlink) {
				g_FrameMan.SetTransTableFromPreset(m_BlinkTimer.AlternateReal(333) ? TransparencyPreset::LessTrans : TransparencyPreset::HalfTrans);
				(*itr)->Draw(pTargetBitmap, targetPos, g_DrawTrans);
			}
			// Drawing of already placed objects that aren't highlighted or anything
			else {
				// In Base edit mode, draw them alternatingly transparent and solid to show that they're just designs
				if (m_FeatureSet == BLUEPRINTEDIT || m_FeatureSet == AIPLANEDIT) {
					//                    if (((int)m_BlinkTimer.GetElapsedRealTimeMS() % 2000) > 1750)
					//                    if (m_BlinkTimer.AlternateReal(1000))
					// Animate the ghosted into appearing solid in the build order to make the order clear
					if (i >= m_RevealIndex) {
						g_FrameMan.SetTransTableFromPreset(pActor ? TransparencyPreset::MoreTrans : TransparencyPreset::HalfTrans);
						(*itr)->Draw(pTargetBitmap, targetPos, g_DrawTrans);
					}
					// Show as non-transparent half the time to still give benefits of WYSIWYG
					else
						(*itr)->Draw(pTargetBitmap, targetPos);
				}
				// In full scene edit mode, we want to give a WYSIWYG view
				else {
					(*itr)->Draw(pTargetBitmap, targetPos);

					// Draw team marks for doors, deployments and assemblies
					Deployment* pDeployment = dynamic_cast<Deployment*>(*itr);
					if (pDeployment)
						(*itr)->DrawTeamMark(pTargetBitmap, targetPos);

					// Works for both doors and bunker assemblies
					TerrainObject* pTObject = dynamic_cast<TerrainObject*>(*itr);
					if (pTObject && !pTObject->GetChildObjects().empty())
						(*itr)->DrawTeamMark(pTargetBitmap, targetPos);

					BunkerAssemblyScheme* pBA = dynamic_cast<BunkerAssemblyScheme*>(*itr);
					if (pBA)
						(*itr)->DrawTeamMark(pTargetBitmap, targetPos);
				}
			}

			// Draw basic HUD if an actor - don't do this for blueprints.. it is confusing
			if (pActor && m_FeatureSet != BLUEPRINTEDIT && m_FeatureSet != AIPLANEDIT)
				pActor->DrawHUD(pTargetBitmap, targetPos);
		}
	}

	// Draw the path between brain and sky, backwards so the dot spacing can be even and they don't crawl as the guy approaches
	// Always draw the path so the player can see what he would be blocking off when building
	if (m_RequireClearPathToOrbit && !m_BrainSkyPath.empty()) {
		int skipPhase = 0;
		std::list<Vector>::const_reverse_iterator lLast = m_BrainSkyPath.rbegin();
		std::list<Vector>::const_reverse_iterator lItr = m_BrainSkyPath.rbegin();
		for (; lItr != m_BrainSkyPath.rend(); ++lItr) {
			// Draw these backwards so the skip phase works
			skipPhase = g_FrameMan.DrawDotLine(pTargetBitmap, (*lLast) - targetPos, (*lItr) - targetPos, m_BrainSkyPathCost <= MAXBRAINPATHCOST ? s_pValidPathDot : s_pInvalidPathDot, 16, skipPhase, true);
			lLast = lItr;
		}
	}

	// Draw the currently placed brain, if any, UNLESS it's being re-placed atm
	if ((m_pCurrentObject && !m_pCurrentObject->IsInGroup("Brains")) && m_EditorGUIMode != INSTALLINGBRAIN && !(m_EditorGUIMode == PLACINGOBJECT && m_PreviousMode == INSTALLINGBRAIN)) {
		SceneObject* pBrain = g_SceneMan.GetScene()->GetResidentBrain(m_pController->GetPlayer());
		if (pBrain) {
			pBrain->Draw(pTargetBitmap, targetPos);
			// Draw basic HUD if an actor
			Actor* pActor = dynamic_cast<Actor*>(pBrain);
			if (pActor)
				pActor->DrawHUD(pTargetBitmap, targetPos);
		}
	}

	// Draw picking object crosshairs and not the selected object
	if (!m_DrawCurrentObject) {
		Vector center = m_CursorPos - targetPos;
		putpixel(pTargetBitmap, center.m_X, center.m_Y, g_YellowGlowColor);
		hline(pTargetBitmap, center.m_X - 5, center.m_Y, center.m_X - 2, g_YellowGlowColor);
		hline(pTargetBitmap, center.m_X + 5, center.m_Y, center.m_X + 2, g_YellowGlowColor);
		vline(pTargetBitmap, center.m_X, center.m_Y - 5, center.m_Y - 2, g_YellowGlowColor);
		vline(pTargetBitmap, center.m_X, center.m_Y + 5, center.m_Y + 2, g_YellowGlowColor);
	}
	// If the held object will be placed at the end of the std::list, draw it last to the scene, transperent blinking
	else if (m_pCurrentObject && (m_ObjectListOrder < 0 || (pSceneObjectList && m_ObjectListOrder == pSceneObjectList->size()))) {
		g_FrameMan.SetTransTableFromPreset(m_BlinkTimer.AlternateReal(333) || m_EditorGUIMode == PLACINGOBJECT ? TransparencyPreset::LessTrans : TransparencyPreset::HalfTrans);
		m_pCurrentObject->Draw(pTargetBitmap, targetPos, g_DrawTrans);
		Actor* pActor = dynamic_cast<Actor*>(m_pCurrentObject);
		if (pActor && m_FeatureSet != BLUEPRINTEDIT && m_FeatureSet != AIPLANEDIT)
			pActor->DrawHUD(pTargetBitmap, targetPos);
	}

	m_pPicker->Draw(pTargetBitmap);

	// Draw the pie menu
	m_PieMenu->Draw(pTargetBitmap, targetPos);
}

void SceneEditorGUI::UpdateBrainSkyPathAndCost(Vector brainPos) {
	if (!m_RequireClearPathToOrbit) {
		m_PathRequest.reset();
		return;
	}

	if (m_PathRequest) {
		if (!m_PathRequest->complete) {
			// Wait for the request to complete
			return;
		}

		if (g_SceneMan.GetScene()->PositionsAreTheSamePathNode(const_cast<Vector&>(m_PathRequest->targetPos), brainPos)) {
			// No need to recalculate
			return;
		}
	}

	Vector orbitPos;

	int staticPos;
	int checkPos;
	int lengthToCheck;

	// If the ceiling directly above is blocked, search the surroundings for gaps.
	Directions orbitDirection = g_SceneMan.GetTerrain()->GetOrbitDirection();
	switch (orbitDirection) {
		default:
		case Directions::Up:
			checkPos = brainPos.GetFloorIntX();
			staticPos = 0;
			lengthToCheck = g_SceneMan.GetSceneWidth();
			break;
		case Directions::Down:
			checkPos = brainPos.GetFloorIntX();
			staticPos = g_SceneMan.GetSceneHeight();
			lengthToCheck = g_SceneMan.GetSceneWidth();
			break;
		case Directions::Left:
			checkPos = brainPos.GetFloorIntY();
			staticPos = 0;
			lengthToCheck = g_SceneMan.GetSceneHeight();
			break;
		case Directions::Right:
			checkPos = brainPos.GetFloorIntY();
			staticPos = g_SceneMan.GetSceneWidth();
			lengthToCheck = g_SceneMan.GetSceneHeight();
			break;
	}

	auto getVector = [&](int pos) {
		switch (orbitDirection) {
			default:
			case Directions::Up:
				return Vector(pos, 0);
			case Directions::Down:
				return Vector(pos, g_SceneMan.GetSceneHeight());
			case Directions::Left:
				return Vector(0, pos);
			case Directions::Right:
				return Vector(g_SceneMan.GetSceneWidth(), pos);
		}
	};

	int offset = 0;
	int spacing = 20;
	for (int i = 0; i < lengthToCheck / spacing; i++) {
		int offsetCheckPos = checkPos + offset;
		orbitPos = getVector(offsetCheckPos);
		if (!g_SceneMan.IsWithinBounds(orbitPos.GetFloorIntX(), orbitPos.GetFloorIntY())) {
			offset *= -1;
			offsetCheckPos = checkPos + offset;
			orbitPos = getVector(offsetCheckPos);
		}

		if (g_SceneMan.GetTerrMatter(orbitPos.GetFloorIntX(), orbitPos.GetFloorIntY()) == g_MaterialAir) {
			break;
		} else {
			offset = i * (i % 2 == 0 ? spacing : -spacing);
		}
	}

	Activity::Teams team = static_cast<Activity::Teams>(g_ActivityMan.GetActivity()->GetTeamOfPlayer(m_pController->GetPlayer()));
	m_PathRequest = g_SceneMan.GetScene()->CalculatePathAsync(orbitPos, brainPos, c_PathFindingDefaultDigStrength, team,
	                                                          [&](std::shared_ptr<volatile PathRequest> pathRequest) {
		                                                          m_BrainSkyPath = const_cast<std::list<Vector>&>(pathRequest->path);
		                                                          m_BrainSkyPathCost = pathRequest->totalCost;
	                                                          });
}

bool SceneEditorGUI::UpdateBrainPath() {
	if (m_pCurrentObject && m_pCurrentObject->IsInGroup("Brains")) {
		UpdateBrainSkyPathAndCost(m_CursorPos);
	} else if (SceneObject* residentBrain = g_SceneMan.GetScene()->GetResidentBrain(m_pController->GetPlayer())) {
		UpdateBrainSkyPathAndCost(residentBrain->GetPos());
	} else {
		m_BrainSkyPath.clear();
		m_BrainSkyPathCost = 0;
		return false;
	}
	return true;
}
