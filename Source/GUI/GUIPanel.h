#pragma once

namespace RTE {

	class GUIPanel;
	class GUIManager;

	/// A rectangle 'window' in the GUI that recieves mouse and keyboard events.
	class GUIPanel {

	public:
		// Mouse buttons
		enum {
			MOUSE_NONE = 0x00,
			MOUSE_LEFT = 0x01,
			MOUSE_MIDDLE = 0x02,
			MOUSE_RIGHT = 0x04,
		} MouseButtons;

		// Mouse Modifiers
		enum {
			MODI_NONE = 0x00,
			MODI_SHIFT = 0x01,
			MODI_CTRL = 0x02,
			MODI_ALT = 0x04,
			MODI_COMMAND = 0x08
		} MouseModifiers;

		// Z Change
		enum {
			TopMost = 0,
			BottomMost,
		} ZChange;

		/// Constructor method used to instantiate a GUIPanel object in system
		/// memory.
		/// @param Manager Manager.
		explicit GUIPanel(GUIManager* Manager);

		/// Constructor method used to instantiate a GUIPanel object in system
		/// memory.
		GUIPanel();

		/// Clears all the settings.
		void Clear();

		/// Recursively goes down the tree to check the last panel under a point
		/// @param x X, Y Coordinates of point
		/// @return A pointer to the panel. 0 if no panel is under the point
		GUIPanel* BottomPanelUnderPoint(int x, int y);

		/// Recursively goes up the tree from to check the first panel under a point
		/// @param x X, Y Coordinates of point
		/// @return A pointer to the panel. 0 if no panel is under the point
		GUIPanel* TopPanelUnderPoint(int x, int y);

		/// Adds a child to this panel
		/// @param child Pointer to the panel to add
		void AddChild(GUIPanel* child, bool convertToAbsolutePos = true);

		/// Removes a child based on name.
		/// @param pChild Child Name.
		void RemoveChild(const GUIPanel* pChild);

		/// Sets up the panel for use with the manager.
		/// @param manager Pointer to the manager to use, ZPosition.
		void Setup(GUIManager* manager, int ZPos);

		/// Loads the base data from a properties page
		/// @param Props Pointer to the properties class
		void LoadProperties(GUIProperties* Props);

		/// Invalidates the panel
		void Invalidate();

		/// Checks if the panel is valid
		bool IsValid() const;

		/// Draws the panel
		/// @param Screen Screen class
		virtual void Draw(GUIScreen* Screen);

		/// Called when the mouse goes down on the panel
		/// @param X Mouse Position, Mouse Buttons, Modifier.
		virtual void OnMouseDown(int X, int Y, int Buttons, int Modifier);

		/// Called when the mouse goes up on the panel
		/// @param X Mouse Position, Mouse Buttons, Modifier.
		virtual void OnMouseUp(int X, int Y, int Buttons, int Modifier);

		/// Called when the mouse has double-clicked on the pane.
		/// @param X Mouse Position, Mouse Buttons, Modifier.
		virtual void OnDoubleClick(int X, int Y, int Buttons, int Modifier);

		/// Called when the mouse moves (over the panel, or when captured).
		/// @param X Mouse Position, Mouse Buttons, Modifier.
		virtual void OnMouseMove(int X, int Y, int Buttons, int Modifier);

		/// Called when the mouse enters the panel.
		/// @param X Mouse Position, Mouse Buttons, Modifier.
		virtual void OnMouseEnter(int X, int Y, int Buttons, int Modifier);

		/// Called when the mouse leaves the panel.
		/// @param X Mouse Position, Mouse Buttons, Modifier.
		virtual void OnMouseLeave(int X, int Y, int Buttons, int Modifier);

		/// Called when the mouse is hovering over the panel (has to be enabled)
		/// @param X Mouse Position, Mouse Buttons, Modifier.
		virtual void OnMouseHover(int X, int Y, int Buttons, int Modifier);

		/// Called when the mouse scroll wheel is moved.
		/// @param x Mouse X position.
		/// @param y Mouse Y position.
		/// @param modifier Activated modifier buttons.
		/// @param mouseWheelChange The amount of wheel movement. Positive is scroll up, negative is scroll down.
		virtual void OnMouseWheelChange(int x, int y, int modifier, int mouseWheelChange){};

		/// Called when a key goes down.
		/// @param KeyCode KeyCode, Modifier.
		virtual void OnKeyDown(int KeyCode, int Modifier);

		/// Called when a key goes up.
		/// @param KeyCode KeyCode, Modifier.
		virtual void OnKeyUp(int KeyCode, int Modifier);

		/// Called when a key is pressed (OnDown & repeating).
		/// @param KeyCode KeyCode, Modifier.
		virtual void OnKeyPress(int KeyCode, int Modifier);

		/// Called when text input is received.
		/// @param inputText The input text being received.
		virtual void OnTextInput(std::string_view inputText);

		/// Called when the panel gains focus.
		virtual void OnGainFocus();

		/// Called when the panel looses focus.
		virtual void OnLoseFocus();

		/// Adjusts the size of the panel.
		/// @param Width Width, Height.
		virtual void SetSize(int Width, int Height);

		/// Adjusts the absolute position of the panel.
		/// @param X X, Y, and whether to move the children too
		virtual void SetPositionAbs(int X, int Y, bool moveChildren = true);

		/// Sets the position of the panel, relative to its parent.
		/// @param X X, Y.
		virtual void SetPositionRel(int X, int Y);

		/// Moves the position of the panel by a relative amount.
		/// @param dX X, Y, relative.
		virtual void MoveRelative(int dX, int dY);

		/// Centers this in its parent, taking this' dimensions into consideration.
		/// @param centerX Which axes to center.
		virtual void CenterInParent(bool centerX, bool centerY);

		/// Called when receiving a signal.
		/// @param Source Signal source, Signal code, Signal data.
		virtual void ReceiveSignal(GUIPanel* Source, int Code, int Data);

		/// Captures the mouse.
		void CaptureMouse();

		/// Releases the mouse.
		void ReleaseMouse();

		/// Sets the font this panel will be using
		/// @param pFont The new font, ownership is NOT transferred!
		virtual void SetFont(GUIFont* pFont) {
			m_Font = pFont;
			m_ValidRegion = false;
		}

		/// Sets the visibility of the panel.
		/// @param Visible Visible.
		void _SetVisible(bool Visible);

		/// Gets the visibility of the panel.
		bool _GetVisible() const;

		/// Sets the enabled state of the panel.
		/// @param Enabled Enabled.
		void _SetEnabled(bool Enabled);

		/// Gets the enabled state of the panel.
		bool _GetEnabled() const;

		/// Gets the width of the panel.
		int GetWidth() const;

		/// Gets the height of the panel.
		int GetHeight() const;

		/// Gets the x position of the panel
		int GetXPos() const { return m_X; }

		/// Gets the y position of the panel
		int GetYPos() const { return m_Y; }

		/// Gets the x position of the panel, relative to its parent
		int GetRelXPos() const { return m_X - m_Parent->GetXPos(); }

		/// Gets the y position of the panel, relative to its parent
		int GetRelYPos() const { return m_Y - m_Parent->GetYPos(); }

		/// Gets the rectangle of the panel.
		GUIRect* GetRect();

		/// Gets the rectangle of the panel.
		/// @param X X, Y, Width, Height
		void GetRect(int* X, int* Y, int* Width, int* Height) const;

		/// Gets the parent of this panel.
		GUIPanel* GetParentPanel();

		/// Gets the panel's ID.
		int GetPanelID() const;

		/// Sets the panel's captured state.
		/// @param Captured Captured.
		void SetCaptureState(bool Captured);

		/// Gets the panel's captured state.
		bool IsCaptured() const;

		/// Gets the panel's enabled state.
		bool IsEnabled() const;

		/// Sets the target panel to receive signals.
		/// @param Target Target panel.
		void SetSignalTarget(GUIPanel* Target);

		/// Checks if a point is inside the panel
		/// @param X X, Y Coordinates of point
		/// @return A boolean of the check result
		virtual bool PointInside(int X, int Y);

		/// Sets the focus of this panel.
		void SetFocus();

		/// Gets the focus value of the panel.
		bool HasFocus() const;

		/// Sets the Z index of the panel.
		/// @param Z ZPos.
		void SetZPos(int Z);

		/// Gets the Z index of the panel.
		int GetZPos() const;

		/// Changes the Z Position of the panel.
		/// @param Type Change type.
		void ChangeZPosition(int Type);

		/// Convert the properties in the panel to a string.
		std::string ToString();

		/// Adds this panels properties to a properties class.
		/// @param Prop GUIProperties.
		void BuildProperties(GUIProperties* Prop);

	protected:
		int m_X; // absolute coordinates
		int m_Y;
		int m_Width;
		int m_Height;

		bool m_Visible;
		bool m_Enabled;
		bool m_GotFocus;
		bool m_Captured;
		GUIManager* m_Manager;
		GUIPanel* m_Parent;

		GUIFont* m_Font;
		unsigned long m_FontColor;
		unsigned long m_FontShadow;
		int m_FontKerning;

		/// Sends a signal to the target.
		/// @param Code Signal code, Data.
		void SendSignal(int Code, int Data);

		/// Sets up the manager to enable/disable hover tracking of this panel
		/// @param Enabled Enabled, Delay (milliseconds)
		void TrackMouseHover(bool Enabled, int Delay);

		/// Changes the Z position of a child panel.
		/// @param Child Child panel, Change type.
		void _ChangeZ(GUIPanel* Child, int Type);

		/// Writes a single value to string.
		/// @param Name Value name, Value.
		std::string WriteValue(const std::string& Name, int Value);

		/// Writes a single value to string.
		/// @param Name Value name, Value.
		std::string WriteValue(const std::string& Name, bool Value);

		/// Applies new properties to the panel.
		/// @param Props GUIProperties.
		void _ApplyProperties(GUIProperties* Props);

	private:
		std::vector<GUIPanel*> m_Children;
		GUIRect m_Rect;

		int m_ID;
		bool m_ValidRegion;
		int m_ZPos;

		GUIPanel* m_SignalTarget;
	};
} // namespace RTE
