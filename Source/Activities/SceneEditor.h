#pragma once

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

	/// Activity for editing scenes.
	/// 9/17/2007 Spliced out and made to derive from EditorActivty
	class SceneEditor : public EditorActivity {

		/// Public member variable, method and friend function declarations
	public:
		// Concrete allocation and cloning definitions
		EntityAllocation(SceneEditor);
		SerializableOverrideMethods;
		ClassInfoGetters;

		/// Constructor method used to instantiate a SceneEditor object in system
		/// memory. Create() should be called before using the object.
		SceneEditor();

		/// Destructor method used to clean up a SceneEditor object before deletion
		/// from system memory.
		~SceneEditor() override;

		/// Makes the SceneEditor object ready for use.
		/// @return An error return value signaling sucess or any particular failure.
		/// Anything below 0 is an error signal.
		int Create() override;

		/// Creates a SceneEditor to be identical to another, by deep copy.
		/// @param reference A reference to the SceneEditor to deep copy.
		/// @return An error return value signaling sucess or any particular failure.
		/// Anything below 0 is an error signal.
		int Create(const SceneEditor& reference);

		/// Resets the entire SceneEditor, including its inherited members, to their
		/// default settings or values.
		void Reset() override;

		/// Destroys and resets (through Clear()) the SceneEditor object.
		/// @param notInherited Whether to only destroy the members defined in this derived class, or (default: false)
		/// to destroy all inherited members also.
		void Destroy(bool notInherited = false) override;

		/// Sets the current mode of this editor.
		/// @param newMode The new mode to set to, see the EditorGUIMode enum.
		void SetEditorMode(EditorActivity::EditorMode newMode);

		/// Gets the current mode of this editor.
		/// @return The current mode this is set to; see the EditorGUIMode enum.
		EditorActivity::EditorMode GetEditorMode() const;

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
		/// Saves the current Scene to an appropriate ini file, and asks user if they want to overwrite first if scene of this name exists.
		/// @param saveAsName The name of the new Scene to be saved.
		/// @param forceOverwrite Whether to force any existing Scene of that name to be overwritten if it already exists.
		/// @return Whether actually managed to save. Will return false both if a scene of this name already exists, or if other error.
		bool SaveScene(const std::string& saveAsName, bool forceOverwrite = false);

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

		// The editor GUI
		SceneEditorGUI* m_pEditorGUI;

		// The combobox which lists all the Terrain:s that can be loaded for a new scene
		GUIComboBox* m_pNewTerrainCombo;
		// The combobox which lists all the background SceneLayer:s that can be loaded for a new scene, near
		GUIComboBox* m_pNewBG1Combo;
		// The combobox which lists all the background SceneLayer:s that can be loaded for a new scene, mid
		GUIComboBox* m_pNewBG2Combo;
		// The combobox which lists all the background SceneLayer:s that can be loaded for a new scene, far/sky
		GUIComboBox* m_pNewBG3Combo;

		/// Private member variable and method declarations
	private:
		/// Clears all the member variables of this Activity, effectively
		/// resetting the members of this abstraction level only.
		void Clear();
	};

} // namespace RTE
