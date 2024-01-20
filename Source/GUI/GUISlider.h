#pragma once

namespace RTE {

	/// A slider control class.
	class GUISlider : public GUIControl, public GUIPanel {

	public:
		// Slider orientation
		enum {
			Horizontal,
			Vertical
		} Orientation;

		// Tick Direction
		enum {
			TopLeft,
			BottomRight
		} TickDirection;

		// Slider Notifications
		enum {
			Changed = 0,
			Clicked
		} Notification;

		/// Constructor method used to instantiate a GUISlider object in
		/// system memory.
		/// @param Manager GUIManager, GUIControlManager.
		GUISlider(GUIManager* Manager, GUIControlManager* ControlManager);

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
		static std::string GetControlID() { return "SLIDER"; };

		/// Called when the control needs to be moved.
		/// @param X New position.
		void Move(int X, int Y) override;

		/// Called when the control needs to be resized.
		/// @param Width New size.
		void Resize(int Width, int Height) override;

		/// Gets the rectangle of the control.
		/// @param X Position, Size.
		void GetControlRect(int* X, int* Y, int* Width, int* Height) override;

		/// Sets the orientation of the slider.
		/// @param Orientation Orientation.
		void SetOrientation(int Orientation);

		/// Gets the orientation of the slider.
		int GetOrientation() const;

		/// Sets the direction of the ticks.
		/// @param TickDir TickDir.
		void SetTickDirection(int TickDir);

		/// Gets the direction of the ticks.
		int GetTickDirection() const;

		/// Sets the minimum value.
		/// @param Minimum Minimum.
		void SetMinimum(int Minimum);

		/// Gets the minimum value.
		int GetMinimum() const;

		/// Sets the maximum value.
		/// @param Maximum Maximum.
		void SetMaximum(int Maximum);

		/// Gets the maximum value.
		int GetMaximum() const;

		/// Sets the value.
		/// @param Value Value.
		void SetValue(int Value);

		/// Gets the value.
		int GetValue() const;

		/// Gets the control to store the values into properties.
		void StoreProperties() override;

		/// Applies new properties to the control.
		/// @param Props GUIProperties.
		void ApplyProperties(GUIProperties* Props) override;

		/// Sets the value resolution for this slider.
		/// @param valueRes The new value resolution
		void SetValueResolution(int valueRes);

	private:
		GUIBitmap* m_DrawBitmap;
		GUIBitmap* m_KnobImage;

		// Properties
		int m_Orientation;
		int m_TickDirection;
		int m_Minimum;
		int m_Maximum;
		int m_Value;
		int m_ValueResolution;

		// Internal variables
		int m_KnobPosition;
		int m_KnobSize;
		bool m_KnobGrabbed;
		int m_KnobGrabPos;
		int m_EndThickness;
		int m_OldValue;

		/// Create the slider bitmap to draw.
		void BuildBitmap();

		/// Builds the background line for the slider
		/// @param Section Section, SrcImage.
		void BuildLine(const std::string& Section, GUIBitmap* SrcImage);

		/// Calculates the knob position and size.
		void CalculateKnob();
	};
} // namespace RTE
