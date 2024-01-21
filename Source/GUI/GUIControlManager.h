#pragma once

#include "GUIWriter.h"
#include "GUIReader.h"

namespace RTE {

	/// A class used to manage the GUI as a whole and provide the interface between the GUI and the rest of the system.
	class GUIControlManager {
		friend class GUIControl;

	public:
		// Cursor types
		enum {
			Pointer,
			Text,
			HorSize
		} CursorType;

		/// Constructor method used to instantiate a GUIControlManager object in
		/// system memory.
		GUIControlManager();

		/// Destructor method used to clean up a GUIControlManager object in
		/// system memory.
		~GUIControlManager();

		/// Creates the data for the control manager
		/// @param Screen Screen and Input Interfaces, Skin directory
		bool Create(GUIScreen* Screen, GUIInput* Input, const std::string& SkinDir, const std::string& SkinFilename = "skin.ini");

		/// Frees all the allocated resources.
		void Destroy();

		/// Clears all the controls.
		void Clear();

		/// Changes the skin of the controls.
		/// @param SkinDir Skin directory.
		void ChangeSkin(const std::string& SkinDir, const std::string& SkinFilename = "skin.ini");

		/// Gets the Skin object currently in use.
		/// @return A pointer to the currently used skin. Please don't mess it up.
		GUISkin* GetSkin() { return m_Skin; }

		/// Updates the GUI every frame
		/// @param ignoreKeyboardEvents Whether keyboard events should be ignored or not. Used to avoid conflicts when custom keyboard handling for GUI elements is preset. (default: false)
		void Update(bool ignoreKeyboardEvents = false);

		/// Draws the GUI to the back buffer.
		void Draw();

		/// Draws the GUI to the back buffer.
		/// @param pScreen The GUIScreen to draw to, overriding the one passed in on construction
		void Draw(GUIScreen* pScreen);

		/// Draws the mouse to the backbuffer.
		/// @param pScreen The GUIScreen to draw to, overriding the one passed in on construction.
		void DrawMouse(GUIScreen* guiScreen = nullptr);

		/// Enables and disables the mouse completely for this.
		/// @param enable Enable? (default: true) { m_GUIManager->EnableMouse(enable)
		void EnableMouse(bool enable = true) { m_GUIManager->EnableMouse(enable); }

		/// Sets the absolute position of this entire GUI on the screen. This is
		/// useful if the UI's are being drawn in a different area of the screen
		/// than the top left corner. This will adjust the mouse input to match
		/// the offset screen location.
		/// @param screenPosX The position.
		void SetPosOnScreen(int screenPosX, int screenPosY) { m_Input->SetMouseOffset(-screenPosX, -screenPosY); }

		/// Gets the control manager
		/// @param Name.
		/// @return The manager, ownership is NOT transferred!
		GUIManager* GetManager() { return m_GUIManager; }

		/// Manually creates a control.
		/// @param Name Name, Type, Position, Size, Parent.
		/// @return GUIControl class created. 0 if not created.
		GUIControl* AddControl(const std::string& Name, const std::string& Type, GUIControl* Parent, int X, int Y, int Width, int Height);

		/// Manually creates a control.
		/// @param Property Properties.
		/// @return GUIControl class created. 0 if not created.
		GUIControl* AddControl(GUIProperties* Property);

		/// Gets a control
		/// @param Name Name.
		/// @return GUIControl class, or 0 if not found.
		GUIControl* GetControl(const std::string& Name);

		/// Gets the control list
		/// @return vector<GUIControl *> Pointer.
		std::vector<GUIControl*>* GetControlList();

		/// Checks if a control is under a specific point
		/// @param pointX The absolute point coordinates to check under.
		/// @param pointY Parent to check under. Pass null to default to the root control.
		/// @param pParent How many levels of children under the parent to look at. If negative, (default: nullptr)
		/// goes as far as it can.
		/// @param depth Returns:			GUIControl. NULL if no control under the point (default: -1)
		GUIControl* GetControlUnderPoint(int pointX, int pointY, GUIControl* pParent = nullptr, int depth = -1);

		/// Removes a control by name
		/// @param Name Name, RemoveFromParent.
		void RemoveControl(const std::string& Name, bool RemoveFromParent);

		/// Gets an event from the queue.
		/// @param Event Pointer to variable receiving the Event.
		/// @return Returns true when an event was grabbed.
		/// Returns false when there was no more events in the queue
		/// OR the Event pointer is 0.
		bool GetEvent(GUIEvent* Event);

		/// Sets the cursor type.
		/// @param CursorType Cursor type.
		void SetCursor(int CursorType);

		/// Saves the layout to a file.
		/// @param Filename Filename.
		/// @return True if successful.
		bool Save(const std::string& Filename);

		/// Saves the layout to a Writer class.
		/// @param W Writer class.
		/// @return True if successful.
		bool Save(GUIWriter* W);

		/// Loads the layout from a file.
		/// @param Filename Filename.
		/// @param keepOld Whether to NOT clear out the manager, but just add the controls loaded (default: false)
		/// to the existing layout.
		/// @return True if successful.
		bool Load(const std::string& Filename, bool keepOld = false);

		/// Gets the GUIScreen that this GUIControlManager is drawing itself to.
		/// @return Pointer to the GUIScreen that this GUIControlManager is drawing itself to.
		GUIScreen* GetScreen() const { return m_Screen; }

	private:
		GUIScreen* m_Screen; // Not owned.
		GUIInput* m_Input; // Not owned.
		GUISkin* m_Skin;
		GUIManager* m_GUIManager;

		std::vector<GUIControl*> m_ControlList;
		std::vector<GUIEvent*> m_EventQueue;

		int m_CursorType;

		/// Add a new event to the queue.
		/// @param Event Event point.
		void AddEvent(GUIEvent* Event);
	};
} // namespace RTE
