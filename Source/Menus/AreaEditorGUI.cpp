//////////////////////////////////////////////////////////////////////////////////////////
// File:            AreaEditorGUI.cpp
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Implementation file for the AreaEditorGUI class
// Project:         GUI Library
// Author(s):       Daniel Tabar
//                  dtabar@datArealms.com
//                  http://www.datArealms.com

//////////////////////////////////////////////////////////////////////////////////////////
// Inclusions of header files

#include "AreaEditorGUI.h"

#include "CameraMan.h"
#include "FrameMan.h"
#include "PresetMan.h"
#include "ActivityMan.h"
#include "UInputMan.h"

#include "Controller.h"
#include "MOSprite.h"
#include "HeldDevice.h"
#include "AHuman.h"
#include "ACrab.h"
#include "SLTerrain.h"
#include "AreaPickerGUI.h"
#include "Scene.h"
#include "GUISound.h"

using namespace RTE;

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Clear
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Clears all the member variables of this AreaEditorGUI, effectively
//                  resetting the members of this abstraction level only.

void AreaEditorGUI::Clear() {
	m_pController = 0;
	m_FullFeatured = false;
	m_EditMade = false;
	m_EditorGUIMode = PICKINGAREA;
	m_PreviousMode = PREADDMOVEBOX;
	m_BlinkTimer.Reset();
	m_BlinkMode = NOBLINK;
	m_RepeatStartTimer.Reset();
	m_RepeatTimer.Reset();
	m_PieMenu = nullptr;
	m_pPicker = 0;
	m_GridSnapping = true;
	m_CursorPos.Reset();
	m_CursorOffset.Reset();
	m_CursorInAir = true;
	m_pCurrentArea = 0;
	m_EditedBox.Reset();
	m_pBoxToBlink = 0;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Create
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Makes the AreaEditorGUI Area ready for use.

int AreaEditorGUI::Create(Controller* pController, bool fullFeatured, int whichModuleSpace) {
	RTEAssert(pController, "No controller sent to AreaEditorGUI on creation!");
	m_pController = pController;

	m_FullFeatured = fullFeatured;

	if (m_PieMenu) {
		m_PieMenu = nullptr;
	}
	std::string pieMenuName = m_FullFeatured ? "Area Editor Full Pie Menu" : "Area Editor Minimal Pie Menu";
	m_PieMenu = std::unique_ptr<PieMenu>(dynamic_cast<PieMenu*>(g_PresetMan.GetEntityPreset("PieMenu", pieMenuName)->Clone()));
	m_PieMenu->SetMenuController(pController);

	// Allocate and (re)create the Editor GUIs
	if (!m_pPicker)
		m_pPicker = new AreaPickerGUI();
	else
		m_pPicker->Destroy();
	m_pPicker->Create(pController);

	// Cursor init
	m_CursorPos = g_SceneMan.GetSceneDim() / 2;

	// Set initial focus, category list, and label settings
	m_EditorGUIMode = PICKINGAREA;
	m_pCurrentArea = 0;

	// Reset repeat timers
	m_RepeatStartTimer.Reset();
	m_RepeatTimer.Reset();

	return 0;
}

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Destroy
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Destroys and resets (through Clear()) the AreaEditorGUI Area.

void AreaEditorGUI::Destroy() {
	delete m_pPicker;

	Clear();
}

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetController
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the controller used by this. The ownership of the controller is
//                  NOT transferred!

void AreaEditorGUI::SetController(Controller* pController) {
	m_pController = pController;
	m_PieMenu->SetMenuController(pController);
	m_pPicker->SetController(pController);
}

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetPosOnScreen
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets where on the screen that this GUI is being drawn to. If upper
//                  left corner, then 0, 0. This will affect the way the mouse is positioned
//                  etc.

void AreaEditorGUI::SetPosOnScreen(int newPosX, int newPosY) {
	m_pPicker->SetPosOnScreen(newPosX, newPosY);
}

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          GetActivatedPieSlice
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Gets any Pie menu slice command activated last update.

PieSlice::SliceType AreaEditorGUI::GetActivatedPieSlice() const {
	return m_PieMenu->GetPieCommand();
}

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          SetCurrentArea
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Sets the currently selected Area of this Editor. Ownership IS NOT
//                  transferred!

void AreaEditorGUI::SetCurrentArea(Scene::Area* pArea) {
	if (!pArea)
		return;

	m_pCurrentArea = pArea;
	// Display the name of the newly seclected Area in the center of the screen
	g_FrameMan.ClearScreenText(g_ActivityMan.GetActivity()->ScreenOfPlayer(m_pController->GetPlayer()));
	g_FrameMan.SetScreenText(m_pCurrentArea->GetName(), g_ActivityMan.GetActivity()->ScreenOfPlayer(m_pController->GetPlayer()), 0, 1500, true);
	// Make the yellow outline edited box vanish
	m_EditedBox.Reset();

	// Jump the cursor pos to a reasonable center of the newly selected Area's coverage area
	if (!m_pCurrentArea->m_BoxList.empty()) {
		// Average center of the all the boxes, weighted by their respective areas
		m_CursorPos = m_pCurrentArea->GetCenterPoint();
	}
}

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          UpdatePickerList
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates the list that the GUI's Area picker has, from the current
//                  scene state.

void AreaEditorGUI::UpdatePickerList(std::string selectAreaName) {
	m_pPicker->UpdateAreasList(selectAreaName);
}

//////////////////////////////////////////////////////////////////////////////////////////
// Method:          Update
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Updates the state of this Menu each frame

void AreaEditorGUI::Update() {
	// Update the user controller
	//    m_pController->Update();

	if (!g_SceneMan.GetScene())
		return;

	// If no Area is selected yet, and there are Areas in the current scene, then select the first one automatically
	if (!m_pCurrentArea && !g_SceneMan.GetScene()->m_AreaList.empty())
		m_pCurrentArea = &(g_SceneMan.GetScene()->m_AreaList.front());

	m_EditMade = false;
	m_pBoxToBlink = 0;

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
	if (m_pController->IsState(PRESS_SECONDARY) && m_EditorGUIMode != INACTIVE && m_EditorGUIMode != PICKINGAREA) {
		m_PieMenu->SetPos(m_GridSnapping ? g_SceneMan.SnapPosition(m_CursorPos) : m_CursorPos);
		m_PieMenu->SetEnabled(true);
	}

	if (!m_pController->IsState(PIE_MENU_ACTIVE) || m_EditorGUIMode == INACTIVE || m_EditorGUIMode == PICKINGAREA) {
		m_PieMenu->SetEnabled(false);
	}

	if (m_PieMenu->GetPieCommand() != PieSlice::SliceType::NoType) {
		if (m_PieMenu->GetPieCommand() == PieSlice::SliceType::EditorPick) {
			m_EditorGUIMode = PICKINGAREA;
		} else if (m_PieMenu->GetPieCommand() == PieSlice::SliceType::EditorMove) {
			m_EditorGUIMode = PREADDMOVEBOX;
		} else if (m_PieMenu->GetPieCommand() == PieSlice::SliceType::EditorRemove) {
			m_EditorGUIMode = DELETINGBOX;
		} else if (m_PieMenu->GetPieCommand() == PieSlice::SliceType::EditorDone) {
			m_EditorGUIMode = DONEEDITING;
		}
	}

	//////////////////////////////////////////
	// Picker logic

	// Enable or disable the picker
	m_pPicker->SetEnabled(m_EditorGUIMode == PICKINGAREA);

	// Update the picker GUI
	m_pPicker->Update();

	if (m_EditorGUIMode == PICKINGAREA && m_pPicker->AreaPicked()) {
		// Assign the pointer of the picked Area to be the currently selected one.
		if (m_pPicker->AreaPicked()) {
			SetCurrentArea(m_pPicker->AreaPicked());
			// TODO: Make the view center on the newly picked Area, somehow using average of all its Box:es centers?

			// If done picking, revert to adding/movind Box mode for the newly selected Area
			if (m_pPicker->DonePicking()) {
				m_EditorGUIMode = PREADDMOVEBOX;
			}
		}
	}

	if (!m_pPicker->IsVisible())
		g_CameraMan.SetScreenOcclusion(Vector(), g_ActivityMan.GetActivity()->ScreenOfPlayer(m_pController->GetPlayer()));

	if (m_EditorGUIMode != PICKINGAREA) {
		// Mousewheel is used as shortcut for getting next and prev items in the picker's Area list
		if (m_pController->IsState(SCROLL_UP)) {
			// Assign the non-owned pointer of the next picked Area to be the currently selected one.
			Scene::Area* pNewArea = m_pPicker->GetPrevArea();
			if (pNewArea)
				SetCurrentArea(pNewArea);
		} else if (m_pController->IsState(SCROLL_DOWN)) {
			// Assign the non-owned pointer of the next picked Area to be the currently selected one.
			Scene::Area* pNewArea = m_pPicker->GetNextArea();
			if (pNewArea)
				SetCurrentArea(pNewArea);
		}
	}

	// Make sure we have a picked area if there are any areas at all!
	if (!m_pCurrentArea && !g_SceneMan.GetScene()->m_AreaList.empty())
		m_pCurrentArea = &(g_SceneMan.GetScene()->m_AreaList.front());
	// If there are no Area:s, AreaEditor should detect it and force user to create a new one with a dialog
	//    else
	//        m_EditorGUIMode = PREADDMOVEBOX;

	/////////////////////////////////////
	// ADDING or MOVING BOX MODE

	if (m_pCurrentArea && m_EditorGUIMode == PREADDMOVEBOX && !m_PieMenu->IsEnabled()) {
		g_FrameMan.SetScreenText("Click and drag to ADD a new box to the Area - Drag existing ones to MOVE them", g_ActivityMan.GetActivity()->ScreenOfPlayer(m_pController->GetPlayer()));

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

		Box* pBox = 0;

		// Start the timer when the button is first pressed, and when the picker has deactivated
		if (m_pController->IsState(PRESS_PRIMARY) && !m_pPicker->IsVisible()) {
			m_BlinkTimer.Reset();

			// See if the start of the drag is inside an existing Box, and set the next mode (MOVE) accordingly
			if (pBox = m_pCurrentArea->GetBoxInside(m_CursorPos)) {
				// Save a copy of the dragged box
				m_EditedBox = *pBox;
				// Set the offset between the Box being dragged and the cursor so the box doesn't jump
				m_CursorOffset = m_CursorPos - m_EditedBox.GetCorner();
				// Remove the dragged box from the area, we are now in control of it
				m_pCurrentArea->RemoveBoxInside(m_CursorPos);
				// Switch the mode
				m_EditorGUIMode = MOVINGBOX;
				m_PreviousMode = PREADDMOVEBOX;
			}
			// Or outside all, and set the next mode to be NEW instead to start a new box drag to add to the current Area
			else {
				// Place the start corner of the new box
				m_EditedBox.SetCorner(m_CursorPos);
				// Cursor offset is 0 becuase we're placing the corner with current cursor pos
				m_CursorOffset.Reset();
				// Switch mode
				m_EditorGUIMode = ADDINGBOX;
				m_PreviousMode = PREADDMOVEBOX;
			}

			g_GUISound.PlacementBlip()->Play();
		}
		// Just hovering over things, show what would be moved if we started dragging
		else {
			pBox = m_pCurrentArea->GetBoxInside(m_CursorPos);
			if (pBox)
				m_EditedBox = *pBox;
			else
				m_EditedBox.Reset();
		}
	}

	/////////////////////////////////////////////////////////////
	// POINTING AT/DRAGGING MODES WITHOUT SNAPPING

	else if (m_pCurrentArea && (m_EditorGUIMode == MOVINGBOX || m_EditorGUIMode == ADDINGBOX || m_EditorGUIMode == DELETINGBOX) && !m_PieMenu->IsEnabled()) {
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
		// MOVING BOX MODE

		if (m_EditorGUIMode == MOVINGBOX) {
			g_FrameMan.SetScreenText("Keep dragging the box to MOVE it", g_ActivityMan.GetActivity()->ScreenOfPlayer(m_pController->GetPlayer()));

			// While primary is kept held down, keep dragging the Box
			if (m_pController->IsState(PRIMARY_ACTION)) {
				// TODO: really wrap?
				Vector wrappedCorner = m_CursorPos - m_CursorOffset;
				g_SceneMan.WrapPosition(wrappedCorner);
				m_EditedBox.SetCorner(wrappedCorner);
			}
			// When released, we are done moving that box and go back to prev mode
			else if (m_pController->IsState(RELEASE_PRIMARY)) {
				// Add it back to the Area
				m_EditedBox.Unflip();
				m_pCurrentArea->AddBox(m_EditedBox);
				// Make the yellow outline edited box vanish
				m_EditedBox.Reset();
				m_EditMade = true;
				m_EditorGUIMode = PREADDMOVEBOX;
				m_PreviousMode = MOVINGBOX;
				g_GUISound.PlacementThud()->Play();
			}
		}

		/////////////////////////////////
		// ADDING BOX MODE

		if (m_EditorGUIMode == ADDINGBOX) {
			g_FrameMan.SetScreenText("Keep dragging the new box out - release and it is ADDED to the current Area", g_ActivityMan.GetActivity()->ScreenOfPlayer(m_pController->GetPlayer()));

			// While primary is kept held down, keep dragging out the otehr corner of the Box being defined
			if (m_pController->IsState(PRIMARY_ACTION)) {
				Vector dimensions = g_SceneMan.ShortestDistance(m_EditedBox.GetCorner(), m_CursorPos).GetFloored();
				m_EditedBox.SetWidth(dimensions.m_X);
				m_EditedBox.SetHeight(dimensions.m_Y);
			}
			// When released, we are done with that box and go back to prev mode
			else if (m_pController->IsState(RELEASE_PRIMARY)) {
				m_EditedBox.Unflip();
				m_pCurrentArea->AddBox(m_EditedBox);
				// Make the yellow outline edited box vanish
				m_EditedBox.Reset();
				m_EditMade = true;
				m_EditorGUIMode = PREADDMOVEBOX;
				m_PreviousMode = ADDINGBOX;
				g_GUISound.PlacementThud()->Play();
			}
		}

		////////////////////////////
		// REMOVING BOX MODE

		else if (m_EditorGUIMode == DELETINGBOX) {
			g_FrameMan.SetScreenText("Click and hold to select a Box - release to DELETE it", g_ActivityMan.GetActivity()->ScreenOfPlayer(m_pController->GetPlayer()));

			m_EditedBox.Reset();

			// When primary is held down, pick Box and show which one will be nuked if released
			if (m_pController->IsState(PRIMARY_ACTION) && !m_pPicker->IsVisible()) {
				Box* pBoxToDelete = m_pCurrentArea->GetBoxInside(m_CursorPos);
				// Indicate which box we're talking aobut to delete
				if (pBoxToDelete)
					m_EditedBox = *pBoxToDelete;
			} else if (m_pController->IsState(RELEASE_PRIMARY)) {
				Box removed = m_pCurrentArea->RemoveBoxInside(m_CursorPos);
				// If we didnt' remove any box, play error sound
				if (removed.IsEmpty())
					g_GUISound.UserErrorSound()->Play();
				else
					m_EditMade = true;
			}
		}
	} else if (m_EditorGUIMode == DONEEDITING) {
		//        if (!m_FullFeatured)
		//            g_FrameMan.SetScreenText("DONE editing, wait for all other players to finish too...", ScreenOfPlayer(m_pController->GetPlayer()));
	}

	// Remove cursor offset if not applicable anymore
	if (m_EditorGUIMode == PREADDMOVEBOX)
		m_CursorOffset.Reset();

	// Keep the cursor position within the world
	bool cursorWrapped = g_SceneMan.ForceBounds(m_CursorPos);
	// TODO: make setscrolltarget with 'sloppy' target
	// Scroll to the cursor's scene position
	g_CameraMan.SetScrollTarget(m_CursorPos, 0.3, g_ActivityMan.GetActivity()->ScreenOfPlayer(m_pController->GetPlayer()));
}

//////////////////////////////////////////////////////////////////////////////////////////
// Virtual Method:  Draw
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Draws the menu

void AreaEditorGUI::Draw(BITMAP* pTargetBitmap, const Vector& targetPos) const {
	// Done or can't, so don't draw the UI
	if (!m_pCurrentArea || m_EditorGUIMode == DONEEDITING)
		return;

	// List to capture scene-wrapped boxes
	std::list<Box> wrappedBoxes;

	// First draw all the objects placed in the scene by the Scene Editor
	const std::list<SceneObject*>* pSceneObjectList = g_SceneMan.GetScene()->GetPlacedObjects(Scene::PLACEONLOAD);
	if (m_FullFeatured) {
		// Draw all already placed Objects, and the currently held one in the order it is about to be placed in the scene
		int i = 0;
		for (std::list<SceneObject*>::const_iterator itr = pSceneObjectList->begin(); itr != pSceneObjectList->end(); ++itr, ++i) {
			(*itr)->Draw(pTargetBitmap, targetPos);
			// Draw basic HUD if an actor
			Actor* pActor = dynamic_cast<Actor*>(*itr);
			if (pActor)
				pActor->DrawHUD(pTargetBitmap, targetPos);
		}
	}

	// Draw the Box:es defined for the currently selected Area
	Vector adjCorner;
	const std::vector<Box>* pBoxList = &(m_pCurrentArea->m_BoxList);
	if (m_FullFeatured) {
		// Set the drawin mode to be transparent and use the
		//        g_FrameMan.SetTransTableFromPreset(m_BlinkTimer.AlternateReal(333) || m_EditorGUIMode == PLACINGOBJECT ? TransparencyPreset::LessTrans : TransparencyPreset::HalfTrans);
		g_FrameMan.SetTransTableFromPreset(TransparencyPreset::MoreTrans);
		drawing_mode(DRAW_MODE_TRANS, 0, 0, 0);

		// Draw all already placed Box:es, and the currently edited one
		for (std::vector<Box>::const_iterator bItr = pBoxList->begin(); bItr != pBoxList->end(); ++bItr) {
			// Handle wrapped boxes properly
			wrappedBoxes.clear();
			g_SceneMan.WrapBox(*bItr, wrappedBoxes);

			// Iterate through the wrapped boxes - will only be one if there's no wrapping
			for (std::list<Box>::iterator wItr = wrappedBoxes.begin(); wItr != wrappedBoxes.end(); ++wItr) {
				// Draw the rectangle of each Box, adjusted for the offet of the target bitmap in the scene
				adjCorner = (*wItr).GetCorner() - targetPos;
				rectfill(pTargetBitmap, adjCorner.m_X, adjCorner.m_Y, adjCorner.m_X + (*wItr).GetWidth(), adjCorner.m_Y + (*wItr).GetHeight(), g_RedColor);
			}
		}
	}

	// Draw the currently edited box outline in glowing yellow
	drawing_mode(DRAW_MODE_SOLID, 0, 0, 0);
	if (!m_EditedBox.IsEmpty()) {
		// Handle wrapped boxes properly
		wrappedBoxes.clear();
		g_SceneMan.WrapBox(m_EditedBox, wrappedBoxes);

		// Iterate through the wrapped boxes - will only be one if there's no wrapping
		for (std::list<Box>::iterator wItr = wrappedBoxes.begin(); wItr != wrappedBoxes.end(); ++wItr) {
			adjCorner = (*wItr).GetCorner() - targetPos;
			// Special 'X' drawing when deleting
			if (m_EditorGUIMode == DELETINGBOX) {
				line(pTargetBitmap, adjCorner.m_X, adjCorner.m_Y, adjCorner.m_X + (*wItr).GetWidth(), adjCorner.m_Y + (*wItr).GetHeight(), g_YellowGlowColor);
				line(pTargetBitmap, adjCorner.m_X, adjCorner.m_Y + (*wItr).GetHeight(), adjCorner.m_X + (*wItr).GetWidth(), adjCorner.m_Y, g_YellowGlowColor);
			} else
				rect(pTargetBitmap, adjCorner.m_X, adjCorner.m_Y, adjCorner.m_X + (*wItr).GetWidth(), adjCorner.m_Y + (*wItr).GetHeight(), g_YellowGlowColor);
		}
	}

	// Set drawing mode back to solid
	drawing_mode(DRAW_MODE_SOLID, 0, 0, 0);

	// Draw picking Area crosshairs
	Vector center = m_CursorPos - targetPos;
	putpixel(pTargetBitmap, center.m_X, center.m_Y, g_YellowGlowColor);
	hline(pTargetBitmap, center.m_X - 5, center.m_Y, center.m_X - 2, g_YellowGlowColor);
	hline(pTargetBitmap, center.m_X + 5, center.m_Y, center.m_X + 2, g_YellowGlowColor);
	vline(pTargetBitmap, center.m_X, center.m_Y - 5, center.m_Y - 2, g_YellowGlowColor);
	vline(pTargetBitmap, center.m_X, center.m_Y + 5, center.m_Y + 2, g_YellowGlowColor);

	m_pPicker->Draw(pTargetBitmap);

	// Draw the pie menu
	m_PieMenu->Draw(pTargetBitmap, targetPos);
}