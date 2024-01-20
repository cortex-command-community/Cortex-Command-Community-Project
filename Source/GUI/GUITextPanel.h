#pragma once

namespace RTE {

	/// A text panel class.
	class GUITextPanel : public GUIPanel {

	public:
		// Text panel signals
		enum {
			Clicked = 0,
			MouseDown,
			Changed,
			Enter
		} Signals;

		/// Constructor method used to instantiate a GUITextPanel object in
		/// system memory.
		/// @param Manager GUIManager.
		explicit GUITextPanel(GUIManager* Manager);

		/// Constructor method used to instantiate a GUITextPanel object in
		/// system memory.
		GUITextPanel();

		/// Create the panel.
		/// @param X Position, Size.
		void Create(int X, int Y, int Width, int Height);

		/// Called when the skin has been changed.
		/// @param Skin New skin pointer.
		void ChangeSkin(GUISkin* Skin);

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

		/// Called when a key is pressed (OnDown & repeating).
		/// @param KeyCode KeyCode, Modifier.
		void OnKeyPress(int KeyCode, int Modifier) override;

		void OnTextInput(std::string_view inputText) override;

		/// Sets the text in the textpanel.
		/// @param Text Text.
		void SetText(const std::string& Text);

		/// Sets the extra text which appears right-justified in the textpanel.
		/// @param rightText Text.
		void SetRightText(const std::string& rightText);

		/// Gets the text in the textpanel.
		std::string GetText() const { return m_Text; }

		/// Gets the extra text which appears right-justified in the textpanel.
		std::string GetRightText() const { return m_RightText; }

		/// Sets the start and end indexes of the selection text.
		/// @param Start Start, End.
		void SetSelection(int Start, int End);

		/// Gets the start index of the selection.
		/// @return Index of the start of the selection. -1 if no selection
		int GetSelectionStart() const;

		/// Gets the end index of the selection.
		/// @return Index of the end of the selection. -1 if no selection
		int GetSelectionEnd() const;

		/// Clears the selection. Does NOT remove the selection text though.
		void ClearSelection();

		/// Gets the selection text.
		std::string GetSelectionText() const;

		/// Removes the characters in the selection.
		void RemoveSelectionText();

		/// Sets where the cursor should be. This will clear any selection.
		/// @param cursorPos The index of the new cursor position.
		void SetCursorPos(int cursorPos);

		/// Sets the locked state on the textbox.
		/// @param Locked Locked.
		void SetLocked(bool Locked);

		/// Gets the locked state on the textbox.
		bool GetLocked() const;

		/// Sets this text panel to accept numeric symbols only.
		/// @param numericOnly Whether to accept numeric symbols only or not.
		void SetNumericOnly(bool numericOnly) { m_NumericOnly = numericOnly; }

		/// Sets this text panel's maximum numeric value when in numeric only mode.
		/// @param maxValue The maximum numeric value. 0 means no maximum value.
		void SetMaxNumericValue(int maxValue) { m_MaxNumericValue = maxValue; }

		/// Sets the maximum length of the text this text panel can contain.
		/// @param maxLength The maximum length of the text this text panel can contain.
		void SetMaxTextLength(int maxLength) { m_MaxTextLength = maxLength; }

	private:
		unsigned long m_FontSelectColor;

		std::string m_Text;
		std::string m_RightText; // Appears right-justified in the text field
		bool m_Focus;
		bool m_Locked;

		// The distance from the side and top of the text box, to the side and top of the first line of text
		int m_WidthMargin;
		int m_HeightMargin;

		// Cursor
		int m_CursorX;
		int m_CursorY;
		int m_CursorIndex;
		unsigned long m_CursorColor;
		Timer m_BlinkTimer;

		int m_StartIndex;

		// Selection
		bool m_GotSelection;
		int m_StartSelection;
		int m_EndSelection;
		unsigned long m_SelectedColorIndex;
		int m_SelectionX;
		int m_SelectionWidth;

		int m_MaxTextLength; //!< The maximum length of the text this text panel can contain.
		bool m_NumericOnly; //!< Whether this text panel only accepts numeric symbols.
		int m_MaxNumericValue; //!< The maximum numeric value when in numeric only mode. 0 means no maximum value.

		/// Updates the cursor and start positions.
		/// @param Typing Typing, Increment. (default: false)
		void UpdateText(bool Typing = false, bool DoIncrement = true);

		/// Update the selection.
		/// @param Start Start, End.
		void DoSelection(int Start, int End);

		/// Gets the index of the start of the next contiguous group of letters or special characters in the given string, or the end of the string if there is none.
		/// Generally used to deal with ctrl + arrows style behavior.
		/// @param stringToCheck A string_view of the string to look for the next word in.
		/// @param currentIndex The index in the string to start looking from.
		/// @return The index of the start of the next contiguous group of letters or special characters in the given string, or the end of the string if there is none.
		int GetStartOfNextCharacterGroup(const std::string_view& stringToCheck, int currentIndex) const;

		/// Gets the index of the start of the previous contiguous group of letters or special characters in the given string, or the end of the string if there is none.
		/// Generally used to deal with ctrl + arrows style behavior.
		/// @param stringToCheck A string_view of the string to look for the next word in.
		/// @param currentIndex The index in the string to start looking from.
		/// @return The index of the start of the previous contiguous group of letters or special characters in the given string, or the end of the string if there is none.
		int GetStartOfPreviousCharacterGroup(const std::string_view& stringToCheck, int currentIndex) const;
	};
} // namespace RTE
