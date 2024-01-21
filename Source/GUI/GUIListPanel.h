#pragma once

#include "GUIScrollPanel.h"
#include "GUIInterface.h"

namespace RTE {

	class Entity;

	/// A listbox panel class used for controls requiring a listbox.
	class GUIListPanel : public GUIPanel {

	public:
		// Signals
		enum {
			Click, // Mouse click. Not just inside the panel
			MouseDown, // Mouse down inside the panel
			MouseUp, // Mouse up inside the panel
			Select, // Selected (on mouse down) an item
			MouseMove, // Mouse moved over the panel
			MouseEnter, // Mouse left the panel
			MouseLeave, // Mouse left the panel
			DoubleClick, // Double click
			KeyDown, // Key Down
			EdgeHit //!< Tried scrolling the selection past the first or last item. data = 0 for top edge, data = 1 for bottom edge.
		} Signal;

		// Item structure
		struct Item {
			int m_ID = 0;
			std::string m_Name;
			// Extra text field displayed right-justified in the item
			std::string m_RightText;
			// Extra index for special indexing or reference that the item is associated with. Menu-specific
			int m_ExtraIndex = 0;
			bool m_Selected = false;
			// Can contain a bitmap to display in the list
			// This is OWNED
			GUIBitmap* m_pBitmap = nullptr;
			// Extra data associated with the item
			// This is NOT OWNED
			const Entity* m_pEntity = nullptr;
			int m_Height = 0;
			// An x offset to apply to the item (to allow nesting)
			int m_OffsetX = 0;

			Item() {}
			~Item() {
				delete m_pBitmap;
				m_pBitmap = 0;
			}
		};

		/// Constructor method used to instantiate a GUIListPanel object in
		/// system memory.
		/// @param Manager GUIManager.
		GUIListPanel(GUIManager* Manager);

		/// Constructor method used to instantiate a GUIListPanel object in
		/// system memory.
		GUIListPanel();

		/// Create the listpanel
		/// @param X Position, Size.
		void Create(int X, int Y, int Width, int Height);

		/// Called when the panel has been destroyed.
		void Destroy();

		/// Add an item to the list.
		/// @param Name Name, extraText, Text which will be displayed right-justified in the item.
		/// a Bitmap object pointer alternatively pointing to a bitmap to display
		/// in the list. Ownership IS transferred!
		/// @param rightText An Extra menu-specific index that this item may be associated with. (default: "")
		/// @param pBitmap Object instance associated with the item. Ownership is NOT TRANSFERRED! (default: nullptr)
		void AddItem(const std::string& Name, const std::string& rightText = "", GUIBitmap* pBitmap = nullptr, const Entity* pEntity = 0, const int extraIndex = -1, const int offsetX = 0);

		/// Clears the list.
		void ClearList();

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

		/// Called when the mouse enters the panel
		/// @param X Mouse Position, Mouse Buttons, Modifier.
		void OnMouseEnter(int X, int Y, int Buttons, int Modifier) override;

		/// Called when the mouse leaves the panel
		/// @param X Mouse Position, Mouse Buttons, Modifier.
		void OnMouseLeave(int X, int Y, int Buttons, int Modifier) override;

		/// Called when the mouse has double-clicked on the pane.
		/// @param X Mouse Position, Mouse Buttons, Modifier.
		void OnDoubleClick(int X, int Y, int Buttons, int Modifier) override;

		/// Called when the mouse scroll wheel is moved.
		/// @param x Mouse X position.
		/// @param y Mouse Y position.
		/// @param modifier Activated modifier buttons.
		/// @param mouseWheelChange The amount of wheel movement. Positive is scroll up, negative is scroll down.
		void OnMouseWheelChange(int x, int y, int modifier, int mouseWheelChange) override;

		/// Called when a key is pressed (OnDown & repeating).
		/// @param KeyCode KeyCode, Modifier.
		void OnKeyPress(int KeyCode, int Modifier) override;

		/// Called when a key goes down.
		/// @param KeyCode KeyCode, Modifier.
		void OnKeyDown(int KeyCode, int Modifier) override;

		/// Checks if a point is inside the panel, but not on the scrollbars if
		/// they are visible
		/// @param X X, Y Coordinates of point
		/// @return A boolean of the check result
		bool PointInsideList(int X, int Y);

		/// Called when the panel gains focus.
		void OnGainFocus() override;

		/// Called when the panel looses focus.
		void OnLoseFocus() override;

		/// Called when receiving a signal.
		/// @param Source Signal source, Signal code, Signal data.
		void ReceiveSignal(GUIPanel* Source, int Code, int Data) override;

		/// Locks the control from updating every time a new item is added.
		void BeginUpdate();

		/// UnLocks the control from updating every time a new item is added.
		/// Will automatically update the control.
		void EndUpdate();

		/// Sets the multi-selection value.
		/// @param MultiSelect MultiSelect
		void SetMultiSelect(bool MultiSelect);

		/// Gets the multi-selection value.
		bool GetMultiSelect() const;

		/// Sets the hot tracking value.
		/// @param HotTrack HotTrack.
		void SetHotTracking(bool HotTrack);

		/// Gets the selected (or first in the selected list) item.
		Item* GetSelected();

		/// Gets the item list.
		std::vector<Item*>* GetItemList();

		/// Gets an item at the index.
		/// @param Index Index.
		Item* GetItem(int Index);

		/// Gets an item at a specific absolute screen coordinate, if any.
		/// @param X The absolute screen coordinates to get the item from.
		Item* GetItem(int X, int Y);

		/// Gets the drawing height of the item passed in.
		/// @param pItem Pointer to the Item to get the height of. Ownership is NOT transferred!
		int GetItemHeight(Item* pItem);

		/// Gets the height, in pixels, of the stack of items up to a specific one.
		/// E.g. If the specified one is the first (top) in the list, 0 is returned.
		/// If the second one is specified, the height of the first is returned.
		/// If the third is specified, the sum of the first and second items' heights
		/// is returned. If 0 is passed, the entire stack's height is returned.
		/// @param pItem Pointer to the Item to get the height up to. Ownership is NOT transferred! (default: nullptr)
		/// If 0 is passed, the entire stack's height is returned.
		int GetStackHeight(Item* pItem = nullptr);

		/// Gets the scroll value, in pixels, of the vertical axis.
		/// @param m_VertScroll->GetValue( The scroll value in pixels.
		int GetScrollVerticalValue() const { return m_VertScroll->GetValue(); }

		/// Sets the values of a specific Item.
		/// @param Index Index and Item reference.
		void SetItemValues(int Index, Item& item);

		/// Gets the selected (or first in the selected list) index.
		/// @return Index, or -1 if there is no items selected.
		int GetSelectedIndex();

		/// Selects the item at the index.
		/// @param Index Index.
		void SetSelectedIndex(int Index);

		/// Selects this to draw its items differently, with lines instead of
		/// rectangles, etc
		/// @param enableAltDrawMode The new mode setting. (default: true)
		void SetAlternateDrawMode(bool enableAltDrawMode = true) { m_AlternateDrawMode = enableAltDrawMode; }

		/// Gets the selection list.
		std::vector<Item*>* GetSelectionList();

		/// Deletes an item at the index.
		/// @param Index Index.
		void DeleteItem(int Index);

		/// Adjusts the size of the panel.
		/// @param Width Width, Height.
		void SetSize(int Width, int Height) override;

		/// Adjusts the position of the panel.
		/// @param X X, Y.
		void SetPositionAbs(int X, int Y);

		/// Sets the enabled state of both scrollbars.
		/// @param Horizontal Horz, Vert
		void EnableScrollbars(bool Horizontal, bool Vertical);

		/// Adjusts the vertical scrollbar to show the specific item in the list.
		/// @param pItem The item you want to show.
		void ScrollToItem(Item* pItem);

		/// Adjusts the vertical scrollbar to show the first selected item in the
		/// list.
		void ScrollToSelected();

		/// Adjusts the vertical scrollbar to show the top of the list
		void ScrollToTop();

		/// Adjusts the vertical scrollbar to show the bottom of the list
		void ScrollToBottom();

		/// Scrolls the GUIListPanel up.
		void ScrollUp();

		/// Scrolls the GUIListPanel down.
		void ScrollDown();

		/// Scrolls the the GUIListPanel to a specific position
		/// @param position The position to scroll to.
		void ScrollTo(int position);

		/// Sets whether the scroll panel scrolls in a loop or not.
		/// @param scrollLoop True to scroll in a loop, false to scroll with edge stopping.
		void SetSelectionScrollingLoop(bool scrollLoop);

		/// Sets whether the list panel can be scrolled with the mouse scroll wheel.
		/// @param mouseScroll True to enable scrolling, false to disable.
		void SetMouseScrolling(bool mouseScroll);

		/// Sets the thickness (width on vertical, height on horizontal) of the ListPanel's scroll bars and adjusts them to the new thickness.
		/// @param newThickness The new scroll bar thickness, in pixels.
		void SetScrollBarThickness(int newThickness) {
			m_ScrollBarThickness = newThickness;
			AdjustScrollbars();
		}

		/// Sets the padding around the ListPanel's scrollbars and adjusts them to the new padding. Used to better size and position scrollbars within panel bounds, allowing to not overdraw on panel borders.
		/// @param newPadding The new scrollbar padding, in pixels.
		void SetScrollBarPadding(int newPadding) {
			m_ScrollBarPadding = newPadding;
			AdjustScrollbars();
		}

		/// Build the bitmap.
		/// @param UpdateBase UpdateBase, UpdateText.
		void BuildBitmap(bool UpdateBase, bool UpdateText);

	private:
		GUISkin* m_Skin;
		GUIBitmap* m_BaseBitmap;
		GUIBitmap* m_DrawBitmap;
		GUIBitmap* m_FrameBitmap;
		unsigned long m_FontSelectColor;

		bool m_UpdateLocked;

		GUIScrollPanel* m_HorzScroll;
		GUIScrollPanel* m_VertScroll;
		bool m_HorzScrollEnabled;
		bool m_VertScrollEnabled;
		int m_ScrollBarThickness; //!< The thickness (width on vertical, height on horizontal) of the ListPanel's scroll bars, in pixels.
		int m_ScrollBarPadding; //!< The padding around the scrollbars, in pixels. Used to better size and position scrollbars within panel bounds, allowing to not overdraw on panel borders.

		bool m_CapturedHorz;
		bool m_CapturedVert;
		bool m_ExternalCapture;

		int m_LargestWidth;
		bool m_MultiSelect;
		bool m_HotTracking;
		int m_LastSelected;
		bool m_LoopSelectionScroll; //!< Whether the list panel scrolls in a loop or not, while scrolling the selection list (as opposed to the scrollbar).
		bool m_MouseScroll; //!< Whether the list panel enables scrolling with the mouse scroll wheel.

		bool m_AlternateDrawMode; // This draws items differently, not with boxes etc.

		std::vector<Item*> m_Items;
		std::vector<Item*> m_SelectedList;
		unsigned long m_SelectedColorIndex;
		unsigned long m_UnselectedColorIndex;

		/// Build the drawing bitmap.
		void BuildDrawBitmap();

		/// Adjusts the scrollbars.
		void AdjustScrollbars();

		/// Selects an item based on mouse position.
		/// @param X Mouse Position, Modifier.
		void SelectItem(int X, int Y, int Modifier);

		/// Perform list scrolling through the scrollbar.
		/// @param MouseWheelChange Amount and direction of scrolling. Positive to scroll up, negative to scroll down.
		void ScrollBarScrolling(int mouseWheelChange);

		/// Perform list scrolling by changing the currently selected list item.
		/// @param MouseWheelChange Amount and direction of scrolling. Positive to scroll up, negative to scroll down.
		void SelectionListScrolling(int mouseWheelChange);
	};
} // namespace RTE
