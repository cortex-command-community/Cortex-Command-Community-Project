#pragma once

namespace RTE {

	/// A class to hold event information.
	class GUIEvent {

	public:
		// Event Types
		enum {
			Command = 0,
			Notification
		} EventType;

		/// Constructor method used to instantiate a GUIEvent object in system
		/// memory.
		GUIEvent();

		/// Constructor method used to instantiate a GUIEvent object in system
		/// memory.
		/// @param Control Control, Event type, Msg, Data.
		GUIEvent(GUIControl* Control, int Type, int Msg, int Data);

		/// Gets the event type
		int GetType() const;

		/// Gets the msg.
		int GetMsg() const;

		/// Gets the data.
		int GetData() const;

		/// Gets the event control.
		GUIControl* GetControl();

	private:
		GUIControl* m_Control;
		int m_Type;
		int m_Msg;
		int m_Data;
	};
} // namespace RTE
