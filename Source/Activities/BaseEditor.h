#pragma once

/// Header file for the BaseEditor class.
/// @author Daniel Tabar
/// data@datarealms.com
/// http://www.datarealms.com
/// Inclusions of header files
#include "RTETools.h"
#include "EditorActivity.h"

namespace RTE {

	class SceneEditorGUI;
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

	/// Activity for editing designs only for the bases of each scene in a
	/// Campaign metagame. Doesn't need all the document-model dlg boxes.
	class BaseEditor : public Activity {

		/// Public member variable, method and friend function declarations
	public:
		// Concrete allocation and cloning definitions
		EntityAllocation(BaseEditor);
		SerializableOverrideMethods;
		ClassInfoGetters;

		/// Constructor method used to instantiate a BaseEditor object in system
		/// memory. Create() should be called before using the object.
		BaseEditor();

		/// Destructor method used to clean up a BaseEditor object before deletion
		/// from system memory.
		~BaseEditor() override;

		/// Makes the BaseEditor object ready for use.
		/// @return An error return value signaling sucess or any particular failure.
		/// Anything below 0 is an error signal.
		int Create() override;

		/// Creates a BaseEditor to be identical to another, by deep copy.
		/// @param reference A reference to the BaseEditor to deep copy.
		/// @return An error return value signaling sucess or any particular failure.
		/// Anything below 0 is an error signal.
		int Create(const BaseEditor& reference);

		/// Resets the entire BaseEditor, including its inherited members, to their
		/// default settings or values.
		void Reset() override {
			Clear();
			Activity::Reset();
		}

		/// Destroys and resets (through Clear()) the BaseEditor object.
		/// @param notInherited Whether to only destroy the members defined in this derived class, or (default: false)
		/// to destroy all inherited members also.
		void Destroy(bool notInherited = false) override;

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
		/// Saves the current scene to an appropriate ini file, and asks user if
		/// they want to overwrite first if scene of this name exists.
		/// @param saveAsName The name of the new scene to be saved.
		/// @param forceOverwrite Whetehr to force any existing Scene of that name to be overwritten if (default: false)
		/// it already exists.
		/// @return Whether actually managed to save. Will return false both if a scene
		/// of this name already exists, or if other error.
		bool SaveScene(std::string saveAsName, bool forceOverwrite = false);

		// Member variables
		static Entity::ClassInfo m_sClass;

		// The editor GUI
		SceneEditorGUI* m_pEditorGUI;

		// Dirty Scene?
		bool m_NeedSave;

		/// Private member variable and method declarations
	private:
		/// Clears all the member variables of this Activity, effectively
		/// resetting the members of this abstraction level only.
		void Clear();
	};

} // namespace RTE
