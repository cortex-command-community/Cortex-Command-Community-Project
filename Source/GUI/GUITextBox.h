#pragma once

#include "GUITextPanel.h"

namespace RTE {

	/// A TextBox control class.
	class GUITextBox : public GUIControl, public GUITextPanel {

	public:
		// Notifications
		enum {
			Changed = 0,
			Clicked,
			Enter
		} Notification;

		/// Constructor method used to instantiate a GUITextBox object in
		/// system memory.
		/// @param Manager GUIManager, GUIControlManager.
		GUITextBox(GUIManager* Manager, GUIControlManager* ControlManager);

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

		/// Draws the panel
		/// @param Screen Screen class
		void Draw(GUIScreen* Screen) override;

		/// Returns the panel of the control.
		/// @return 0 if the control does not have a panel, otherwise the topmost panel.
		GUIPanel* GetPanel() override;

		/// Returns a string representing the control's ID
		static std::string GetControlID() { return "TEXTBOX"; };

		/// Called when the control needs to be moved.
		/// @param X New position.
		void Move(int X, int Y) override;

		/// Called when the control needs to be resized.
		/// @param Width New size.
		void Resize(int Width, int Height) override;

		/// Gets the rectangle of the control.
		/// @param X Position, Size.
		void GetControlRect(int* X, int* Y, int* Width, int* Height) override;

		/// Called when receiving a signal.
		/// @param Source Signal source, Signal code, Signal data.
		void ReceiveSignal(GUIPanel* Source, int Code, int Data) override;

		/// Applies new properties to the control.
		/// @param Props GUIProperties.
		void ApplyProperties(GUIProperties* Props) override;

	private:
		GUIBitmap* m_DrawBitmap;
		int m_HAlignment;
		int m_VAlignment;
	};
} // namespace RTE
