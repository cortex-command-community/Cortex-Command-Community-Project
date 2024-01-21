#pragma once

namespace RTE {

	/// A scrollbar panel class used for controls requiring a scrollbar.
	class GUIScrollPanel : public GUIPanel {

	public:
		// Scroll panel orientation
		enum {
			Horizontal,
			Vertical
		};

		// Pre-built draw bitmaps
		enum {
			ButtonStates = 0,
			KnobStates,
			Back
		};

		// Signals
		enum {
			ChangeValue = 0,
			Grab,
			Release
		};

		/// Constructor method used to instantiate a GUIScrollPanel object in
		/// system memory.
		/// @param Manager GUIManager.
		explicit GUIScrollPanel(GUIManager* Manager);

		/// Constructor method used to instantiate a GUIScrollPanel object in
		/// system memory.
		GUIScrollPanel();

		/// Create the scrollpanel
		/// @param X Position, Size.
		void Create(int X, int Y, int Width, int Height);

		/// Called when the panel has been destroyed.
		void Destroy();

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

		/// Called when the mouse is hovering over the panel (has to be enabled)
		/// @param X Mouse Position, Mouse Buttons, Modifier.
		void OnMouseHover(int X, int Y, int Buttons, int Modifier) override;

		/// Adjusts the size of the panel.
		/// @param Width Width, Height.
		void SetSize(int Width, int Height);

		/// Sets the minimum value for the scrollpanel
		/// @param Min Minimum value.
		void SetMinimum(int Min);

		/// Gets the minimum value for the scrollpanel
		int GetMinimum() const;

		/// Sets the maximum value for the scrollpanel
		/// @param Max Maximum value.
		void SetMaximum(int Max);

		/// Gets the maximum value for the scrollpanel
		int GetMaximum() const;

		/// Sets the current value for the scrollpanel
		/// @param Value Value.
		void SetValue(int Value);

		/// Gets the current value of the scrollpanel.
		int GetValue() const;

		/// Sets the page size value for the scrollpanel.
		/// @param PageSize PageSize.
		void SetPageSize(int PageSize);

		/// Gets the size of the page.
		int GetPageSize() const;

		/// Sets the orientation of the scrollpanel.
		/// @param Orientation Orientation.
		void SetOrientation(int Orientation);

		/// Gets the orientation of the scrollpanel.
		int GetOrientation() const;

		/// Sets the small change value.
		/// @param SmallChange SmallChange.
		void SetSmallChange(int SmallChange);

		/// Gets the small change value.
		int GetSmallChange() const;

		/// Gets the value resolution for this scroll panel.
		/// @return The value resolution
		int GetValueResolution() const;

	protected:
		/// Load values from a property class.
		/// @param Props Properties.
		void LoadProps(GUIProperties* Props);

		/// Save values to a property class.
		/// @param Props Properties.
		void SaveProps(GUIProperties* Props) const;

		/// Build the bitmap.
		void BuildBitmap(bool UpdateSize, bool UpdateKnob);

	private:
		GUISkin* m_Skin;
		GUIBitmap* m_DrawBitmap[3];

		// User attributes
		int m_Orientation;
		int m_Minimum;
		int m_Maximum;
		int m_Value;
		int m_PageSize;
		int m_SmallChange;

		// Internal attributes
		bool m_RebuildSize;
		bool m_RebuildKnob;
		int m_ButtonSize;
		int m_MinimumKnobSize;
		int m_KnobPosition;
		int m_KnobLength;
		bool m_ButtonPushed[2];
		bool m_GrabbedKnob;
		bool m_GrabbedBackg;
		int m_GrabbedPos;
		int m_GrabbedSide;
		int m_ValueResolution; //!< How much the value increases/decreases on each mouse wheel change when scrolling.

		/// Build a button.
		/// @param ArrowName ArrowName, Width, Height.
		void BuildButton(const std::string& ArrowName, int Y, int Width, int Height);

		/// Build the background.
		void BuildBackground();

		/// Build the knob.
		void BuildKnob(const std::string& Section, int X, int Y, int Width, int Height);

		/// Calculate the knob size and position.
		void CalculateKnob();

		/// Adjusts the value.
		/// @param Delta Delta movement.
		void AdjustValue(int Delta);
	};
} // namespace RTE
