#pragma once

namespace RTE {

	class GUILabel;

	/// A button control class.
	class GUIButton : public GUIControl, public GUIPanel {

	public:
		// Button Notifications
		enum {
			Pushed = 0,
			UnPushed,
			Clicked,
			Focused
		} Notification;

		/// Constructor method used to instantiate a GUIButton object in
		/// system memory.
		/// @param Manager GUIManager, GUIControlManager.
		GUIButton(GUIManager* Manager, GUIControlManager* ControlManager);

		/// Called when the control has been created.
		/// @param Name Name, Position.
		void Create(const std::string& Name, int X, int Y, int Width = -1, int Height = -1) override;

		/// Called when the control has been destroyed.
		void Destroy() override;

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

		/// Called when the mouse moves (over the panel, or when captured).
		/// @param X Mouse Position, Mouse Buttons, Modifier.
		void OnMouseMove(int X, int Y, int Buttons, int Modifier) override;

		/// Called when the mouse enters the panel.
		/// @param X Mouse Position, Mouse Buttons, Modifier.
		void OnMouseEnter(int X, int Y, int Buttons, int Modifier) override;

		/// Called when the mouse leaves the panel.
		/// @param X Mouse Position, Mouse Buttons, Modifier.
		void OnMouseLeave(int X, int Y, int Buttons, int Modifier) override;

		/// Called when the panel gains focus.
		void OnGainFocus() override;

		/// Called when the panel looses focus.
		void OnLoseFocus() override;

		/// Called when a key goes down.
		/// @param KeyCode KeyCode, Modifier.
		void OnKeyDown(int KeyCode, int Modifier) override;

		/// Returns the panel of the control.
		/// @return 0 if the control does not have a panel, otherwise the topmost panel.
		GUIPanel* GetPanel() override;

		/// Returns a string representing the control's ID
		static std::string GetControlID() { return "BUTTON"; };

		/// Gets the rectangle of the control.
		/// @param X Position, Size.
		void GetControlRect(int* X, int* Y, int* Width, int* Height) override;

		/// Gets the control to store the values into properties.
		void StoreProperties() override;

		/// Called when the control needs to be moved.
		/// @param X New position.
		void Move(int X, int Y) override;

		/// Called when the control needs to be resized.
		/// @param Width New size.
		void Resize(int Width, int Height) override;

		/// Gets whether or not this button is currently pushed.
		/// @return Whether or not this button is currently pushed.
		bool IsPushed() const { return m_Pushed; }

		/// Sets whether or not this button is currently pushed.
		/// @param pushed Whether or not this button should be pushed.
		void SetPushed(bool pushed = false);

		/// Gets whether or not this button is currently being moused over.
		/// @return Whether or not this button is currently being moused over.
		bool IsMousedOver() const { return m_Over; }

		/// Gets the text of this GUIButton's GUILabel.
		/// @return The text of this GUIButton's GUILabel.
		const std::string& GetText() const;

		/// Sets the text of this GUIButton's GUILabel.
		/// @param newText The new text for this GUIButton's GUILabel.
		/// @param noBitmapRebuild Lets this method NOT rebuild the button bitmap, even if the icon has changed. Defaults to false and should almost always stay that way.
		void SetText(const std::string_view& newText, bool noBitmapRebuild = false);

		/// Sets whether or not this GUIButton's text should scroll horizontally (right) when it overflows the button.
		/// @param newOverflowScroll Whether or not this GUIButton's text should scroll horizontally when it overflows.
		void SetHorizontalOverflowScroll(bool newOverflowScroll);

		/// Sets whether or not this GUIButton's text should scroll vertically (down) when it overflows the button.
		/// @param newOverflowScroll Whether or not this GUIButton's text should scroll vertically when it overflows.
		void SetVerticalOverflowScroll(bool newOverflowScroll);

		/// Gets whether or not this GUIButton has an icon with a Bitmap.
		bool HasIcon() const { return m_Icon->GetBitmap(); }

		/// Sets the icon for this GUIButton. Ownership is NOT transferred.
		/// @param newIcon A pointer to the new icon BITMAP for this GUIButton.
		/// @param noBitmapRebuild Lets this method NOT rebuild the button bitmap, even if the icon has changed. Defaults to false and should almost always stay that way.
		void SetIcon(BITMAP* newIcon, bool noBitmapRebuild = false);

		/// Helper method to set both text and icon for this GUIButton at the same time.
		/// @param newIcon A pointer to the new icon BITMAP for this GUIButton.
		/// @param newText The new text for this GUIButton's GUILabel.
		void SetIconAndText(BITMAP* newIcon, const std::string_view& newText);

		/// Applies new properties to the control.
		/// @param Props GUIProperties.
		void ApplyProperties(GUIProperties* Props) override;

	private:
		GUIBitmap* m_DrawBitmap;

		bool m_Pushed;
		bool m_Over;
		std::unique_ptr<GUILabel> m_Text;
		std::unique_ptr<GUIBitmap> m_Icon;
		std::unique_ptr<GUIRect> m_BorderSizes;

		/// Create the button bitmap to draw.
		void BuildBitmap();
	};
} // namespace RTE
