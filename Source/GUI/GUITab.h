#pragma once

namespace RTE {

	/// A tab control class.
	class GUITab : public GUIControl, public GUIPanel {

	public:
		// Tab Notifications
		enum {
			Hovered = 0,
			Pushed,
			UnPushed,
			Changed,
		} Notification;

		/// Constructor method used to instantiate a GUITab object in
		/// system memory.
		/// @param Manager GUIManager, GUIControlManager.
		GUITab(GUIManager* Manager, GUIControlManager* ControlManager);

		/// Called when the control has been created.
		/// @param Name Name, Position.
		void Create(const std::string& Name, int X, int Y, int Width = -1, int Height = -1) override;

		/// Called when the control has been created.
		/// @param Props Properties.
		void Create(GUIProperties* Props) override;

		/// Called when the skin has been changed.
		/// @param Skin New skin pointer.
		void ChangeSkin(GUISkin* Skin) override;

		/// Draws the panel
		/// @param Screen Screen class
		void Draw(GUIScreen* Screen) override;

		/// Called when the mouse goes down on the panel
		/// @param X Mouse Position, Mouse Buttons, Modifier.
		void OnMouseDown(int X, int Y, int Buttons, int Modifier) override;

		/// Called when the mouse goes up on the panel
		/// @param X Mouse Position, Mouse Buttons, Modifier.
		void OnMouseUp(int X, int Y, int Buttons, int Modifier) override;

		/// Called when the mouse enters the panel.
		/// @param X Mouse Position, Mouse Buttons, Modifier.
		void OnMouseEnter(int X, int Y, int Buttons, int Modifier) override;

		/// Called when the mouse leaves the panel.
		/// @param X Mouse Position, Mouse Buttons, Modifier.
		void OnMouseLeave(int X, int Y, int Buttons, int Modifier) override;

		/// Returns the panel of the control.
		/// @return 0 if the control does not have a panel, otherwise the topmost panel.
		GUIPanel* GetPanel() override;

		/// Returns a string representing the control's ID
		static std::string GetControlID() { return "TAB"; };

		/// Called when the control needs to be moved.
		/// @param X New position.
		void Move(int X, int Y) override;

		/// Called when the control needs to be resized.
		/// @param Width New size.
		void Resize(int Width, int Height) override;

		/// Gets the rectangle of the control.
		/// @param X Position, Size.
		void GetControlRect(int* X, int* Y, int* Width, int* Height) override;

		/// Gets the control to store the values into properties.
		void StoreProperties() override;

		/// Sets the check state.
		/// @param Check State.
		void SetCheck(bool Check);

		/// Gets the check state.
		bool GetCheck() const;

		/// Sets the text.
		/// @param Text Text.
		void SetText(const std::string& Text);

		/// Gets the text.
		std::string GetText() const;

		/// Applies new properties to the control.
		/// @param Props GUIProperties.
		void ApplyProperties(GUIProperties* Props) override;

	private:
		GUIBitmap* m_Image;
		GUIRect m_ImageRects[4];

		bool m_Selected;
		int m_Mouseover;
		std::string m_Text;

		/// Create the checkbox bitmap to draw.
		void BuildBitmap();
	};
} // namespace RTE
