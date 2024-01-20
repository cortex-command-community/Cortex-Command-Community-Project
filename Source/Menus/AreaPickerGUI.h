#ifndef _AREAPICKERGUI_
#define _AREAPICKERGUI_

/// AreaPickerGUI class
/// @author Daniel Tabar
/// dtabar@datarealms.com
/// http://www.datarealms.com
/// Inclusions of header files
// #include "FrameMan.h"
#include "Timer.h"
#include "Controller.h"
#include "Scene.h"

struct BITMAP;

namespace RTE {

	class GUIScreen;
	class GUIInput;
	class GUIControlManager;
	class GUICollectionBox;
	class GUIListBox;
	class GUITextBox;
	class GUIButton;
	class GUILabel;

	/// A GUI for picking isntance areas in Cortex Command
	class AreaPickerGUI {

		/// Public member variable, method and friend function declarations
	public:
		/// Constructor method used to instantiate a AreaPickerGUI area in system
		/// memory. Create() should be called before using the area.
		AreaPickerGUI() { Clear(); }

		/// Destructor method used to clean up a AreaPickerGUI area before deletion
		/// from system memory.
		~AreaPickerGUI() { Destroy(); }

		/// Makes the AreaPickerGUI area ready for use.
		/// @param pController A poitner to a Controller which will control this Menu. Ownership is
		/// @param onlyOfType NOT TRANSFERRED! (default: "All")
		/// Which lowest common denominator type to be showing.
		/// @return An error return value signaling sucess or any particular failure.
		/// Anything below 0 is an error signal.
		int Create(Controller* pController, std::string onlyOfType = "All");

		/// Resets the entire AreaPickerGUI, including its inherited members, to
		/// their default settings or values.
		void Reset() { Clear(); }

		/// Destroys and resets (through Clear()) the AreaPickerGUI area.
		void Destroy();

		/// Sets the controller used by this. The ownership of the controller is
		/// NOT transferred!
		/// @param pController The new controller for this menu. Ownership is NOT transferred
		void SetController(Controller* pController) { m_pController = pController; }

		/// Enables or disables the menu. This will animate it in and out of view.
		/// @param enable Whether to enable or disable the menu. (default: true)
		void SetEnabled(bool enable = true);

		/// Reports whether the menu is enabled or not.
		bool IsEnabled() const { return m_PickerEnabled == ENABLED || m_PickerEnabled == ENABLING; }

		/// Reports whether the menu is at all visible or not.
		bool IsVisible() const { return m_PickerEnabled != DISABLED; }

		/// Sets where on the screen that this GUI is being drawn to. If upper
		/// left corner, then 0, 0. This will affect the way the mouse is positioned
		/// etc.
		/// @param newPosX The new screen position of this entire GUI.
		void SetPosOnScreen(int newPosX, int newPosY);

		/// Reports whether and which Area has been picked by the player. This
		/// may happen even though the player isn't done with the picker. (ie
		/// a different area is picked each time the user selects something else
		/// in the areas list).
		/// @return Whether an area has been picked bt the player. 0 if not. Ownership
		/// is NOT transferred!
		Scene::Area* AreaPicked() { return m_pPickedArea; }

		/// Reports whether the user has finished using the picker, and the final
		/// picked Area is returned.
		/// @return Whether an area has been positively and finally picked bt the player.
		/// 0 if not. Ownership is NOT transferred!
		Scene::Area* DonePicking() { return !IsEnabled() && m_pPickedArea ? m_pPickedArea : 0; }

		/// Gets the next area in the areas list, even if the picker is disabled.
		/// @return The next area in the picker list, looping around if necessary.
		/// 0 if no area can be selected. OWNERSHIP IS NOT TRANSFERRED!
		Scene::Area* GetNextArea();

		/// Gets the prev area in the areas list, even if the picker is disabled.
		/// @return The prev area in the picker list, looping around if necessary.
		/// 0 if no area can be selected. OWNERSHIP IS NOT TRANSFERRED!
		Scene::Area* GetPrevArea();

		/// Adds all areas of the currently selected group to the Areas list.
		/// @param selectAreaName The name of the Area to leave selected after the list is updated. (default: "")
		void UpdateAreasList(std::string selectAreaName = "");

		/// Updates the state of this Menu each frame
		void Update();

		/// Draws the menu
		/// @param drawBitmap The bitmap to draw on.
		void Draw(BITMAP* drawBitmap) const;

		/// Protected member variable and method declarations
	protected:
		enum PickerEnabled {
			ENABLING = 0,
			ENABLED,
			DISABLING,
			DISABLED
		};

		// Controller which conrols this menu. Not owned
		Controller* m_pController;
		// GUI Screen for use by the in-game GUI
		GUIScreen* m_pGUIScreen;
		// Input controller
		GUIInput* m_pGUIInput;
		// The control manager which holds all the controls
		GUIControlManager* m_pGUIController;
		// Visibility state of the area picker
		int m_PickerEnabled;
		// Speed at which the menus appear and disappear
		float m_MenuSpeed;
		// Only show areas of this type. "" or "All" will show areas of all types
		std::string m_ShowType;
		// Which Group in the groups list box we have selected
		int m_SelectedGroupIndex;
		// Which Oroup in the Areas list box we have selected
		int m_SelectedAreaIndex;
		// Measures the time to when to start repeating inputs when they're held down
		Timer m_RepeatStartTimer;
		// Measures the interval between input repeats
		Timer m_RepeatTimer;

		// Collection box of the puicker GUI
		GUICollectionBox* m_pParentBox;
		// The Listbox which lists all the groups
		GUIListBox* m_pGroupsList;
		// The Listbox which lists all the areas in the currently selected group
		GUIListBox* m_pAreasList;
		// The Button for deleting the currently selected Area
		GUIButton* m_pDeleteAreaButton;
		// Currently picked area. This is 0 until the user actually picks somehting, not just has the cursor over it
		// Not owned by this.
		Scene::Area* m_pPickedArea;
		// The cursor image shared by all pickers
		static BITMAP* s_pCursor;
		// Screen position of the cursor
		Vector m_CursorPos;

		/// Private member variable and method declarations
	private:
		/// Clears all the member variables of this AreaPickerGUI, effectively
		/// resetting the members of this abstraction level only.
		void Clear();

		// Disallow the use of some implicit methods.
		AreaPickerGUI(const AreaPickerGUI& reference) = delete;
		AreaPickerGUI& operator=(const AreaPickerGUI& rhs) = delete;
	};

} // namespace RTE

#endif // File
