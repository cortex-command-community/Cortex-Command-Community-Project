#pragma once

#include "ScenarioActivityConfigGUI.h"
#include "MOSParticle.h"

namespace RTE {

	class Activity;
	class Scene;
	class AllegroScreen;
	class GUIInputWrapper;
	class AllegroBitmap;
	class GUIControlManager;
	class GUICollectionBox;
	class GUIComboBox;
	class GUIButton;
	class GUILabel;

	/// Handling for the scenario menu screen composition and sub-menu interaction.
	class ScenarioGUI {

	public:
		/// Enumeration for the results of the ScenarioGUI input and event update.
		enum class ScenarioMenuUpdateResult {
			NoEvent,
			BackToMain,
			ActivityResumed,
			ActivityStarted
		};

#pragma region Creation
		/// Constructor method used to instantiate this ScenarioGUI object in system memory.
		/// @param guiScreen Pointer to a GUIScreen interface that will be used by this ScenarioGUI's GUIControlManager. Ownership is NOT transferred!
		/// @param guiInput Pointer to a GUIInput interface that will be used by this ScenarioGUI's GUIControlManager. Ownership is NOT transferred!
		ScenarioGUI(AllegroScreen* guiScreen, GUIInputWrapper* guiInput) {
			Clear();
			Create(guiScreen, guiInput);
		}

		/// Makes the ScenarioGUI object ready for use.
		/// @param guiScreen Pointer to a GUIScreen interface that will be used by this ScenarioGUI's GUIControlManager. Ownership is NOT transferred!
		/// @param guiInput Pointer to a GUIInput interface that will be used by this ScenarioGUI's GUIControlManager. Ownership is NOT transferred!
		void Create(AllegroScreen* guiScreen, GUIInputWrapper* guiInput);
#pragma endregion

#pragma region Setters
		/// Enables the GUI elements for the menu, sets the planet coordinates on the screen so the menu can properly overlay it, and fetches the Scenes and Activities lists.
		/// @param center The absolute screen coordinates of the planet's center.
		/// @param radius The radius, in screen pixel units, of the planet.
		void SetEnabled(const Vector& center, float radius);
#pragma endregion

#pragma region Concrete Methods
		/// Updates the ScenarioGUI state.
		/// @return The result of the ScenarioGUI input and event update. See ScenarioMenuUpdateResult enumeration.
		ScenarioMenuUpdateResult Update();

		/// Draws the ScenarioGUI to the screen.
		void Draw() const;
#pragma endregion

	private:
		int m_RootBoxMaxWidth; //!< The maximum width the root CollectionBox that holds all this menu's GUI elements. This is to constrain this menu to the primary window's display (left-most) while in multi-display fullscreen, otherwise positioning can get stupid.

		std::unique_ptr<GUIControlManager> m_GUIControlManager; //!< The GUIControlManager which owns all the GUIControls of the ScenarioGUI.
		ScenarioMenuUpdateResult m_UpdateResult; //!< The result of the ScenarioGUI update. See ScenarioMenuUpdateResult enumeration.

		std::map<Activity*, std::vector<Scene*>> m_ScenarioActivities; //!< The map of Activities and the Scenes compatible with each, neither of which are owned here.
		const Activity* m_SelectedActivity; //!< The currently selected Activity. Not owned.

		std::vector<Scene*>* m_ActivityScenes; //!< Pointer to the current set of Scenes being displayed. Not owned, and neither are the Scenes.
		Scene* m_SelectedScene; //!< The scene preset currently selected. Not owned.
		Scene* m_HoveredScene; //!< The scene preset currently hovered. Not owned.

		Vector m_PlanetCenter; //!< The absolute screen position of the planet center.
		float m_PlanetRadius; //!< The screen radius of the planet.
		std::vector<Vector> m_LineToSitePoints; //!< Collection of points that form lines from a screen point to the selected site point.

		GUICollectionBox* m_DraggedBox; //!< Currently dragged GUI box.
		Vector m_PrevMousePos; //!< Previous position of the mouse to calculate dragging.

		Timer m_BlinkTimer; //!< Timer for blinking the resume and config start buttons.
		Timer m_ScenePreviewAnimTimer; //!< Timer for animating the Scene preview image.

		MOSParticle m_DefaultScenePreview; //!< MOSParticle that acts as the default Scene preview image, to avoid having to animate manually.
		bool m_DrawDefaultScenePreview; //!< Whether the default Scene preview should be drawn or not.

		std::unique_ptr<ScenarioActivityConfigGUI> m_ActivityConfigBox; //!< The Activity configuration box.

		/// GUI elements that compose the scenario menu screen.
		GUICollectionBox* m_RootBox;
		GUICollectionBox* m_ActivityConfigBoxRootBox;
		GUIButton* m_BackToMainButton;
		GUIButton* m_ResumeButton;
		GUICollectionBox* m_ActivityInfoBox;
		GUIComboBox* m_ActivitySelectComboBox;
		GUILabel* m_ActivityDescriptionLabel;
		GUICollectionBox* m_SceneInfoBox;
		GUIButton* m_SceneBoxCloseButton;
		GUILabel* m_SceneNameLabel;
		GUILabel* m_SceneDescriptionLabel;
		GUICollectionBox* m_ScenePreviewImageBox;
		std::unique_ptr<AllegroBitmap> m_ScenePreviewBitmap;
		GUIButton* m_StartActivityConfigButton;
		GUILabel* m_SitePointNameLabel;

#pragma region Create Breakdown
		/// Creates all the elements that compose the Activity info box.
		void CreateActivityInfoBox();

		/// Creates all the elements that compose the Scene info box.
		void CreateSceneInfoBox();
#pragma endregion

#pragma region Scenario Menu Handling
		/// Sets the CollectionBox that is currently being dragged, if applicable.
		/// @param mouseX Mouse X position.
		/// @param mouseY Mouse Y position.
		void SetDraggedBox(int mouseX, int mouseY);

		/// Sets the selected Activity, refreshes the compatible Scenes on the planet and updates the Activity info box appropriately.
		/// @param newSelectedActivity The new selected Activity.
		void SetSelectedActivity(const Activity* newSelectedActivity);

		/// Sets the currently selected Scene and updates the Scene info box appropriately.
		/// @param newSelectedScene The new selected Scene.
		void SetSelectedScene(Scene* newSelectedScene);

		/// Moves the CollectionBox that is selected as being dragged, if any.
		/// @param mouseX Mouse X position to calculate box position.
		/// @param mouseY Mouse Y position to calculate box position.
		void DragBox(int mouseX, int mouseY);

		/// Fetches all the available Scenes and Activity presets from PresetMan.
		void FetchActivitiesAndScenesLists();

		/// Adjusts the positions of the site points on the planet if they don't fit the screen or overlap.
		/// @param sceneList Vector of Scenes to adjust positions for.
		void AdjustSitePointOffsetsOnPlanet(const std::vector<Scene*>& sceneList) const;

		/// Calculates how to draw lines from the Scene info box to the selected site point on the planet.
		void CalculateLinesToSitePoint();
#pragma endregion

#pragma region Update Breakdown
		/// Displays the site name label if the mouse is over a site point.
		/// @param mouseX Mouse X position.
		/// @param mouseY Mouse Y position.
		void UpdateHoveredSitePointLabel(int mouseX, int mouseY);

		/// Handles the player interaction with the ScenarioGUI GUI elements.
		/// @param mouseX Mouse X position.
		/// @param mouseY Mouse Y position.
		void HandleInputEvents(int mouseX, int mouseY);
#pragma endregion

#pragma region Draw Breakdown
		/// Draws the site points on top of the planet.
		/// @param drawBitmap The bitmap to draw on.
		void DrawSitePoints(BITMAP* drawBitmap) const;

		/// Draws fancy thick flickering lines from the Scene info box to the selected scene point on the planet.
		/// @param drawBitmap The bitmap to draw to.
		void DrawLinesToSitePoint(BITMAP* drawBitmap) const;
#pragma endregion

		/// Clears all the member variables of this ScenarioGUI, effectively resetting the members of this object.
		void Clear();

		// Disallow the use of some implicit methods.
		ScenarioGUI(const ScenarioGUI& reference) = delete;
		ScenarioGUI& operator=(const ScenarioGUI& rhs) = delete;
	};
} // namespace RTE
