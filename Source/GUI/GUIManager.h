#pragma once

namespace RTE {

	class Timer;

	/// The main manager that handles all the panels and inputs.
	class GUIManager {

	public:
		/// Constructor method used to instantiate a GUIManager object in system
		/// memory.
		/// @param input Input Interface
		explicit GUIManager(GUIInput* input);

		/// Destructor method used to clean up a GUIManager object.
		~GUIManager();

		/// Clears the manager.
		void Clear();

		/// Adds a panel to the list.
		/// @param panel Pointer to a panel.
		void AddPanel(GUIPanel* panel);

		/// Updates the GUI.
		/// @param ignoreKeyboardEvents Whether keyboard events should be ignored or not. Used to avoid conflicts when custom keyboard handling for GUI elements is preset. (default: false)
		void Update(bool ignoreKeyboardEvents = false);

		/// Draw all the panels
		/// @param Screen Screen.
		void Draw(GUIScreen* Screen);

		/// Enables and disables the mouse completely for this.
		/// @param enable Enable? (default: true)
		void EnableMouse(bool enable = true) { m_MouseEnabled = enable; }

		/// Sets up capturing a mouse for a panel.
		/// @param Panel Panel.
		void CaptureMouse(GUIPanel* Panel);

		/// Releases a mouse capture.
		void ReleaseMouse();

		/// Gets a unique ID for a panel.
		int GetPanelID();

		/// Gets the input controller object
		GUIInput* GetInputController() { return m_Input; }

		/// Sets up the manager to enable/disable hover tracking of this panel
		/// @param Pan Panel, Enabled, Delay (milliseconds)
		void TrackMouseHover(GUIPanel* Pan, bool Enabled, int Delay);

		/// Give focus to a panel.
		/// @param Pan Panel.
		void SetFocus(GUIPanel* Pan);

	private:
		std::vector<GUIPanel*> m_PanelList;
		GUIPanel* m_CapturedPanel;
		GUIPanel* m_FocusPanel;
		GUIPanel* m_MouseOverPanel;

		GUIInput* m_Input;
		bool m_MouseEnabled;
		int m_OldMouseX;
		int m_OldMouseY;

		int m_DoubleClickTime;
		int m_DoubleClickSize;
		int m_DoubleClickButtons;
		float m_LastMouseDown[3];
		GUIRect m_DoubleClickRect;

		bool m_HoverTrack;
		GUIPanel* m_HoverPanel;
		float m_HoverTime;

		bool m_UseValidation;
		int m_UniqueIDCount;

		Timer* m_pTimer;

		/// Goes through the panel list and selects the bottommost
		/// ('first', render wise) panel on a specific point.
		/// @param X Mouse Position.
		GUIPanel* FindBottomPanel(int X, int Y);

		/// Goes through the panel list and selects the topmost ('last', render
		/// wise) panel on a specific point.
		/// @param X Mouse Position.
		GUIPanel* FindTopPanel(int X, int Y);

		/// Checks if the mouse point is inside a rectangle.
		/// @param Rect Rectangle, Mouse position.
		bool MouseInRect(const GUIRect* Rect, int X, int Y);
	};
} // namespace RTE
