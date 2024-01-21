#pragma once

/// Header file for the ActorEditor class.
/// @author Daniel Tabar
/// data@datarealms.com
/// http://www.datarealms.com
/// Inclusions of header files
#include "RTETools.h"
#include "EditorActivity.h"

namespace RTE {

	class Actor;
	class ObjectPickerGUI;
	class PieMenu;
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

	/// Activity for testing and quickly iterating on Actor data definitions.
	class ActorEditor : public EditorActivity {

		/// Public member variable, method and friend function declarations
	public:
		// Concrete allocation and cloning definitions
		EntityAllocation(ActorEditor);
		SerializableOverrideMethods;
		ClassInfoGetters;

		/// Constructor method used to instantiate a ActorEditor object in system
		/// memory. Create() should be called before using the object.
		ActorEditor();

		/// Destructor method used to clean up a ActorEditor object before deletion
		/// from system memory.
		~ActorEditor() override;

		/// Makes the ActorEditor object ready for use.
		/// @return An error return value signaling sucess or any particular failure.
		/// Anything below 0 is an error signal.
		int Create() override;

		/// Creates a ActorEditor to be identical to another, by deep copy.
		/// @param reference A reference to the ActorEditor to deep copy.
		/// @return An error return value signaling sucess or any particular failure.
		/// Anything below 0 is an error signal.
		int Create(const ActorEditor& reference);

		/// Resets the entire ActorEditor, including its inherited members, to their
		/// default settings or values.
		void Reset() override {
			Clear();
			EditorActivity::Reset();
		}

		/// Destroys and resets (through Clear()) the ActorEditor object.
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
		/// Reloads the Actor itself and sets up the pie menu to match its setup.
		/// @param pActorToLoad An Entity Preset of the Actor to load into the editor. OWNERSHIP IS NOT TRANSFERRED!
		/// @return Whether the Actor was loaded successfully from the PresetMan.
		bool LoadActor(const Entity* pActorToLoad);

		/// Reloads the ini with the currently edited Actor's definitions.
		/// @return Whether the data file was successfully read.
		bool ReloadActorData();

		// Member variables
		static Entity::ClassInfo m_sClass;

		// The loaded MOSR of which we are editing. Owned by this.
		Actor* m_pEditedActor;
		// The picker for selecting which object to load
		ObjectPickerGUI* m_pPicker;

		/// Private member variable and method declarations
	private:
		/// Clears all the member variables of this Activity, effectively
		/// resetting the members of this abstraction level only.
		void Clear();
	};

} // namespace RTE
