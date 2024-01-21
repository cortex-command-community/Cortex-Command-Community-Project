#pragma once

/// Header file for the GibEditor class.
/// @author Daniel Tabar
/// data@datarealms.com
/// http://www.datarealms.com
/// Inclusions of header files
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

	/// Activity for editing gib placement within MOSRotating:s.
	class GibEditor : public EditorActivity {

		/// Public member variable, method and friend function declarations
	public:
		// Concrete allocation and cloning definitions
		EntityAllocation(GibEditor);
		SerializableOverrideMethods;
		ClassInfoGetters;

		/// Constructor method used to instantiate a GibEditor object in system
		/// memory. Create() should be called before using the object.
		GibEditor();

		/// Destructor method used to clean up a GibEditor object before deletion
		/// from system memory.
		~GibEditor() override;

		/// Makes the GibEditor object ready for use.
		/// @return An error return value signaling sucess or any particular failure.
		/// Anything below 0 is an error signal.
		int Create() override;

		/// Creates a GibEditor to be identical to another, by deep copy.
		/// @param reference A reference to the GibEditor to deep copy.
		/// @return An error return value signaling sucess or any particular failure.
		/// Anything below 0 is an error signal.
		int Create(const GibEditor& reference);

		/// Resets the entire GibEditor, including its inherited members, to their
		/// default settings or values.
		void Reset() override {
			Clear();
			EditorActivity::Reset();
		}

		/// Destroys and resets (through Clear()) the GibEditor object.
		/// @param notInherited Whether to only destroy the members defined in this derived class, or (default: false)
		/// to destroy all inherited members also.
		void Destroy(bool notInherited = false) override;

		/// Sets the current mode of this editor.
		/// @param newMode The new mode to set to, see the EditorGUIMode enum.
		void SetEditorMode(EditorActivity::EditorMode newMode) { m_EditorMode = newMode; }

		/// Gets the current mode of this editor.
		/// @return The current mode this is set to; see the EditorGUIMode enum.
		EditorActivity::EditorMode GetEditorMode() const { return m_EditorMode; }

		/// Officially starts the game accroding to parameters previously set.
		/// @return An error return value signaling sucess or any particular failure.
		/// Anything below 0 is an error signal.
		int Start() override;

		/// Pauses and unpauses the game.
		/// @param pause Whether to pause the game or not. (default: true)
		void SetPaused(bool pause = true) override;

		/// Forces the current game's end.
		void End() override;

		/// Updates the state of this ActivityMan. Supposed to be done every frame
		/// before drawing.
		void Update() override;

		/// Draws the currently active GUI of a screen to a BITMAP of choice.
		/// @param pTargetBitmap A pointer to a screen-sized BITMAP to draw on.
		/// @param targetPos The absolute position of the target bitmap's upper left corner in the scene. (default: Vector())
		/// @param which Which screen's GUI to draw onto the bitmap. (default: 0)
		void DrawGUI(BITMAP* pTargetBitmap, const Vector& targetPos = Vector(), int which = 0) override;

		/// Draws this ActivityMan's current graphical representation to a
		/// BITMAP of choice. This includes all game-related graphics.
		/// @param pTargetBitmap A pointer to a BITMAP to draw on. OWNERSHIP IS NOT TRANSFERRED!
		/// @param targetPos The absolute position of the target bitmap's upper left corner in the scene. (default: Vector())
		void Draw(BITMAP* pTargetBitmap, const Vector& targetPos = Vector()) override;

		/// Protected member variable and method declarations
	protected:
		/// Saves the current object to an appropriate ini file, and asks user if they want to overwrite first if object of this name exists.
		/// @param saveAsName The name of the new object to be saved.
		/// @param forceOverwrite Whether to force any existing Object of that name to be overwritten if it already exists.
		/// @return Whether actually managed to save. Will return false both if an object of this name already exists (and not overwriting), or if other error.
		bool SaveObject(const std::string& saveAsName, bool forceOverwrite = false);

		/// Replaces all the gibs owned by the passed in MOSR with the ones being
		/// edited that represent the new gibbing setup.
		/// @param pEditedObject The MOSRotating to replace the gibs of. Ownership is NOT trnasferred!
		void StuffEditedGibs(MOSRotating* pEditedObject);

		/// Updates the New dialog box, populates its lists etc.
		void UpdateNewDialog() override;

		/// Updates the Load dialog box, populates its lists etc.
		void UpdateLoadDialog() override;

		/// Updates the Save dialog box, populates its lists etc.
		void UpdateSaveDialog() override;

		/// Updates the Save Changes dialog box, populates its lists etc.
		void UpdateChangesDialog() override;

		/// Updates the Overwrite dialog box, populates its lists etc.
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

		/// Private member variable and method declarations
	private:
		/// Clears all the layers of the testing area terrain so nothing that somehow settled lingers between edited object changes and testing phases.
		void ClearTestArea() const;

		/// Clears all the member variables of this Activity, effectively
		/// resetting the members of this abstraction level only.
		void Clear();
	};

} // namespace RTE
