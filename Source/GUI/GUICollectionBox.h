#pragma once

namespace RTE {

	/// A collection box control class that contains child controls.
	class GUICollectionBox : public GUIControl, public GUIPanel {

	public:
		// CollectionBox Notifications
		enum {
			Clicked = 0,
			MouseMove // Mouse moved over the panel
		} Notification;

		// Drawing type
		enum {
			Color,
			Image,
			Panel
		} DrawType;

		/// Constructor method used to instantiate a GUICollectionBox object in
		/// system memory.
		/// @param Manager GUIManager, GUIControlManager.
		GUICollectionBox(GUIManager* Manager, GUIControlManager* ControlManager);

		/// Called when the control has been created.
		/// @param Name Name, Position.
		void Create(const std::string& Name, int X, int Y, int Width = -1, int Height = -1) override;

		/// Called when the control has been created.
		/// @param Props Properties.
		void Create(GUIProperties* Props) override;

		/// Destroys and frees this' allocated data
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

		/// Called when the control needs to be moved.
		/// @param X New position.
		void Move(int X, int Y) override;

		/// Called when the control needs to be resized.
		/// @param Width New size.
		void Resize(int Width, int Height) override;

		/// Gets the rectangle of the control.
		/// @param X Position, Size.
		void GetControlRect(int* X, int* Y, int* Width, int* Height) override;

		/// Returns the panel of the control.
		/// @return 0 if the control does not have a panel, otherwise the topmost panel.
		GUIPanel* GetPanel() override;

		/// Returns a string representing the control's ID
		static std::string GetControlID() { return "COLLECTIONBOX"; };

		/// Gets the control to store the values into properties.
		void StoreProperties() override;

		/// Sets the drawing image bitmap to draw
		/// @param Bitmap Bitmap, ownership IS transferred!
		void SetDrawImage(GUIBitmap* Bitmap);

		/// Gets the drawing image bitmap that is being drawn
		/// @param Bitmap, ownership IS NOT transferred!
		GUIBitmap* GetDrawImage() { return m_DrawBitmap; }

		/// Sets whether to draw the background.
		/// @param DrawBack Draw.
		void SetDrawBackground(bool DrawBack);

		/// Sets the drawing type.
		/// @param Type Type.
		void SetDrawType(int Type);

		/// Gets the current drawing type.
		/// @return Type.
		int GetDrawType() const { return m_DrawType; }

		/// Sets the drawing color.
		/// @param Color Color.
		void SetDrawColor(unsigned long Color);

		/// Gets the drawing color.
		/// @return Color.
		unsigned long GetDrawColor() const { return m_DrawColor; }

		/// Applies new properties to the control.
		/// @param Props GUIProperties.
		void ApplyProperties(GUIProperties* Props) override;

	private:
		GUIBitmap* m_Background;

		bool m_DrawBackground;
		int m_DrawType;
		unsigned long m_DrawColor;
		GUIBitmap* m_DrawBitmap;

		/// Create the button bitmap to draw.
		void BuildBitmap();
	};
} // namespace RTE
