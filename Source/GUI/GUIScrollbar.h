#pragma once

#include "GUIScrollPanel.h"

namespace RTE {

	/// A Scrollbar control class.
	class GUIScrollbar : public GUIControl, public GUIScrollPanel {

	public:
		// Notifications
		enum {
			ChangeValue = 0,

		} Notifications;

		/// Constructor method used to instantiate a GUIScrollbar object in
		/// system memory.
		/// @param Manager GUIManager, GUIControlManager.
		GUIScrollbar(GUIManager* Manager, GUIControlManager* ControlManager);

		/// Called when the control has been created.
		/// @param Name Name, Position.
		void Create(const std::string& Name, int X, int Y, int Width = -1, int Height = -1) override;

		/// Called when the control has been created.
		/// @param Props Properties.
		void Create(GUIProperties* Props) override;

		/// Called when the control has been destroyed.
		void Destroy() override;

		/// Called when the skin has been changed.
		/// @param Skin New skin pointer.
		void ChangeSkin(GUISkin* Skin) override;

		/// Called when the mouse goes down on the panel
		/// @param X Mouse Position, Mouse Buttons, Modifier.
		void OnMouseDown(int X, int Y, int Buttons, int Modifier) override;

		/// Called when the mouse goes up on the panel
		/// @param X Mouse Position, Mouse Buttons, Modifier.
		void OnMouseUp(int X, int Y, int Buttons, int Modifier) override;

		/// Called when the mouse scroll wheel is moved.
		/// @param x Mouse X position.
		/// @param y Mouse Y position.
		/// @param modifier Activated modifier buttons.
		/// @param mouseWheelChange The amount of wheel movement. Positive is scroll up, negative is scroll down.
		void OnMouseWheelChange(int x, int y, int modifier, int mouseWheelChange) override;

		/// Returns the panel of the control.
		/// @return 0 if the control does not have a panel, otherwise the topmost panel.
		GUIPanel* GetPanel() override;

		/// Returns a string representing the control's ID
		static std::string GetControlID() { return "SCROLLBAR"; };

		/// Called when receiving a signal.
		/// @param Source Signal source, Signal code, Signal data.
		void ReceiveSignal(GUIPanel* Source, int Code, int Data) override;

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

		/// Applies new properties to the control.
		/// @param Props GUIProperties.
		void ApplyProperties(GUIProperties* Props) override;

	private:
	};
} // namespace RTE
