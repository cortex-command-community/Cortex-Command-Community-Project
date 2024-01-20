#pragma once

#include "Controller.h"

#include "GUI.h"
#include "GUIControlManager.h"
#include "AllegroScreen.h"
#include "GUIInputWrapper.h"

namespace RTE {

	class Controller;
	class GUICollectionBox;
	class GUIListBox;
	class GUILabel;
	class SceneObject;

	/// A GUI for picking object instances for placement to the Scene in various editors.
	class ObjectPickerGUI {

	public:
#pragma region Creation
		/// Constructor method used to instantiate a ObjectPickerGUI object in system memory. Create() should be called before using the object.
		ObjectPickerGUI() { Clear(); }

		/// Makes the ObjectPickerGUI object ready for use.
		/// @param controller A pointer to a Controller which will control this Menu. Ownership is NOT transferred!
		/// @param whichModuleSpace Which DataModule space to be picking from. -1 means pick from all objects loaded in all DataModules.
		/// @param onlyOfType Which lowest common denominator type to be showing.
		/// @return An error return value signaling success or any particular failure. Anything below 0 is an error signal.
		int Create(Controller* controller, int whichModuleSpace = -1, const std::string_view& onlyOfType = "All");
#pragma endregion

#pragma region Destruction
		/// Resets the entire ObjectPickerGUI, including its inherited members, to their default settings or values.
		void Reset() { Clear(); }
#pragma endregion

#pragma region Getters and Setters
		/// Reports whether the menu is at all visible or not.
		/// @return
		bool IsVisible() const { return m_PickerState != PickerState::Disabled; }

		/// Reports whether the menu is enabled or not.
		/// @return
		bool IsEnabled() const { return m_PickerState == PickerState::Enabled || m_PickerState == PickerState::Enabling; }

		/// Enables or disables the menu. This will animate it in and out of view.
		/// @param enable Whether to enable or disable the menu.
		void SetEnabled(bool enable = true);

		/// Sets the controller used by this. The ownership of the controller is NOT transferred!
		/// @param controller The new controller for this menu. Ownership is NOT transferred!
		void SetController(Controller* controller) { m_Controller = controller; }

		/// Sets where on the screen that this GUI is being drawn to. If upper left corner, then 0, 0. This will affect the way the mouse is positioned etc.
		/// @param newPosX The new X position of this entire GUI on the screen.
		/// @param newPosY The new Y position of this entire GUI on the screen.
		void SetPosOnScreen(int newPosX, int newPosY) const { m_GUIControlManager->SetPosOnScreen(newPosX, newPosY); }

		/// Sets which DataModule space to be picking objects from. If -1, then let the player pick from all loaded modules.
		/// @param newModuleSpaceID The ID of the module to let the player pick objects from. All official module objects will always be presented, in addition to the one passed in here.
		void SetModuleSpace(int newModuleSpaceID = -1) {
			if (newModuleSpaceID != m_ModuleSpaceID) {
				m_ModuleSpaceID = newModuleSpaceID;
				UpdateGroupsList();
			}
		}

		/// Sets which DataModule space to be picking objects from. If -1, then let the player pick from all loaded modules.
		/// @param showType The ID of the module to let the player pick objects from. All official module objects will always be presented, in addition to the one passed in here.
		void ShowOnlyType(const std::string_view& showType = "All") {
			m_ShowType = showType;
			UpdateGroupsList();
		}

		/// Sets which DataModule ID should be treated as the native tech of the user of this menu.
		/// This will also apply the DataModule's faction BuyMenu theme skin and background color for visual consistency, if applicable.
		/// @param whichModule The module ID to set as the native one. 0 means everything is native.
		void SetNativeTechModule(int whichModule);

		/// Sets the multiplier of the cost of any foreign Tech items.
		/// @param newMultiplier The scalar multiplier of the costs of foreign Tech items.
		void SetForeignCostMultiplier(float newMultiplier) { m_ForeignCostMult = newMultiplier; }

		/// Selects the specified group name in the groups list and updates the objects list to show the group's objects.
		/// @param groupName The name of the group to select in the picker.
		/// @return Whether the group was found and switched to successfully.
		bool SelectGroupByName(const std::string_view& groupName);
#pragma endregion

#pragma region Object Picking Handling
		/// Gets the next object in the objects list, even if the picker is disabled.
		/// @return The next object in the picker list, looping around if necessary. If the next object is an invalid SceneObject (e.g. a module subgroup) then this will recurse until a valid object is found.
		const SceneObject* GetNextObject() {
			SelectNextOrPrevObject(false);
			const SceneObject* object = GetSelectedObject();
			return object ? object : GetNextObject();
		}

		/// Gets the previous object in the objects list, even if the picker is disabled.
		/// @return The previous object in the picker list, looping around if necessary. If the previous object is an invalid SceneObject (e.g. a module subgroup) then this will recurse until a valid object is found.
		const SceneObject* GetPrevObject() {
			SelectNextOrPrevObject(true);
			const SceneObject* object = GetSelectedObject();
			return object ? object : GetPrevObject();
		}

		/// Reports whether and which object has been picked by the player. There may be an object picked even when the player is not done with the picker, as scrolling through objects (but not mousing over them) picks them.
		/// @return A pointer to the object picked by the player, or nullptr if none was picked. Ownership is NOT transferred!
		const SceneObject* ObjectPicked() const { return m_PickedObject; }

		/// Reports whether the player has finished using the picker, and the final picked object is returned.
		/// @return The object the player picked before they closed the picker, or nullptr if none was picked. Ownership is NOT transferred!
		const SceneObject* DonePicking() const { return (!IsEnabled() && m_PickedObject) ? m_PickedObject : nullptr; }
#pragma endregion

#pragma region Concrete Methods
		/// Updates the state of this ObjectPickerGUI each frame.
		void Update();

		/// Draws the ObjectPickerGUI to the specified BITMAP.
		/// @param drawBitmap The BITMAP to draw on.
		void Draw(BITMAP* drawBitmap) const;
#pragma endregion

	private:
		/// Enumeration for ObjectPicker states when enabling/disabling the ObjectPicker.
		enum class PickerState {
			Enabling,
			Enabled,
			Disabling,
			Disabled
		};

		/// Enumeration for the ObjectPicker columns ListBox focus states.
		enum class PickerFocus {
			GroupList,
			ObjectList
		};

		static BITMAP* s_Cursor; //!< The cursor image shared by all pickers.

		std::unique_ptr<AllegroScreen> m_GUIScreen; //!< The GUIScreen interface that will be used by this ObjectPickerGUI's GUIControlManager.
		std::unique_ptr<GUIInputWrapper> m_GUIInput; //!< The GUIInput interface that will be used by this ObjectPickerGUI's GUIControlManager.
		std::unique_ptr<GUIControlManager> m_GUIControlManager; //!< The control manager which holds all the controls.
		GUICollectionBox* m_ParentBox; //!< Collection box of the picker GUI.
		GUICollectionBox* m_PopupBox; //!< Collection box of the buy popups that contain information about items.
		GUILabel* m_PopupText; //!< Label displaying the item popup description.
		GUIListBox* m_GroupsList; //!< The ListBox which lists all the groups.
		GUIListBox* m_ObjectsList; //!< The ListBox which lists all the objects in the currently selected group.

		Controller* m_Controller; //!< Controller which controls this menu. Not owned.

		PickerState m_PickerState; //!< Visibility state of the object picker.
		PickerFocus m_PickerFocus; //!< The currently focused list in the Picker.
		float m_OpenCloseSpeed; //!< Speed at which the picker appears and disappears.

		int m_ModuleSpaceID; //!< The DataModule ID of the non-official module that this picker should be restricted to, in addition to all the official modules as well. If -1, the picker will be able to pick from ALL loaded DataModules.
		std::string m_ShowType; //!< Only show objects of this type. Empty string or "All" will show objects of all types.
		int m_NativeTechModuleID; //!< The ID of the DataModule that contains the native Tech of the Player using this menu.
		float m_ForeignCostMult; //!< The multiplier of costs of any foreign tech items.

		int m_ShownGroupIndex; //!< Which group in the groups list is currently showing it's objects list.
		int m_SelectedGroupIndex; //!< Which group in the groups list box we have selected.
		int m_SelectedObjectIndex; //!< Which object in the objects list box we have selected.
		const SceneObject* m_PickedObject; //!< Currently picked object. This may be a valid object even if the player is not done with the picker, as scrolling through objects (but not mousing over them) picks them. Not owned by this.

		Timer m_RepeatStartTimer; //!< Measures the time to when to start repeating inputs when they're held down.
		Timer m_RepeatTimer; //!< Measures the interval between input repeats.

		std::vector<bool> m_ExpandedModules; //!< The modules that have been expanded in the item list.

#pragma region General List Handling
		/// Sets the currently focused list in the picker. For list item highlighting and non-mouse input handling.
		/// @param listToFocusOn The list to focus on. See PickerFocus enumeration.
		/// @return Whether a focus change was made or not.
		bool SetListFocus(PickerFocus listToFocusOn);
#pragma endregion

#pragma region Group List Handling
		/// Selects the specified group index in the groups list and updates the objects list to show the group's objects.
		/// @param groupIndex The group index to select. Removing any selection (with index -1) will be overridden and the currently shown group will be selected instead.
		/// @param updateObjectsList Whether to update the objects list after making the selection or not.
		void SelectGroupByIndex(int groupIndex, bool updateObjectsList = true);

		/// Selects the next or previous group from the one that is currently selected in the groups list.
		/// @param selectPrev Whether to select the previous group. Next group will be selected by default.
		void SelectNextOrPrevGroup(bool selectPrev = false);

		/// Adds all groups with a specific type already defined in PresetMan that are within the set ModuleSpaceID and aren't empty to the current groups list.
		void UpdateGroupsList();
#pragma endregion

#pragma region Object List Handling
		/// Gets the SceneObject from the currently selected index in the objects list. Ownership is NOT transferred!
		/// @return The SceneObject of the currently selected index in the objects list. Nullptr if no valid object is selected (eg. a module subgroup).
		const SceneObject* GetSelectedObject();

		/// Selects the specified object index in the objects list.
		/// @param objectIndex The object index to select.
		/// @param playSelectionSound Whether to play the selection change sound or not.
		void SelectObjectByIndex(int objectIndex, bool playSelectionSound = true);

		/// Selects the next or previous object from the one that is currently selected in the objects list.
		/// @param getPrev Whether to select the previous object. Next object will be selected by default.
		void SelectNextOrPrevObject(bool getPrev = false);

		/// Add the expandable DataModule group separator in the objects list with appropriate name and icon.
		/// @param moduleID The DataModule ID to add group separator for.
		void AddObjectsListModuleGroup(int moduleID);

		/// Sets whether a DataModule group separator shown in the objects list should be expanded or collapsed.
		/// @param moduleID The module ID to set as expanded or collapsed.
		/// @param expanded Whether should be expanded or not.
		void SetObjectsListModuleGroupExpanded(int moduleID, bool expanded = true);

		/// Toggles the expansion/collapse of a DataModule group separator in the objects list.
		/// @param moduleID The module ID to toggle for.
		void ToggleObjectsListModuleGroupExpansion(int moduleID);

		/// Displays the popup box with the description of the selected item in the objects list.
		void ShowDescriptionPopupBox();

		/// Adds all objects of the currently selected group to the objects list.
		/// @param selectTop Whether to reset the selection to the top of the list when we're done updating this.
		void UpdateObjectsList(bool selectTop = true);
#pragma endregion

#pragma region Update Breakdown
		/// Player input handling for all types of input devices.
		/// @return True if the picker was set to be closed or by making a selection from the objects list.
		bool HandleInput();

		/// Player mouse input event handling of the GUIControls of this ObjectPickerGUI.
		/// @return True if the picker was set to be closed by clicking off it or by making a selection from the objects list.
		bool HandleMouseEvents();

		/// Open/close animation handling and GUI element enabling/disabling.
		void AnimateOpenClose();
#pragma endregion

		/// Clears all the member variables of this ObjectPickerGUI, effectively resetting the members of this abstraction level only.
		void Clear();

		// Disallow the use of some implicit methods.
		ObjectPickerGUI(const ObjectPickerGUI& reference) = delete;
		ObjectPickerGUI& operator=(const ObjectPickerGUI& rhs) = delete;
	};
} // namespace RTE
