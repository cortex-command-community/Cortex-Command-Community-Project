#pragma once

#ifdef GUI_STANDALONE
#include "Timer.h"
#endif

namespace RTE {

	/// A label control class.
	class GUILabel : public GUIControl, public GUIPanel {

	public:
		// Label Notifications
		enum {
			Clicked = 0,
		} Notification;

		enum class OverflowScrollState {
			Deactivated = 0,
			WaitAtStart,
			Scrolling,
			WaitAtEnd
		};

		/// Constructor method used to instantiate a GUILabel object in
		/// system memory.
		/// @param Manager GUIManager, GUIControlManager.
		GUILabel(GUIManager* Manager, GUIControlManager* ControlManager);

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

		/// Draws the Label to the given GUIBitmap.
		/// @param Bitmap The GUIBitmap to draw the label to.
		/// @param overwiteFontColorAndKerning Whether to overwrite the font's color and kerning with the stored values. Defaults to true, which is usually what you want.
		void Draw(GUIBitmap* Bitmap, bool overwiteFontColorAndKerning = true);

		/// Called when the mouse goes down on the panel
		/// @param X Mouse Position, Mouse Buttons, Modifier.
		void OnMouseDown(int X, int Y, int Buttons, int Modifier) override;

		/// Called when the mouse goes up on the panel
		/// @param X Mouse Position, Mouse Buttons, Modifier.
		void OnMouseUp(int X, int Y, int Buttons, int Modifier) override;

		/// Returns the panel of the control.
		/// @return 0 if the control does not have a panel, otherwise the topmost panel.
		GUIPanel* GetPanel() override;

		/// Returns a string representing the control's ID
		static std::string GetControlID() { return "LABEL"; };

		/// Called when the control needs to be moved.
		/// @param X New position.
		void Move(int X, int Y) override;

		/// Called when the control needs to be resized.
		/// @param Width New size.
		void Resize(int Width, int Height) override;

		/// Resize the height of the label to fit the amount of text it has to
		/// display.
		/// @return The new height in pixels.
		int ResizeHeightToFit();

		/// Gets the rectangle of the control.
		/// @param X Position, Size.
		void GetControlRect(int* X, int* Y, int* Width, int* Height) override;

		/// Sets the text of the label.
		/// @param text.
		void SetText(const std::string_view& text) { m_Text = text; }

		/// Gets the text of the label.
		const std::string& GetText() const { return m_Text; }

		/// Shows how tall the current text is with the current width and font etc.
		/// @return The text height, in pixels
		int GetTextHeight();

		/// Sets the horizontal alignment of the text of this label.
		/// @param HAlignment The desired alignment. (default: GUIFont::Left)
		void SetHAlignment(int HAlignment = GUIFont::Left) { m_HAlignment = HAlignment; }

		/// Sets the vertical alignment of the text of this label.
		/// @param VAlignment The desired alignment. (default: GUIFont::Top)
		void SetVAlignment(int VAlignment = GUIFont::Top) { m_VAlignment = VAlignment; }

		/// Gets the horizontal alignment of the text of this label.
		int GetHAlignment() const { return m_HAlignment; }

		/// Gets the vertical alignment of the text of this label.
		/// @param The desired alignment.
		int GetVAlignment() const { return m_VAlignment; }

		/// Gets whether or not this GUILabel should scroll horizontally (right) when it overflows.
		/// @return Whether or not this GUILabel should scroll horizontally when it overflows.
		bool GetHorizontalOverflowScroll() const { return m_HorizontalOverflowScroll; }

		/// Sets whether or not this GUILabel should scroll horizontally (right) when it overflows. Mutually exclusive with horizontal overflow scrolling.
		/// @param newOverflowScroll Whether or not this GUILabel should scroll horizontally when it overflows.
		void SetHorizontalOverflowScroll(bool newOverflowScroll);

		/// Gets whether or not this GUILabel should scroll vertically (down) when it overflows.
		/// @return Whether or not this GUILabel should scroll vertically when it overflows.
		bool GetVerticalOverflowScroll() const { return m_VerticalOverflowScroll; }

		/// Sets whether or not this GUILabel should scroll vertically (down) when it overflows. Mutually exclusive with horizontal overflow scrolling.
		/// @param newOverflowScroll Whether or not this GUILabel should scroll vertically when it overflows.
		void SetVerticalOverflowScroll(bool newOverflowScroll);

		/// Gets whether or not horizontal or vertical overflow scrolling is turned on.
		/// @return Whether or not horizontal or vertical overflow scrolling is turned on.
		bool OverflowScrollIsEnabled() const { return m_HorizontalOverflowScroll || m_VerticalOverflowScroll; }

		/// Gets whether or not horizontal/vertical scrolling is happening.
		/// @return Whether or not horizontal/vertical scrolling is happening.
		bool OverflowScrollIsActivated() const { return OverflowScrollIsEnabled() && m_OverflowScrollState != OverflowScrollState::Deactivated; }

		/// Sets whether or not horizontal/vertical scrolling should be happening. When it's deactivated, text will instantly go back to un-scrolled.
		/// @param activateScroll Whether the overflow scrolling should activate (true) or deactivate (false).
		void ActivateDeactivateOverflowScroll(bool activateScroll);

		/// Gets the control to store the values into properties.
		void StoreProperties() override;

		/// Applies new properties to the control.
		/// @param Props GUIProperties.
		void ApplyProperties(GUIProperties* Props) override;

	private:
		std::string m_Text;
		int m_HAlignment;
		int m_VAlignment;
		bool m_HorizontalOverflowScroll; //!< Note that horizontal overflow scrolling means text will always be on one line.
		bool m_VerticalOverflowScroll;
		OverflowScrollState m_OverflowScrollState;
		Timer m_OverflowScrollTimer;
	};
} // namespace RTE
