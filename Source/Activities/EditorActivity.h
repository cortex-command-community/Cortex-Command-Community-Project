#pragma once

/// Header file for the EditorActivity class.
/// @author Daniel Tabar
/// data@datarealms.com
/// http://www.datarealms.com
/// Inclusions of header files
#include "GUISound.h"
#include "RTETools.h"
#include "ActivityMan.h"
#include "PostProcessMan.h"
#include "PieMenu.h"

namespace RTE {

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

	/// Activity for editing things; owns and manages all the dialog boxes
	/// etc for docuemnt model, ie new, load, save, etc.
	class EditorActivity : public Activity {

		/// Public member variable, method and friend function declarations
	public:
		SerializableOverrideMethods;
		ClassInfoGetters;

		// Different modes of this editor
		enum EditorMode {
			NEWDIALOG = 0,
			LOADDIALOG,
			SAVEDIALOG,
			CHANGESDIALOG,
			OVERWRITEDIALOG,
			EDITINGOBJECT,
			TESTINGOBJECT,
			EDITORMODECOUNT
		};

		/// Constructor method used to instantiate a EditorActivity object in system
		/// memory. Create() should be called before using the object.
		EditorActivity();

		/// Destructor method used to clean up a EditorActivity object before deletion
		/// from system memory.
		~EditorActivity() override;

		/// Makes the EditorActivity object ready for use.
		/// @return An error return value signaling sucess or any particular failure.
		/// Anything below 0 is an error signal.
		int Create() override;

		/// Creates a EditorActivity to be identical to another, by deep copy.
		/// @param reference A reference to the EditorActivity to deep copy.
		/// @return An error return value signaling sucess or any particular failure.
		/// Anything below 0 is an error signal.
		int Create(const EditorActivity& reference);

		/// Resets the entire EditorActivity, including its inherited members, to their
		/// default settings or values.
		void Reset() override {
			Clear();
			Activity::Reset();
		}

		/// Destroys and resets (through Clear()) the EditorActivity object.
		/// @param notInherited Whether to only destroy the members defined in this derived class, or (default: false)
		/// to destroy all inherited members also.
		void Destroy(bool notInherited = false) override;

		/// Sets the current mode of this editor.
		/// @param newMode The new mode to set to, see the EditorGUIMode enum.
		void SetEditorMode(EditorMode newMode) { m_EditorMode = newMode; }

		/// Gets the current mode of this editor.
		/// @return The current mode this is set to; see the EditorGUIMode enum.
		EditorMode GetEditorMode() const { return m_EditorMode; }

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
		/// Updates the New dialog box, populates its lists etc.
		virtual void UpdateNewDialog() {}

		/// Updates the Load dialog box, populates its lists etc.
		virtual void UpdateLoadDialog() {}

		/// Updates the Save dialog box, populates its lists etc.
		virtual void UpdateSaveDialog() {}

		/// Updates the Save Changes dialog box, populates its lists etc.
		virtual void UpdateChangesDialog() {}

		/// Updates the Overwrite dialog box, populates its lists etc.
		virtual void UpdateOverwriteDialog() {}

		// Member variables
		static Entity::ClassInfo m_sClass;
		// The current mode of the whole editor. See EditorGUIMode enum.
		EditorMode m_EditorMode;
		// Indicates the previous mode before a confirm dialog box was shown
		EditorMode m_PreviousMode;
		// Whether the mode has been changed
		bool m_ModeChange;
		// The DataModule space that we are editing within; can only place objects defined in the official modules plus this one
		int m_ModuleSpaceID;
		// Whether the edited object has been changed since last save
		bool m_NeedSave;
		// Whether this thing has ever been saved (or loaded). Will be true of new until it is saved
		bool m_HasEverBeenSaved;

		std::unique_ptr<PieMenu> m_PieMenu; //!< The pie menu this EditorActivity should use, if any.

		// GUI Screen for use by the GUI dialog boxes. Owned
		GUIScreen* m_pGUIScreen;
		// Input controller for he dialog box gui.  Owned
		GUIInput* m_pGUIInput;
		// The control manager which holds all the gui elements for the dialog boxes.  Owned
		GUIControlManager* m_pGUIController;

		// New Dialog box
		GUICollectionBox* m_pNewDialogBox;
		// The combobox which lists all the DataModules that the new Entity can belong to
		GUIComboBox* m_pNewModuleCombo;
		// The button for asking for new Entity
		GUIButton* m_pNewButton;
		// The button for canceling new Entity dialog
		GUIButton* m_pNewCancel;

		// Load Dialog box
		GUICollectionBox* m_pLoadDialogBox;
		// The combobox which lists all the Entities that can be loaded
		GUIComboBox* m_pLoadNameCombo;
		// The button for going to new dialog instead
		GUIButton* m_pLoadToNewButton;
		// The button for confirming Entity load
		GUIButton* m_pLoadButton;
		// The button for canceling load dialog
		GUIButton* m_pLoadCancel;

		// Save Dialog box
		GUICollectionBox* m_pSaveDialogBox;
		// Textbox for entering the name of the thing to save.
		GUITextBox* m_pSaveNameBox;
		// The label which shows which DataModule this Scene is set to be saved to
		GUILabel* m_pSaveModuleLabel;
		// The button for confirming save
		GUIButton* m_pSaveButton;
		// The button for canceling save dialog
		GUIButton* m_pSaveCancel;

		// Changes Dialog box
		GUICollectionBox* m_pChangesDialogBox;
		// The label for showing where it'll be saved
		GUILabel* m_pChangesNameLabel;
		// The button for confirming save changes
		GUIButton* m_pChangesYesButton;
		// The button for No resposnes to save changes
		GUIButton* m_pChangesNoButton;

		// Overwrite Dialog box
		GUICollectionBox* m_pOverwriteDialogBox;
		// The label for showing what is about to be overwritten
		GUILabel* m_pOverwriteNameLabel;
		// The button for confirming overwrite existing
		GUIButton* m_pOverwriteYesButton;
		// The button for No resposnes to overwrite
		GUIButton* m_pOverwriteNoButton;

		/// Private member variable and method declarations
	private:
		/// Clears all the member variables of this Activity, effectively
		/// resetting the members of this abstraction level only.
		void Clear();
	};

} // namespace RTE
