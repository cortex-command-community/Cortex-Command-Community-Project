#pragma once

#include "GUIWriter.h"

namespace RTE {

	class GUIControlManager;

	/// A base class inherited by all controls.
	class GUIControl {

	public:
		// Anchor points
		enum {
			Anchor_Left = 0x01,
			Anchor_Top = 0x02,
			Anchor_Right = 0x04,
			Anchor_Bottom = 0x08
		} Anchor;

		/// Constructor method used to instantiate a GUIControl object in
		/// system memory.
		GUIControl();

		virtual ~GUIControl() = default;

		/// Called when the control has been created.
		/// @param Name Name, Position, Size
		virtual void Create(const std::string& Name, int X, int Y, int Width = -1, int Height = -1);

		/// Called when the control has been created.
		/// @param Props Properties.
		virtual void Create(GUIProperties* Props);

		/// Called when the control has been destroyed.
		virtual void Destroy();

		/// Called when the control is activated and ready for use.
		virtual void Activate();

		/// Called when the skin has been changed.
		/// @param Skin New skin pointer.
		virtual void ChangeSkin(GUISkin* Skin);

		/// Add a new event to the queue.
		/// @param Type Type, Message, Data.
		void AddEvent(int Type, int Msg, int Data);

		/// Gets the control's name.
		std::string GetName();

		/// Sets the control's tooltip string.
		/// @param m_Properties.SetValue("ToolTip" The new ToolTip for this.
		void SetToolTip(const std::string& tip) { m_Properties.SetValue("ToolTip", tip); }

		/// Gets the control's tooltip string.
		std::string GetToolTip();

		/// Returns a string representing the control's ID
		std::string GetID() const;

		/// Returns the anchor flags.
		int GetAnchor();

		/// Adds a child to this control
		/// @param Control Control.
		void AddChild(GUIControl* Control);

		/// Gets the children lst
		std::vector<GUIControl*>* GetChildren();

		/// Returns the panel of the control.
		/// @return 0 if the control does not have a panel, otherwise the topmost panel.
		virtual GUIPanel* GetPanel();

		/// Saves the control properties.
		/// @param W Writer.
		/// @return True if sucessful
		bool Save(GUIWriter* W);

		/// Gets the control to store the values into properties.
		virtual void StoreProperties();

		/// Called when the control needs to be moved.
		/// @param X New position.
		virtual void Move(int X, int Y);

		/// Called when the control needs to be resized.
		/// @param Width New size.
		virtual void Resize(int Width, int Height);

		/// Gets the rectangle of the control.
		/// @param X Position, Size.
		virtual void GetControlRect(int* X, int* Y, int* Width, int* Height);

		/// Sets the visibility of the control.
		/// @param Visible Visible.
		virtual void SetVisible(bool Visible);

		/// Gets the visibility of the control.
		virtual bool GetVisible();

		/// Sets the enabled state of the control.
		/// @param Enabled Enabled.
		virtual void SetEnabled(bool Enabled);

		/// Gets the enabled state of the control.
		virtual bool GetEnabled();

		/// Gets the parent of this control.
		GUIControl* GetParent();

		/// Gets the control properties.
		GUIProperties* GetProperties();

		/// Applies new properties to the control.
		/// @param Props GUIProperties.
		virtual void ApplyProperties(GUIProperties* Props);

		/// Returns the IsContainer value.
		bool IsContainer();

		/// Removes a child based on name.
		/// @param Name Child Name.
		void RemoveChild(const std::string Name);

		/// Removes all the children.
		void RemoveChildren();

	protected:
		GUISkin* m_Skin;
		int m_SkinPreset;
		GUIProperties m_Properties;
		GUIControl* m_ControlParent;
		std::vector<GUIControl*> m_ControlChildren;

		std::string m_ControlID;

		bool m_IsContainer;

		// For the GUI editor
		int m_MinWidth;
		int m_MinHeight;
		int m_DefWidth;
		int m_DefHeight;

		GUIControlManager* m_ControlManager;
	};
} // namespace RTE
