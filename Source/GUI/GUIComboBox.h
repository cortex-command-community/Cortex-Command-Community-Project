#pragma once

#include "GUITextPanel.h"
#include "GUIListPanel.h"

namespace RTE {

	class GUIComboBoxButton;

	/// A ComboBox control class.
	class GUIComboBox : public GUIControl, public GUIPanel {

		/// Public member variable, method and friend function declarations
	public:
		// Notifications
		enum {
			Dropped, // When listpanel has dropped
			Closed, // When listpanel has closed
		} Notifications;

		// Combo Style
		enum {
			DropDown,
			DropDownList,
		} DropDownStyles;

		/// Constructor method used to instantiate a GUIComboBox object in
		/// system memory.
		/// @param Manager GUIManager, GUIControlManager.
		GUIComboBox(GUIManager* Manager, GUIControlManager* ControlManager);

		/// Called when the control has been created.
		/// @param Name Name, Position.
		void Create(const std::string& Name, int X, int Y, int Width = -1, int Height = -1) override;

		/// Called when the control has been created.
		/// @param Props Properties.
		void Create(GUIProperties* Props) override;

		/// Called when the control has been destroyed.
		void Destroy() override;

		/// Called when the control is activated and ready for use.
		void Activate() override;

		/// Called when the skin has been changed.
		/// @param Skin New skin pointer.
		void ChangeSkin(GUISkin* Skin) override;

		/// Draws the panel
		/// @param Screen Screen class
		void Draw(GUIScreen* Screen) override;

		/// Returns the panel of the control.
		/// @return 0 if the control does not have a panel, otherwise the topmost panel.
		GUIPanel* GetPanel() override;

		/// Returns the ListPanel component of the control.
		/// @return The ListPanel component of this ComboBox.
		GUIListPanel* GetListPanel() { return m_ListPanel; }

		/// Returns a string representing the control's ID
		static std::string GetControlID() { return "COMBOBOX"; };

		/// Called when receiving a signal.
		/// @param Source Signal source, Signal code, Signal data.
		void ReceiveSignal(GUIPanel* Source, int Code, int Data) override;

		/// Locks the control from updating every time a new item is added.
		void BeginUpdate();

		/// UnLocks the control from updating every time a new item is added.
		/// Will automatically update the control.
		void EndUpdate();

		/// Called when the control needs to be moved.
		/// @param X New position.
		void Move(int X, int Y) override;

		/// Called when the control needs to be resized.
		/// @param Width New size.
		void Resize(int Width, int Height) override;

		/// Gets the rectangle of the control.
		/// @param X Position, Size.
		void GetControlRect(int* X, int* Y, int* Width, int* Height) override;

		/// Add an item to the list.
		/// @param Name Name, Extra text, bitmap to show in the list, extra entity data
		void AddItem(const std::string& Name, const std::string& ExtraText = "", GUIBitmap* pBitmap = nullptr, const Entity* pEntity = nullptr);

		/// Delete an item from the list.
		/// @param Index Item Index.
		void DeleteItem(int Index);

		/// Clears the list.
		void ClearList();

		/// Get the item count.
		int GetCount();

		/// Get the index of the selected item.
		int GetSelectedIndex();

		/// Get the index of the previously selected item before the selection is
		/// made.
		int GetOldSelectionIndex() const { return m_OldSelection; }

		/// Sets the index of the selected item.
		void SetSelectedIndex(int Index);

		/// Rolls back the selection to the previous selected item.
		/// @return Whether the rollback worked and was performed.
		bool RollbackSelection();

		/// Returns the Item structure at the index.
		/// @param Index Index.
		/// @return Pointer to the item structure. 0 if the index was invalid.
		GUIListPanel::Item* GetItem(int Index);

		/// Returns the Item structure at the currently selected index.
		/// @param GetItem(GetSelectedIndex() Index.
		/// @return Pointer to the item structure. 0 if nothing valid is selected.
		GUIListPanel::Item* GetSelectedItem() { return GetItem(GetSelectedIndex()); }

		/// Sets the drop height of the list.
		/// @param Drop Height.
		void SetDropHeight(int Drop);

		/// Gets the drop height of the list.
		/// @return The drop height of the list.
		int GetDropHeight() const { return m_DropHeight; }

		/// Gets the control to store the values into properties.
		void StoreProperties() override;

		/// Sets the drop down style of the combo box.
		/// @param Style Style.
		void SetDropDownStyle(int Style);

		/// Gets the drop down style of the combo box.
		int GetDropDownStyle() const;

		/// Sets the visibility of the control.
		/// @param Visible Visible.
		void SetVisible(bool Visible) override;

		/// Gets the visibility of the control.
		bool GetVisible() override;

		/// Sets the enabled state of the control.
		/// @param Enabled Enabled.
		void SetEnabled(bool Enabled) override;

		/// Gets the enabled state of the control.
		bool GetEnabled() override;

		/// Gets text (only if style is DropDown).
		/// @return Text. Returns empty string is style is not DropDown.
		std::string GetText();

		/// Sets text (only if style is DropDown).
		/// @param Text Text.
		void SetText(const std::string& Text);

		/// Applies new properties to the control.
		/// @param Props GUIProperties.
		void ApplyProperties(GUIProperties* Props) override;

		/// Shows whether the list is currently dropped down or not.
		/// @return Whether this is currently dropped down and showing the list.
		bool IsDropped() { return m_ListPanel->_GetVisible(); }

	private:
		GUIBitmap* m_DrawBitmap;
		int m_OldSelection;
		bool m_CreatedList;

		int m_DropHeight;
		int m_DropDownStyle;

		GUITextPanel* m_TextPanel;
		GUIListPanel* m_ListPanel;
		GUIComboBoxButton* m_Button;
	};

	/// A ComboBoxButton control class.
	class GUIComboBoxButton : public GUIPanel {

	public:
		// Signals
		enum {
			Clicked
		} Signals;

		/// Constructor method used to instantiate a GUIComboBoxButton object in
		/// system memory.
		/// @param Manager GUIManager.
		explicit GUIComboBoxButton(GUIManager* Manager);

		/// Called when the skin has been changed.
		/// @param Skin New skin pointer.
		void ChangeSkin(GUISkin* Skin);

		/// Draws the panel
		/// @param Screen Screen class
		void Draw(GUIScreen* Screen) override;

		/// Create the panel.
		/// @param X Position, Size.
		void Create(int X, int Y, int Width, int Height);

		/// Destroys the button.
		void Destroy();

		/// Called when the mouse goes down on the panel
		/// @param X Mouse Position, Mouse Buttons, Modifier.
		void OnMouseDown(int X, int Y, int Buttons, int Modifier) override;

		/// Called when the mouse goes up on the panel
		/// @param X Mouse Position, Mouse Buttons, Modifier.
		void OnMouseUp(int X, int Y, int Buttons, int Modifier) override;

		/// Sets the pushed state of the button.
		/// @param Pushed Pushed.
		void SetPushed(bool Pushed);

	private:
		GUIBitmap* m_DrawBitmap;
		bool m_Pushed;
	};
} // namespace RTE
