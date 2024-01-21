#pragma once

#include "GUITextPanel.h"
#include "GUIScrollPanel.h"

namespace RTE {

	/// A property page control class.
	class GUIPropertyPage : public GUIControl, public GUIPanel {

	public:
		// PropertyPage Notifications
		enum {
			Changed = 0, // Any text panel has changed. Property values are NOT updated
			Enter // A text panel has lost focus or the enter key was hit
		} Notification;

		/// Constructor method used to instantiate a GUIPropertyPage object in
		/// system memory.
		/// @param Manager GUIManager, GUIControlManager.
		GUIPropertyPage(GUIManager* Manager, GUIControlManager* ControlManager);

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

		/// Returns the panel of the control.
		/// @return 0 if the control does not have a panel, otherwise the topmost panel.
		GUIPanel* GetPanel() override;

		/// Returns a string representing the control's ID
		static std::string GetControlID() { return "PROPERTYPAGE"; };

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

		/// Refreshes the page with new variables & values.
		/// @param Props GUIProperties.
		void SetPropertyValues(GUIProperties* Props);

		/// Gets the properties in the page.
		GUIProperties* GetPropertyValues();

		/// Called when receiving a signal.
		/// @param Source Signal source, Signal code, Signal data.
		void ReceiveSignal(GUIPanel* Source, int Code, int Data) override;

		/// Clears the property page values.
		void ClearValues();

		/// Invokes an explicit update on text panels to property page.
		/// @return Boolean whether or not any values have changed.
		bool InvokeUpdate();

		/// Checks if any of the visible text panels have focus.
		bool HasTextFocus();

	private:
		GUIBitmap* m_DrawBitmap;
		unsigned long m_LineColor;

		GUIProperties m_PageValues;
		std::vector<GUITextPanel*> m_TextPanelList;
		GUIScrollPanel* m_VertScroll;

		/// Create the property page bitmap to draw.
		void BuildBitmap();
	};
} // namespace RTE
