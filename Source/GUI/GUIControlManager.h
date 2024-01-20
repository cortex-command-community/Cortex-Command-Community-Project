#ifndef _GUICONTROLMANAGER_
#define _GUICONTROLMANAGER_

#include "GUIWriter.h"
#include "GUIReader.h"

namespace RTE {

	/// <summary>
	/// A class used to manage the GUI as a whole and provide the interface between the GUI and the rest of the system.
	/// </summary>
	class GUIControlManager {
		friend class GUIControl;

	public:
		// Cursor types
		enum {
			Pointer,
			Text,
			HorSize
		} CursorType;

		//////////////////////////////////////////////////////////////////////////////////////////
		// Constructor:     GUIControlmanager
		//////////////////////////////////////////////////////////////////////////////////////////
		// Description:     Constructor method used to instantiate a GUIControlManager object in
		//                  system memory.
		// Arguments:       None.

		GUIControlManager();

		//////////////////////////////////////////////////////////////////////////////////////////
		// Destructor:      GUIControlmanager
		//////////////////////////////////////////////////////////////////////////////////////////
		// Description:     Destructor method used to clean up a GUIControlManager object in
		//                  system memory.
		// Arguments:       None.

		~GUIControlManager() { Destroy(); }

		//////////////////////////////////////////////////////////////////////////////////////////
		// Method:          Create
		//////////////////////////////////////////////////////////////////////////////////////////
		// Description:     Creates the data for the control manager
		// Arguments:       Screen and Input Interfaces, Skin directory

		bool Create(GUIScreen* Screen, GUIInput* Input, const std::string& SkinDir, const std::string& SkinFilename = "skin.ini");

		//////////////////////////////////////////////////////////////////////////////////////////
		// Method:          Destroy
		//////////////////////////////////////////////////////////////////////////////////////////
		// Description:     Frees all the allocated resources.
		// Arguments:       None.

		void Destroy();

		//////////////////////////////////////////////////////////////////////////////////////////
		// Method:          Clear
		//////////////////////////////////////////////////////////////////////////////////////////
		// Description:     Clears all the controls.
		// Arguments:       None.

		void Clear();

		//////////////////////////////////////////////////////////////////////////////////////////
		// Method:          ChangeSkin
		//////////////////////////////////////////////////////////////////////////////////////////
		// Description:     Changes the skin of the controls.
		// Arguments:       Skin directory.

		void ChangeSkin(const std::string& SkinDir, const std::string& SkinFilename = "skin.ini");

		//////////////////////////////////////////////////////////////////////////////////////////
		// Method:          GetSkin
		//////////////////////////////////////////////////////////////////////////////////////////
		// Description:     Gets the Skin object currently in use.
		// Arguments:       None.
		// Returns:         A pointer to the currently used skin. Please don't mess it up.

		GUISkin* GetSkin() { return m_Skin; }

		//////////////////////////////////////////////////////////////////////////////////////////
		// Method:          Update
		//////////////////////////////////////////////////////////////////////////////////////////
		// Description:     Updates the GUI every frame
		// Arguments:       Whether keyboard events should be ignored or not. Used to avoid conflicts when custom keyboard handling for GUI elements is preset.

		void Update(bool ignoreKeyboardEvents = false);

		//////////////////////////////////////////////////////////////////////////////////////////
		// Method:          Draw
		//////////////////////////////////////////////////////////////////////////////////////////
		// Description:     Draws the GUI to the back buffer.
		// Arguments:       None.

		void Draw();

		//////////////////////////////////////////////////////////////////////////////////////////
		// Method:          Draw
		//////////////////////////////////////////////////////////////////////////////////////////
		// Description:     Draws the GUI to the back buffer.
		// Arguments:       The GUIScreen to draw to, overriding the one passed in on construction

		void Draw(GUIScreen* pScreen);

		/// <summary>
		/// Draws the mouse to the backbuffer.
		/// </summary>
		/// <param name="pScreen">The GUIScreen to draw to, overriding the one passed in on construction.</param>
		void DrawMouse(GUIScreen* guiScreen = nullptr);

		//////////////////////////////////////////////////////////////////////////////////////////
		// Method:          EnableMouse
		//////////////////////////////////////////////////////////////////////////////////////////
		// Description:     Enables and disables the mouse completely for this.
		// Arguments:       Enable?

		void EnableMouse(bool enable = true) { m_GUIManager->EnableMouse(enable); }

		//////////////////////////////////////////////////////////////////////////////////////////
		// Method:          SetPosOnScreen
		//////////////////////////////////////////////////////////////////////////////////////////
		// Description:     Sets the absolute position of this entire GUI on the screen. This is
		//                  useful if the UI's are being drawn in a different area of the screen
		//                  than the top left corner. This will adjust the mouse input to match
		//                  the offset screen location.
		// Arguments:       The position.

		void SetPosOnScreen(int screenPosX, int screenPosY) { m_Input->SetMouseOffset(-screenPosX, -screenPosY); }

		//////////////////////////////////////////////////////////////////////////////////////////
		// Method:          GetManager
		//////////////////////////////////////////////////////////////////////////////////////////
		// Description:     Gets the control manager
		// Arguments:       Name.
		// Returns:         The manager, ownership is NOT transferred!

		GUIManager* GetManager() { return m_GUIManager; }

		//////////////////////////////////////////////////////////////////////////////////////////
		// Method:          AddControl
		//////////////////////////////////////////////////////////////////////////////////////////
		// Description:     Manually creates a control.
		// Arguments:       Name, Type, Position, Size, Parent.
		// Returns:         GUIControl class created. 0 if not created.

		GUIControl* AddControl(const std::string& Name, const std::string& Type, GUIControl* Parent, int X, int Y, int Width, int Height);

		//////////////////////////////////////////////////////////////////////////////////////////
		// Method:          AddControl
		//////////////////////////////////////////////////////////////////////////////////////////
		// Description:     Manually creates a control.
		// Arguments:       Properties.
		// Returns:         GUIControl class created. 0 if not created.

		GUIControl* AddControl(GUIProperties* Property);

		//////////////////////////////////////////////////////////////////////////////////////////
		// Method:          GetControl
		//////////////////////////////////////////////////////////////////////////////////////////
		// Description:     Gets a control
		// Arguments:       Name.
		// Returns:         GUIControl class, or 0 if not found.

		GUIControl* GetControl(const std::string& Name);

		//////////////////////////////////////////////////////////////////////////////////////////
		// Method:          GetControlList
		//////////////////////////////////////////////////////////////////////////////////////////
		// Description:     Gets the control list
		// Arguments:       None.
		// Returns:         vector<GUIControl *> Pointer.

		std::vector<GUIControl*>* GetControlList();

		//////////////////////////////////////////////////////////////////////////////////////////
		// Method:          GetControlUnderPoint
		//////////////////////////////////////////////////////////////////////////////////////////
		// Description:     Checks if a control is under a specific point
		// Arguments:       The absolute point coordinates to check under.
		//                  Parent to check under. Pass null to default to the root control.
		//                  How many levels of children under the parent to look at. If negative,
		//                  goes as far as it can.
		// Returns:			GUIControl. NULL if no control under the point

		GUIControl* GetControlUnderPoint(int pointX, int pointY, GUIControl* pParent = nullptr, int depth = -1);

		//////////////////////////////////////////////////////////////////////////////////////////
		// Method:          RemoveControl
		//////////////////////////////////////////////////////////////////////////////////////////
		// Description:     Removes a control by name
		// Arguments:       Name, RemoveFromParent.
		// Returns:         None.

		void RemoveControl(const std::string& Name, bool RemoveFromParent);

		//////////////////////////////////////////////////////////////////////////////////////////
		// Method:          GetEvent
		//////////////////////////////////////////////////////////////////////////////////////////
		// Description:     Gets an event from the queue.
		// Arguments:       Pointer to variable receiving the Event.
		// Returns:         Returns true when an event was grabbed.
		//                  Returns false when there was no more events in the queue
		//                  OR the Event pointer is 0.

		bool GetEvent(GUIEvent* Event);

		//////////////////////////////////////////////////////////////////////////////////////////
		// Method:          SetCursor
		//////////////////////////////////////////////////////////////////////////////////////////
		// Description:     Sets the cursor type.
		// Arguments:       Cursor type.

		void SetCursor(int CursorType);

		//////////////////////////////////////////////////////////////////////////////////////////
		// Method:          Save
		//////////////////////////////////////////////////////////////////////////////////////////
		// Description:     Saves the layout to a file.
		// Arguments:       Filename.
		// Returns:         True if successful.

		bool Save(const std::string& Filename);

		//////////////////////////////////////////////////////////////////////////////////////////
		// Method:          Save
		//////////////////////////////////////////////////////////////////////////////////////////
		// Description:     Saves the layout to a Writer class.
		// Arguments:       Writer class.
		// Returns:         True if successful.

		bool Save(GUIWriter* W);

		//////////////////////////////////////////////////////////////////////////////////////////
		// Method:          Load
		//////////////////////////////////////////////////////////////////////////////////////////
		// Description:     Loads the layout from a file.
		// Arguments:       Filename.
		//                  Whether to NOT clear out the manager, but just add the controls loaded
		//                  to the existing layout.
		// Returns:         True if successful.

		bool Load(const std::string& Filename, bool keepOld = false);

		/// <summary>
		/// Gets the GUIScreen that this GUIControlManager is drawing itself to.
		/// </summary>
		/// <returns>Pointer to the GUIScreen that this GUIControlManager is drawing itself to.</returns>
		GUIScreen* GetScreen() const { return m_Screen; }

	private:
		GUIScreen* m_Screen; // Not owned.
		GUIInput* m_Input; // Not owned.
		GUISkin* m_Skin;
		GUIManager* m_GUIManager;

		std::vector<GUIControl*> m_ControlList;
		std::vector<GUIEvent*> m_EventQueue;

		int m_CursorType;

		//////////////////////////////////////////////////////////////////////////////////////////
		// Method:          AddEvent
		//////////////////////////////////////////////////////////////////////////////////////////
		// Description:     Add a new event to the queue.
		// Arguments:       Event point.

		void AddEvent(GUIEvent* Event);
	};
}; // namespace RTE
#endif //  _GUICONTROLMANAGER_