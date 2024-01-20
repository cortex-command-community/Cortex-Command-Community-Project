#ifndef _RTEGIBEDITOR_
#define _RTEGIBEDITOR_

//////////////////////////////////////////////////////////////////////////////////////////
// File:            GibEditor.h
//////////////////////////////////////////////////////////////////////////////////////////
// Description:     Header file for the GibEditor class.
// Project:         Retro Terrain Engine
// Author(s):       Daniel Tabar
//                  data@datarealms.com
//                  http://www.datarealms.com

//////////////////////////////////////////////////////////////////////////////////////////
// Inclusions of header files

#include "RTETools.h"
#include "EditorActivity.h"

namespace RTE {

	class MOSRotating;
	class GibEditorGUI;
	class GUIScreen;
	class GUIInput;
	class GUIControlManager;
	class GUICollectionBox;
	class GUITab;
	class GUIListBox;
	class GUITextBox;
	class GUIButton;
	class GUILabel;
	class GUIComboBox;

	//////////////////////////////////////////////////////////////////////////////////////////
	// Class:           GibEditor
	//////////////////////////////////////////////////////////////////////////////////////////
	// Description:     Activity for editing gib placement within MOSRotating:s.
	// Parent(s):       EditorActivity.
	// Class history:   9/17/2007 GibEditor Created.

	class GibEditor : public EditorActivity {

		//////////////////////////////////////////////////////////////////////////////////////////
		// Public member variable, method and friend function declarations

	public:
		// Concrete allocation and cloning definitions
		EntityAllocation(GibEditor);
		SerializableOverrideMethods;
		ClassInfoGetters;

		//////////////////////////////////////////////////////////////////////////////////////////
		// Constructor:     GibEditor
		//////////////////////////////////////////////////////////////////////////////////////////
		// Description:     Constructor method used to instantiate a GibEditor object in system
		//                  memory. Create() should be called before using the object.
		// Arguments:       None.

		GibEditor() { Clear(); }

		//////////////////////////////////////////////////////////////////////////////////////////
		// Destructor:      ~GibEditor
		//////////////////////////////////////////////////////////////////////////////////////////
		// Description:     Destructor method used to clean up a GibEditor object before deletion
		//                  from system memory.
		// Arguments:       None.

		~GibEditor() override { Destroy(true); }

		//////////////////////////////////////////////////////////////////////////////////////////
		// Virtual method:  Create
		//////////////////////////////////////////////////////////////////////////////////////////
		// Description:     Makes the GibEditor object ready for use.
		// Arguments:       None.
		// Return value:    An error return value signaling sucess or any particular failure.
		//                  Anything below 0 is an error signal.

		int Create() override;

		//////////////////////////////////////////////////////////////////////////////////////////
		// Virtual method:  Create
		//////////////////////////////////////////////////////////////////////////////////////////
		// Description:     Creates a GibEditor to be identical to another, by deep copy.
		// Arguments:       A reference to the GibEditor to deep copy.
		// Return value:    An error return value signaling sucess or any particular failure.
		//                  Anything below 0 is an error signal.

		int Create(const GibEditor& reference);

		//////////////////////////////////////////////////////////////////////////////////////////
		// Virtual method:  Reset
		//////////////////////////////////////////////////////////////////////////////////////////
		// Description:     Resets the entire GibEditor, including its inherited members, to their
		//                  default settings or values.
		// Arguments:       None.
		// Return value:    None.

		void Reset() override {
			Clear();
			EditorActivity::Reset();
		}

		//////////////////////////////////////////////////////////////////////////////////////////
		// Virtual method:  Destroy
		//////////////////////////////////////////////////////////////////////////////////////////
		// Description:     Destroys and resets (through Clear()) the GibEditor object.
		// Arguments:       Whether to only destroy the members defined in this derived class, or
		//                  to destroy all inherited members also.
		// Return value:    None.

		void Destroy(bool notInherited = false) override;

		//////////////////////////////////////////////////////////////////////////////////////////
		// Method:          SetEditorMode
		//////////////////////////////////////////////////////////////////////////////////////////
		// Description:     Sets the current mode of this editor.
		// Arguments:       The new mode to set to, see the EditorGUIMode enum.
		// Return value:    None.

		void SetEditorMode(EditorActivity::EditorMode newMode) { m_EditorMode = newMode; }

		//////////////////////////////////////////////////////////////////////////////////////////
		// Method:          GetEditorMode
		//////////////////////////////////////////////////////////////////////////////////////////
		// Description:     Gets the current mode of this editor.
		// Arguments:       None.
		// Return value:    The current mode this is set to; see the EditorGUIMode enum.

		EditorActivity::EditorMode GetEditorMode() const { return m_EditorMode; }

		//////////////////////////////////////////////////////////////////////////////////////////
		// Virtual method:  Start
		//////////////////////////////////////////////////////////////////////////////////////////
		// Description:     Officially starts the game accroding to parameters previously set.
		// Arguments:       None.
		// Return value:    An error return value signaling sucess or any particular failure.
		//                  Anything below 0 is an error signal.

		int Start() override;

		//////////////////////////////////////////////////////////////////////////////////////////
		// Virtual method:  Pause
		//////////////////////////////////////////////////////////////////////////////////////////
		// Description:     Pauses and unpauses the game.
		// Arguments:       Whether to pause the game or not.
		// Return value:    None.

		void SetPaused(bool pause = true) override;

		//////////////////////////////////////////////////////////////////////////////////////////
		// Virtual method:  End
		//////////////////////////////////////////////////////////////////////////////////////////
		// Description:     Forces the current game's end.
		// Arguments:       None.
		// Return value:    None.

		void End() override;

		//////////////////////////////////////////////////////////////////////////////////////////
		// Virtual method:  Update
		//////////////////////////////////////////////////////////////////////////////////////////
		// Description:     Updates the state of this ActivityMan. Supposed to be done every frame
		//                  before drawing.
		// Arguments:       None.
		// Return value:    None.

		void Update() override;

		//////////////////////////////////////////////////////////////////////////////////////////
		// Virtual method:  DrawGUI
		//////////////////////////////////////////////////////////////////////////////////////////
		// Description:     Draws the currently active GUI of a screen to a BITMAP of choice.
		// Arguments:       A pointer to a screen-sized BITMAP to draw on.
		//                  The absolute position of the target bitmap's upper left corner in the scene.
		//                  Which screen's GUI to draw onto the bitmap.
		// Return value:    None.

		void DrawGUI(BITMAP* pTargetBitmap, const Vector& targetPos = Vector(), int which = 0) override;

		//////////////////////////////////////////////////////////////////////////////////////////
		// Virtual method:  Draw
		//////////////////////////////////////////////////////////////////////////////////////////
		// Description:     Draws this ActivityMan's current graphical representation to a
		//                  BITMAP of choice. This includes all game-related graphics.
		// Arguments:       A pointer to a BITMAP to draw on. OWNERSHIP IS NOT TRANSFERRED!
		//                  The absolute position of the target bitmap's upper left corner in the scene.
		// Return value:    None.

		void Draw(BITMAP* pTargetBitmap, const Vector& targetPos = Vector()) override;

		//////////////////////////////////////////////////////////////////////////////////////////
		// Protected member variable and method declarations

	protected:
		/// <summary>
		/// Saves the current object to an appropriate ini file, and asks user if they want to overwrite first if object of this name exists.
		/// </summary>
		/// <param name="saveAsName">The name of the new object to be saved.</param>
		/// <param name="forceOverwrite">Whether to force any existing Object of that name to be overwritten if it already exists.</param>
		/// <returns>Whether actually managed to save. Will return false both if an object of this name already exists (and not overwriting), or if other error.</returns>
		bool SaveObject(const std::string& saveAsName, bool forceOverwrite = false);

		//////////////////////////////////////////////////////////////////////////////////////////
		// Method:          StuffEditedGibs
		//////////////////////////////////////////////////////////////////////////////////////////
		// Description:     Replaces all the gibs owned by the passed in MOSR with the ones being
		//                  edited that represent the new gibbing setup.
		// Arguments:       The MOSRotating to replace the gibs of. Ownership is NOT trnasferred!
		// Return value:    None.

		void StuffEditedGibs(MOSRotating* pEditedObject);

		//////////////////////////////////////////////////////////////////////////////////////////
		// Virutal method:  UpdateNewDialog
		//////////////////////////////////////////////////////////////////////////////////////////
		// Description:     Updates the New dialog box, populates its lists etc.
		// Arguments:       None.
		// Return value:    None.

		void UpdateNewDialog() override;

		//////////////////////////////////////////////////////////////////////////////////////////
		// Virutal method:  UpdateLoadDialog
		//////////////////////////////////////////////////////////////////////////////////////////
		// Description:     Updates the Load dialog box, populates its lists etc.
		// Arguments:       None.
		// Return value:    None.

		void UpdateLoadDialog() override;

		//////////////////////////////////////////////////////////////////////////////////////////
		// Virutal method:  UpdateSaveDialog
		//////////////////////////////////////////////////////////////////////////////////////////
		// Description:     Updates the Save dialog box, populates its lists etc.
		// Arguments:       None.
		// Return value:    None.

		void UpdateSaveDialog() override;

		//////////////////////////////////////////////////////////////////////////////////////////
		// Virutal method:  UpdateChangesDialog
		//////////////////////////////////////////////////////////////////////////////////////////
		// Description:     Updates the Save Changes dialog box, populates its lists etc.
		// Arguments:       None.
		// Return value:    None.

		void UpdateChangesDialog() override;

		//////////////////////////////////////////////////////////////////////////////////////////
		// Virutal method:  UpdateOverwriteDialog
		//////////////////////////////////////////////////////////////////////////////////////////
		// Description:     Updates the Overwrite dialog box, populates its lists etc.
		// Arguments:       None.
		// Return value:    None.

		void UpdateOverwriteDialog() override;

		// Member variables
		static Entity::ClassInfo m_sClass;

		// The loaded MOSR of which we are editing the gibs. Owned by this.
		MOSRotating* m_pEditedObject;
		// The temporary copy of the edited object which will be gibbed in the test mode. Owned by this.
		MOSRotating* m_pTestingObject;
		// How many times tests ahve been run; then the terrain is cleared
		int m_TestCounter;
		// The MOSR which the user has picked to load. Not owned
		const MOSRotating* m_pObjectToLoad;
		// The editor GUI
		GibEditorGUI* m_pEditorGUI;

		//////////////////////////////////////////////////////////////////////////////////////////
		// Private member variable and method declarations

	private:
		/// <summary>
		/// Clears all the layers of the testing area terrain so nothing that somehow settled lingers between edited object changes and testing phases.
		/// </summary>
		void ClearTestArea() const;

		//////////////////////////////////////////////////////////////////////////////////////////
		// Method:          Clear
		//////////////////////////////////////////////////////////////////////////////////////////
		// Description:     Clears all the member variables of this Activity, effectively
		//                  resetting the members of this abstraction level only.
		// Arguments:       None.
		// Return value:    None.

		void Clear();
	};

} // namespace RTE

#endif // File
