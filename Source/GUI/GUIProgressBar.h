#pragma once

namespace RTE {

	/// A progressbar control class.
	class GUIProgressBar : public GUIControl, public GUIPanel {

	public:
		// Progressbar Notifications
		enum {
			Clicked = 0,
			Changed
		} Notification;

		/// Constructor method used to instantiate a GUIProgressBar object in
		/// system memory.
		/// @param Manager GUIManager, GUIControlManager.
		GUIProgressBar(GUIManager* Manager, GUIControlManager* ControlManager);

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

		/// Called when the mouse goes down on the panel
		/// @param X Mouse Position, Mouse Buttons, Modifier.
		void OnMouseDown(int X, int Y, int Buttons, int Modifier) override;

		/// Called when the mouse goes up on the panel
		/// @param X Mouse Position, Mouse Buttons, Modifier.
		void OnMouseUp(int X, int Y, int Buttons, int Modifier) override;

		/// Called when the mouse moves (over the panel, or when captured).
		/// @param X Mouse Position, Mouse Buttons, Modifier.
		void OnMouseMove(int X, int Y, int Buttons, int Modifier) override;

		/// Returns the panel of the control.
		/// @return 0 if the control does not have a panel, otherwise the topmost panel.
		GUIPanel* GetPanel() override;

		/// Returns a string representing the control's ID
		static std::string GetControlID() { return "PROGRESSBAR"; };

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

		/// Sets the value.
		/// @param Value Value.
		void SetValue(int Value);

		/// Gets the value.
		int GetValue() const;

		/// Sets the minimum.
		/// @param Minimum Minimum.
		void SetMinimum(int Minimum);

		/// Gets the minimum.
		int GetMinimum() const;

		/// Sets the maximum.
		/// @param Maximum Maximum.
		void SetMaximum(int Maximum);

		/// Gets the maximum.
		int GetMaximum() const;

		/// Applies new properties to the control.
		/// @param Props GUIProperties.
		void ApplyProperties(GUIProperties* Props) override;

	private:
		GUIBitmap* m_DrawBitmap;
		GUIBitmap* m_IndicatorImage;

		int m_Minimum;
		int m_Maximum;
		int m_Value;
		int m_Spacing;

		/// Create the progressbar bitmap to draw.
		void BuildBitmap();
	};
} // namespace RTE
